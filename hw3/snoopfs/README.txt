README.txt

Joshua Vaughen, 996696191
jshvn, jshvn@ucdavis.edu
vnode.c README.txt
HW3
ECS150 - Operating systems
5/21/13

General logic: with snoopfs, and the modified vnode.c file, we aim to intercept calls to the
file system in order to print out information about what the calls are actually doing. For
example, if a process such as cat is run, we are interested in knowing which file was
read in terms of inode number, what the block size was, and how many bytes were read.

We do not obstruct the users call to the underlying file system, however, so we call
the bypass function to be sure that the users query goes down another level to the 
file system that snoopfs sits atop of. 

vnode.c :

    includes the two added functions which intercept calls to the file system. these are
    both the read and write calls, as detailed below.

    modules:
    	static int snoopfs_read(ap) struct vop_read_args *ap;
    	this module intercepts the reads from the user and prints out
    	the information 0::inode#::block#::#ofbytesread

    	static int snoopfs_write(ap) struct vop_write_args *ap;
    	this module intercepts the writes from the user and prints out
    	the information 1::inode#::block#::#ofbyteswritten

    I had to add the lines:

        { &vop_read_desc, (vop_t *) snoopfs_read },
 		{ &vop_write_desc, (vop_t *) snoopfs_write },

 	into the 

 		static struct vnodeopv_entry_desc snoopfs_vnodeop_entries[]

 	structure to register the new calls for use by snoopfs. Otherwise, if 
 	they are called, there would be no method defined for them to do anything
 	with, which wouldn't be fun.

 	as a final note the headers

 		#include <ufs/ufs/quota.h>
		#include <ufs/ufs/inode.h>
		#include <sys/types.h>
		#include <sys/uio.h>

	had to be included to provide access to the necessary data structures.

	
