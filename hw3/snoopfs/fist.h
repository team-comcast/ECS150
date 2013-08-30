/*
 * Copyright (c) 1997-2005 Erez Zadok <ezk@cs.stonybrook.edu>
 * Copyright (c) 2001-2005 Stony Brook University
 *
 * For specific licensing information, see the COPYING file distributed with
 * this package, or get one from ftp://ftp.filesystems.org/pub/fistgen/COPYING.
 *
 * This Copyright notice must be kept intact and distributed with all
 * fistgen sources INCLUDING sources generated by fistgen.
 */
#ifndef __FIST_H_
#define	__FIST_H_

#include <sys/types.h>
#ifdef _KERNEL
#include <sys/dirent.h>
#include <sys/ioccom.h>
#include <sys/malloc.h>
#include <sys/namei.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/sysctl.h>
#include <sys/systm.h>
#include <sys/vnode.h>
#include <sys/bio.h>
#include <sys/buf.h>
#include <sys/kernel.h>
#include <sys/mount.h>
#include <sys/fcntl.h>
#include <sys/vnode.h>

#include <vm/vm.h>
#include <vm/vm_extern.h>
#include <vm/vm_object.h>
#include <vm/vm_pageout.h>
#include <vm/vm_page.h>
#include <vm/vm_pager.h>
#include <vm/uma.h>
#include <vm/vnode_pager.h>


#include <machine/cputypes.h>

#include <limits.h>

#else /* not _KERNEL */
#include <string.h>
#endif /* not _KERNEL */

/* definitions for both user and kernel code */
#include <stdarg.h>

#endif /* not __FIST_H_ */
