/*
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software donated to Berkeley by
 * Jan-Simon Pendry.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)snoopfs_subr.c	8.7 (Berkeley) 5/14/95
 *
 * $FreeBSD: src/sys/fs/snoopfs/snoopfs_subr.c,v 1.38 2002/10/14 03:20:33 mckusick Exp $
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */
#ifdef FISTGEN
# include "fist_snoopfs.h"
#endif /* FISTGEN */
#include <fist.h>
#include <snoopfs.h>

#if 0
#include "opt_debug.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/malloc.h>
#include <sys/mount.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#include <fs/snoopfs/snoopfs.h>
#endif

#define LOG2_SIZEVNODE 7		/* log2(sizeof struct vnode) */
#define	NSNOOPFSNODECACHE 16

/*
 * Wrapfs layer cache:
 * Each cache entry holds a reference to the lower vnode
 * along with a pointer to the alias vnode.  When an
 * entry is added the lower vnode is VREF'd.  When the
 * alias is removed the lower vnode is vrele'd.
 */

#define	SNOOPFS_NHASH(vp) \
	(&snoopfs_node_hashtbl[(((uintptr_t)vp)>>LOG2_SIZEVNODE) & snoopfs_node_hash])

static LIST_HEAD(snoopfs_node_hashhead, snoopfs_node) *snoopfs_node_hashtbl;
static u_long snoopfs_node_hash;
struct mtx snoopfs_hashmtx;

static MALLOC_DEFINE(M_SNOOPFSHASH, "SNOOPFS hash", "SNOOPFS hash table");
MALLOC_DEFINE(M_SNOOPFSNODE, "SNOOPFS node", "SNOOPFS vnode private part");

static struct vnode * snoopfs_hashget(struct vnode *);
static struct vnode * snoopfs_hashins(struct snoopfs_node *);

int snoopfs_init(struct vfsconf *vfsp);
int snoopfs_uninit(struct vfsconf *vfsp);

/* defined by EZK */
void * kmem_zalloc(unsigned long size);
int fist_uiomove(caddr_t cp, int n, enum uio_rw rwflag, struct uio *uio);

/*
 * Initialise cache headers
 */
int
snoopfs_init(vfsp)
	struct vfsconf *vfsp;
{

   fist_dprint(4, "FXN=%s FILE=%s LINE=%d\n",__FUNCTION__,__FILE__,__LINE__);
  fist_dprint(2, "snoopfs_init\n");		/* printed during system boot */
   snoopfs_node_hashtbl = hashinit(NSNOOPFSNODECACHE, M_SNOOPFSHASH, &snoopfs_node_hash);
	mtx_init(&snoopfs_hashmtx, "snoopfs", NULL, MTX_DEF);
	return (0);
}

int
snoopfs_uninit(vfsp)
	struct vfsconf *vfsp;
{
   fist_dprint(4, "FXN=%s FILE=%s LINE=%d\n",__FUNCTION__,__FILE__,__LINE__);
   mtx_destroy(&snoopfs_hashmtx);
   if (snoopfs_node_hashtbl) {
    free(snoopfs_node_hashtbl, M_SNOOPFSHASH);
   }
   return (0);
}

/*
 * Return a VREF'ed alias for lower vnode if already exists, else 0.
 * Lower vnode should be locked on entry and will be left locked on exit.
 */
static struct vnode *
snoopfs_hashget(lowervp)
	struct vnode *lowervp;
{
  struct thread *td = curthread;	/* XXX */
  struct snoopfs_node_hashhead *hd;
  struct snoopfs_node *a;
  struct vnode *vp;

  fist_dprint(4, "FXN=%s FILE=%s LINE=%d\n",__FUNCTION__,__FILE__,__LINE__);

  /*
   * Find hash base, and then search the (two-way) linked
   * list looking for a snoopfs_node structure which is referencing
   * the lower vnode.  If found, the increment the snoopfs_node
   * reference count (but NOT the lower vnode's VREF counter).
   */
  hd = SNOOPFS_NHASH(lowervp);
loop:
   mtx_lock(&snoopfs_hashmtx);
   LIST_FOREACH(a, hd, snoopfs_hash) {
     if (a->snoopfs_lowervp == lowervp) {
       vp = SNOOPFS_TO_VP(a);
       mtx_lock(&vp->v_interlock);
       mtx_unlock(&snoopfs_hashmtx);
       /*
        * We need vget for the VXLOCK
        * stuff, but we don't want to lock
        * the lower node.
        */
       if (vget(vp, LK_EXCLUSIVE | LK_THISLAYER | LK_INTERLOCK, td)) {
	 printf ("snoopfs_node_find: vget failed.\n");
	 goto loop;
       }
       return (vp);
     }
   }
   mtx_unlock(&snoopfs_hashmtx);

   print_location();
   return (NULLVP);
}

/*
 * Act like snoopfs_hashget, but add passed snoopfs_node to hash if no existing
 * node found.
 */
static struct vnode *
snoopfs_hashins(xp)
	struct snoopfs_node *xp;
{
  struct thread *td = curthread;	/* XXX */
  struct snoopfs_node_hashhead *hd;
  struct snoopfs_node *oxp;
  struct vnode *ovp;
  fist_dprint(4, "FXN=%s FILE=%s LINE=%d\n",__FUNCTION__,__FILE__,__LINE__);
  hd = SNOOPFS_NHASH(xp->snoopfs_lowervp);
loop:
  mtx_lock(&snoopfs_hashmtx);
  LIST_FOREACH(oxp, hd, snoopfs_hash) {
    if (oxp->snoopfs_lowervp == xp->snoopfs_lowervp) {
      ovp = SNOOPFS_TO_VP(oxp);
      mtx_lock(&ovp->v_interlock);
      mtx_unlock(&snoopfs_hashmtx);
      if (vget(ovp, LK_EXCLUSIVE | LK_THISLAYER | LK_INTERLOCK, td))
	goto loop;

      return (ovp);
    }
  }
  LIST_INSERT_HEAD(hd, xp, snoopfs_hash);
  mtx_unlock(&snoopfs_hashmtx);
  print_location();
  return (NULLVP);
}

/*
 * Make a new or get existing snoopfs node.
 * Vp is the alias vnode, lowervp is the lower vnode.
 *
 * The lowervp assumed to be locked and having "spare" reference. This routine
 * vrele lowervp if snoopfs node was taken from hash. Otherwise it "transfers"
 * the caller's "spare" reference to created snoopfs vnode.
 */
int
snoopfs_nodeget(mp, lowervp, vpp)
	struct mount *mp;
	struct vnode *lowervp;
	struct vnode **vpp;
{
  struct thread *td = curthread;	/* XXX */
  struct snoopfs_node *xp;
  struct vnode *vp;
  int error;

  fist_dprint(4, "FXN=%s FILE=%s LINE=%d\n",__FUNCTION__,__FILE__,__LINE__);
  /* Lookup the hash firstly */
  *vpp = snoopfs_hashget(lowervp);
  if (*vpp != NULL) {
    vrele(lowervp);
    return (0);
  }

  /*
   * We do not serialize vnode creation, instead we will check for
   * duplicates later, when adding new vnode to hash.
   *
   * Note that duplicate can only appear in hash if the lowervp is
   * locked LK_SHARED.
   */

  /*
   * Do the MALLOC before the getnewvnode since doing so afterward
   * might cause a bogus v_data pointer to get dereferenced
   * elsewhere if MALLOC should block.
   */
  MALLOC(xp, struct snoopfs_node *, sizeof(struct snoopfs_node),
	 M_SNOOPFSNODE, M_WAITOK);

  error = getnewvnode("snoopfs", mp, snoopfs_vnodeop_p, &vp);
  if (error) {
    FREE(xp, M_SNOOPFSNODE);
    return (error);
  }

  xp->snoopfs_vnode = vp;
  xp->snoopfs_lowervp = lowervp;

  vp->v_type = lowervp->v_type;
  vp->v_data = xp;

  /*
   * From NetBSD:
   * Now lock the new node. We rely on the fact that we were passed
   * a locked vnode. If the lower node is exporting a struct lock
   * (v_vnlock != NULL) then we just set the upper v_vnlock to the
   * lower one, and both are now locked. If the lower node is exporting
   * NULL, then we copy that up and manually lock the new vnode.
   */

  vp->v_vnlock = lowervp->v_vnlock;
  error = VOP_LOCK(vp, LK_EXCLUSIVE | LK_THISLAYER, td);
  if (error)
    panic("snoopfs_nodeget: can't lock new vnode\n");

  /*
   * Atomically insert our new node into the hash or vget existing
   * if someone else has beaten us to it.
   */
  *vpp = snoopfs_hashins(xp);
  if (*vpp != NULL) {
    vrele(lowervp);
    VOP_UNLOCK(vp, LK_THISLAYER, td);
    vp->v_vnlock = NULL;
    xp->snoopfs_lowervp = NULL;
    vrele(vp);
    return (0);
  }

  /*
   * XXX We take extra vref just to workaround UFS's XXX:
   * UFS can vrele() vnode in VOP_CLOSE() in some cases. Luckily, this
   * can only happen if v_usecount == 1. To workaround, we just don't
   * let v_usecount be 1, it will be 2 or more.
   */
  VREF(lowervp);

  *vpp = vp;

  print_location();
  return (0);
}

/*
 * Remove node from hash.
 */
void
snoopfs_hashrem(xp)
	struct snoopfs_node *xp;
{

	mtx_lock(&snoopfs_hashmtx);
	LIST_REMOVE(xp, snoopfs_hash);
	mtx_unlock(&snoopfs_hashmtx);
}

#ifdef DIAGNOSTIC
#include "opt_ddb.h"

#ifdef DDB
#define	snoopfs_checkvp_barrier	1
#else
#define	snoopfs_checkvp_barrier	0
#endif

struct vnode *
snoopfs_checkvp(vp, fil, lno)
	struct vnode *vp;
	char *fil;
	int lno;
{
  struct snoopfs_node *a = VP_TO_SNOOPFS(vp);
  fist_dprint(4, "FXN=%s FILE=%s LINE=%d\n",__FUNCTION__,__FILE__,__LINE__);
#ifdef notyet
  /*
   * Can't do this check because vop_reclaim runs
   * with a funny vop vector.
   */
  if (vp->v_op != snoopfs_vnodeop_p) {
    printf ("snoopfs_checkvp: on non-snoopfs-node\n");
    while (snoopfs_checkvp_barrier) /*WAIT*/ ;
    panic("snoopfs_checkvp");
  };
#endif
  if (a->snoopfs_lowervp == NULLVP) {
    /* Should never happen */
    int i;
    u_long *p;
    printf("vp = %p, ZERO ptr\n", (void *)vp);
    for (p = (u_long *) a, i = 0; i < 8; i++)
      printf(" %lx", p[i]);
    printf("\n");
    /* wait for debugger */
    while (snoopfs_checkvp_barrier) /*WAIT*/ ;
    panic("snoopfs_checkvp");
  }
  if (vrefcnt(a->snoopfs_lowervp) < 1) {
    int i;
    u_long *p;
    printf("vp = %p, unref'ed lowervp\n", (void *)vp);
    for (p = (u_long *) a, i = 0; i < 8; i++)
      printf(" %lx", p[i]);
    printf("\n");
    /* wait for debugger */
    while (snoopfs_checkvp_barrier) /*WAIT*/ ;
       panic ("null with unref'ed lowervp");
  };
#ifdef notyet
  printf("null %x/%d -> %x/%d [%s, %d]\n",
	 SNOOPFS_TO_VP(a), vrefcnt(SNOOPFS_TO_VP(a)),
	 a->snoopfs_lowervp, vrefcnt(a->snoopfs_lowervp),fil, lno);
#endif
  return a->snoopfs_lowervp;
}
#endif

/****************************************************************************/
/* ADDED BY EZK */

/* allocate memory and zero it */
void *
kmem_zalloc(unsigned long size)
{
  void *addr;

  fist_dprint(4, "FXN=%s FILE=%s LINE=%d\n",__FUNCTION__,__FILE__,__LINE__);
  addr = malloc(size, M_TEMP, M_WAITOK);
  if (addr)
    bzero(addr, size);
  print_location();
  return addr;
}

/* do an I/O move */
int
fist_uiomove(caddr_t cp, int n, enum uio_rw rwflag, struct uio *uio)
{
  enum uio_rw saved_rwflag;
  int ret;

  fist_dprint(4, "FXN=%s FILE=%s LINE=%d\n",__FUNCTION__,__FILE__,__LINE__);
  if (uio->uio_rw != rwflag) {
    printf("UIOMOVE mismatching flags: uio->uio_rw=%d, rwflag=%d\n",
	   uio->uio_rw, rwflag);
  }
  /* save uio's rwflag */
  saved_rwflag = uio->uio_rw;
  uio->uio_rw = rwflag;
  ret = uiomove(cp, n, uio);
  uio->uio_rw = saved_rwflag;
  print_location();
  return ret;
}


