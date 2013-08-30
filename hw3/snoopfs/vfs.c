/*
 * Copyright (c) 1992, 1993, 1995
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
 *	@(#)vfs.c	8.2 (Berkeley) 1/21/94
 *
 * @(#)lofs_vfsops.c	1.2 (Berkeley) 6/18/92
 * $FreeBSD: src/sys/fs/snoopfs/vfs.c,v 1.55 2002/09/25 02:28:07 jeff Exp $
 */

/*
 * Wrapfs Layer
 * (See snoopfs_vfsops.c for a description of what this does.)
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
#include <sys/malloc.h>
#include <sys/mount.h>
#include <sys/namei.h>
#include <sys/proc.h>
#include <sys/vnode.h>

#include <fs/snoopfs/snoopfs.h>
#endif

static MALLOC_DEFINE(M_SNOOPFSMNT, "SNOOPFS mount", "SNOOPFS mount structure");

static int	snoopfs_fhtovp(struct mount *mp, struct fid *fidp,
				   struct vnode **vpp);
static int	snoopfs_checkexp(struct mount *mp, struct sockaddr *nam,
				    int *extflagsp, struct ucred **credanonp);
static int	snoopfs_mount(struct mount *mp, struct thread *td);
static int	snoopfs_quotactl(struct mount *mp, int cmd, uid_t uid,
				     caddr_t arg, struct thread *td);
static int	snoopfs_root(struct mount *mp, struct vnode **vpp, struct thread *td);
static int	snoopfs_start(struct mount *mp, int flags, struct thread *td);
static int	snoopfs_statfs(struct mount *mp, struct statfs *sbp,
				   struct thread *td);
static int	snoopfs_sync(struct mount *mp, int waitfor,
				 struct ucred *cred, struct thread *td);
static int	snoopfs_unmount(struct mount *mp, int mntflags, struct thread *td);
static int	snoopfs_vget(struct mount *mp, ino_t ino, int flags,
				 struct vnode **vpp);
static int	snoopfs_vptofh(struct vnode *vp, struct fid *fhp);
static int	snoopfs_extattrctl(struct mount *mp, int cmd,
				       struct vnode *filename_vp,
				       int namespace, const char *attrname,
				       struct thread *td);

/*
 * Mount snoopfs layer
 */
static int
snoopfs_mount(mp, td)
	struct mount *mp;
	struct thread *td;
{
  struct nameidata nd;
  int error = 0;
  struct vnode *lowerrootvp, *vp;
  struct vnode *snoopfsm_rootvp;
  struct snoopfs_mount *xmp;
  char *target;
  size_t size;
  int isvnunlocked = 0, len;

  fist_dprint(2, "snoopfs_mount(mp = %p)\n", (void *)mp);

  /*
   * Update is a no-op
   */
  if (mp->mnt_flag & MNT_UPDATE) {
    return (EOPNOTSUPP);
    /* return VFS_MOUNT(MOUNT_TO_SNOOPFS_MOUNT(mp)->snoopfsm_vfs, path, data, ndp, td);*/
  }

  /*
   * Get argument
   */
  error = vfs_getopt(mp->mnt_optnew, "target", (void **)&target, &len);
  if (error || target[len - 1] != '\0')
    return (EINVAL);

  /*
   * Unlock lower node to avoid deadlock.
   * (XXX) VOP_ISLOCKED is needed?
   */
  if ((mp->mnt_vnodecovered->v_op == snoopfs_vnodeop_p) &&
      VOP_ISLOCKED(mp->mnt_vnodecovered, NULL)) {
    VOP_UNLOCK(mp->mnt_vnodecovered, 0, td);
    isvnunlocked = 1;
  }
  /*
   * Find lower node
   */
  NDINIT(&nd, LOOKUP, FOLLOW|WANTPARENT|LOCKLEAF,
	 UIO_SYSSPACE, target, td);
  error = namei(&nd);
  /*
   * Re-lock vnode.
   */
  if (isvnunlocked && !VOP_ISLOCKED(mp->mnt_vnodecovered, NULL))
    vn_lock(mp->mnt_vnodecovered, LK_EXCLUSIVE | LK_RETRY, td);

  if (error)
    return (error);
  NDFREE(&nd, NDF_ONLY_PNBUF);

  /*
   * Sanity check on lower vnode
   */
  lowerrootvp = nd.ni_vp;

  vrele(nd.ni_dvp);
  nd.ni_dvp = NULLVP;

  /*
   * Check multi snoopfs mount to avoid `lock against myself' panic.
   */
  if (lowerrootvp == VP_TO_SNOOPFS(mp->mnt_vnodecovered)->snoopfs_lowervp) {
    fist_dprint(2, "snoopfs_mount: multi snoopfs mount?\n");
    vput(lowerrootvp);
    return (EDEADLK);
  }

  xmp = (struct snoopfs_mount *) malloc(sizeof(struct snoopfs_mount),
				M_SNOOPFSMNT, M_WAITOK);	/* XXX */

  /*
   * Save reference to underlying FS
   */
  xmp->snoopfsm_vfs = lowerrootvp->v_mount;

  /*
   * Save reference.  Each mount also holds
   * a reference on the root vnode.
   */
  error = snoopfs_nodeget(mp, lowerrootvp, &vp);
  /*
   * Make sure the node alias worked
   */
  if (error) {
    VOP_UNLOCK(vp, 0, td);
    vrele(lowerrootvp);
    free(xmp, M_SNOOPFSMNT);	/* XXX */
    return (error);
  }

  /*
   * Keep a held reference to the root vnode.
   * It is vrele'd in snoopfs_unmount.
   */
  snoopfsm_rootvp = vp;
  snoopfsm_rootvp->v_vflag |= VV_ROOT;
  xmp->snoopfsm_rootvp = snoopfsm_rootvp;

  /*
   * Unlock the node (either the lower or the alias)
   */
  VOP_UNLOCK(vp, 0, td);

  if (SNOOPFS_VP_TO_LOWERVP(snoopfsm_rootvp)->v_mount->mnt_flag & MNT_LOCAL)
    mp->mnt_flag |= MNT_LOCAL;
  mp->mnt_data = (qaddr_t) xmp;
  vfs_getnewfsid(mp);

  (void) copystr(target, mp->mnt_stat.f_mntfromname,
		 MNAMELEN - 1, &size);
  bzero(mp->mnt_stat.f_mntfromname + size, MNAMELEN - size);
  (void)snoopfs_statfs(mp, &mp->mnt_stat, td);
  fist_dprint(2, "snoopfs_mount: lower %s, alias at %s\n",
	      mp->mnt_stat.f_mntfromname, mp->mnt_stat.f_mntonname);
  print_location();
  return (0);
}

/*
 * VFS start.  Nothing needed here - the start routine
 * on the underlying filesystem will have been called
 * when that filesystem was mounted.
 */
static int
snoopfs_start(mp, flags, td)
	struct mount *mp;
	int flags;
	struct thread *td;
{
	return (0);
	/* return VFS_START(MOUNTTOSNOOPFSMOUNT(mp)->snoopfsm_vfs, flags, td); */
}

/*
 * Free reference to snoopfs layer
 */
static int
snoopfs_unmount(mp, mntflags, td)
	struct mount *mp;
	int mntflags;
	struct thread *td;
{
  void *mntdata;
  int error;
  int flags = 0;

  fist_dprint(2, "snoopfs_unmount: mp = %p\n", (void *)mp);

  if (mntflags & MNT_FORCE)
    flags |= FORCECLOSE;

  /* There is 1 extra root vnode reference (snoopfsm_rootvp). */
  error = vflush(mp, 1, flags, curthread);
  if (error)
    return (error);

  /*
   * Finally, throw away the snoopfs_mount structure
   */
  mntdata = mp->mnt_data;
  mp->mnt_data = 0;
  free(mntdata, M_SNOOPFSMNT);
  print_location();
  return 0;
}

static int
snoopfs_root(mp, vpp, td)
	struct mount *mp;
	struct vnode **vpp;
	struct thread *td;
{
  td = curthread;	/* XXX */
  struct vnode *vp;

  fist_dprint(2, "snoopfs_root(mp = %p, vp = %p->%p)\n", (void *)mp,
	      (void *)MOUNT_TO_SNOOPFS_MOUNT(mp)->snoopfsm_rootvp,
	      (void *)SNOOPFS_VP_TO_LOWERVP(MOUNT_TO_SNOOPFS_MOUNT(mp)->snoopfsm_rootvp));

  /*
   * Return locked reference to root.
   */
  vp = MOUNT_TO_SNOOPFS_MOUNT(mp)->snoopfsm_rootvp;
  VREF(vp);

  vn_lock(vp, LK_EXCLUSIVE | LK_RETRY, td);
  *vpp = vp;
  print_location();
  return 0;
}

static int
snoopfs_quotactl(mp, cmd, uid, arg, td)
	struct mount *mp;
	int cmd;
	uid_t uid;
	caddr_t arg;
	struct thread *td;
{
  return VFS_QUOTACTL(MOUNT_TO_SNOOPFS_MOUNT(mp)->snoopfsm_vfs, cmd, uid, arg, td);
}

static int
snoopfs_statfs(mp, sbp, td)
	struct mount *mp;
	struct statfs *sbp;
	struct thread *td;
{
  int error;
  struct statfs mstat;

  fist_dprint(2, "snoopfs_statfs(mp = %p, vp = %p->%p)\n", (void *)mp,
	      (void *)MOUNT_TO_SNOOPFS_MOUNT(mp)->snoopfsm_rootvp,
	      (void *)SNOOPFS_VP_TO_LOWERVP(MOUNT_TO_SNOOPFS_MOUNT(mp)->snoopfsm_rootvp));

  bzero(&mstat, sizeof(mstat));

  error = VFS_STATFS(MOUNT_TO_SNOOPFS_MOUNT(mp)->snoopfsm_vfs, &mstat, td);
  if (error)
    return (error);

  /* now copy across the "interesting" information and fake the rest */
  sbp->f_type = mstat.f_type;
  sbp->f_flags = mstat.f_flags;
  sbp->f_bsize = mstat.f_bsize;
  sbp->f_iosize = mstat.f_iosize;
  sbp->f_blocks = mstat.f_blocks;
  sbp->f_bfree = mstat.f_bfree;
  sbp->f_bavail = mstat.f_bavail;
  sbp->f_files = mstat.f_files;
  sbp->f_ffree = mstat.f_ffree;
  if (sbp != &mp->mnt_stat) {
    bcopy(&mp->mnt_stat.f_fsid, &sbp->f_fsid, sizeof(sbp->f_fsid));
    bcopy(mp->mnt_stat.f_mntonname, sbp->f_mntonname, MNAMELEN);
    bcopy(mp->mnt_stat.f_mntfromname, sbp->f_mntfromname, MNAMELEN);
  }
  print_location();
  return (0);
}

static int
snoopfs_sync(mp, waitfor, cred, td)
	struct mount *mp;
	int waitfor;
	struct ucred *cred;
	struct thread *td;
{
  /*
   * XXX - Assumes no data cached at snoopfs layer.
   */
  return (0);
}

static int
snoopfs_vget(mp, ino, flags, vpp)
	struct mount *mp;
	ino_t ino;
	int flags;
	struct vnode **vpp;
{
  int error;
  fist_dprint(2, "FXN=%s FILE=%s LINE=%d\n",__FUNCTION__,__FILE__,__LINE__);
  error = VFS_VGET(MOUNT_TO_SNOOPFS_MOUNT(mp)->snoopfsm_vfs, ino, flags, vpp);
  if (error)
    return (error);

  return (snoopfs_nodeget(mp, *vpp, vpp));
}

static int
snoopfs_fhtovp(mp, fidp, vpp)
	struct mount *mp;
	struct fid *fidp;
	struct vnode **vpp;
{
  int error;
  fist_dprint(2, "FXN=%s FILE=%s LINE=%d\n",__FUNCTION__,__FILE__,__LINE__);
  error = VFS_FHTOVP(MOUNT_TO_SNOOPFS_MOUNT(mp)->snoopfsm_vfs, fidp, vpp);
  if (error)
    return (error);

  return (snoopfs_nodeget(mp, *vpp, vpp));
}

static int
snoopfs_checkexp(mp, nam, extflagsp, credanonp)
        struct mount *mp;
	struct sockaddr *nam;
	int *extflagsp;
	struct ucred **credanonp;
{
  fist_dprint(2, "FXN=%s FILE=%s LINE=%d\n",__FUNCTION__,__FILE__,__LINE__);
  return VFS_CHECKEXP(MOUNT_TO_SNOOPFS_MOUNT(mp)->snoopfsm_vfs, nam,
		      extflagsp, credanonp);
}

static int
snoopfs_vptofh(vp, fhp)
	struct vnode *vp;
	struct fid *fhp;
{
  fist_dprint(2, "FXN=%s FILE=%s LINE=%d\n",__FUNCTION__,__FILE__,__LINE__);
  return VFS_VPTOFH(SNOOPFS_VP_TO_LOWERVP(vp), fhp);
}

static int
snoopfs_extattrctl(mp, cmd, filename_vp, namespace, attrname, td)
	struct mount *mp;
	int cmd;
	struct vnode *filename_vp;
	int namespace;
	const char *attrname;
	struct thread *td;
{
  fist_dprint(2, "FXN=%s FILE=%s LINE=%d\n",__FUNCTION__,__FILE__,__LINE__);
  return VFS_EXTATTRCTL(MOUNT_TO_SNOOPFS_MOUNT(mp)->snoopfsm_vfs, cmd, filename_vp,
			namespace, attrname, td);
}

static struct vfsops snoopfs_vfsops = {
	snoopfs_mount,
	NULL,
	snoopfs_start,
	snoopfs_unmount,
	snoopfs_root,
	snoopfs_quotactl,
	snoopfs_statfs,
	snoopfs_sync,
	snoopfs_vget,
	snoopfs_fhtovp,
	snoopfs_checkexp,
	snoopfs_vptofh,
	snoopfs_init,
	snoopfs_uninit,
	snoopfs_extattrctl,
};

VFS_SET(snoopfs_vfsops, snoopfs, VFCF_LOOPBACK);
