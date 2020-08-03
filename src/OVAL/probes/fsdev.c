/**
 * @file   fsdev.c
 * @brief  fsdev API implementation
 * @author "Daniel Kopecek" <dkopecek@redhat.com>
 *
 */
/*
 * Copyright 2009 Red Hat Inc., Durham, North Carolina.
 * All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * Authors:
 *      "Daniel Kopecek" <dkopecek@redhat.com>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#if defined(OS_LINUX)
# include <mntent.h>
# include <unistd.h>
#elif defined(OS_SOLARIS)
# include <sys/mnttab.h>
# include <sys/mntent.h>
# include <sys/unistd.h>
#elif defined(OS_FREEBSD) || defined(OS_APPLE)
# include <sys/param.h>
# include <sys/ucred.h>
# include <sys/mount.h>
#elif defined(OS_AIX)
# include <unistd.h>
# include <mntent.h>
# include <fshelp.h>
# include <sys/vfs.h>
# include <sys/vmount.h>
# define _PATH_MOUNTED MOUNTED
#else
# error "Sorry, your OS isn't supported."
#endif

#include "fsdev.h"

/**
 * Compare two dev_t variables.
 */
static int fsdev_cmp(const void *a, const void *b)
{
	return memcmp(a, b, sizeof(dev_t));
}

#if defined(OS_LINUX) || defined(OS_AIX)

#define DEVID_ARRAY_SIZE 16
#define DEVID_ARRAY_ADD  8

#if defined(OS_LINUX)
static int is_local_fs(struct mntent *ment)
{
// todo: would it be usefull to provide the choice during build-time?
#if 1
	char *s;

	/*
	 * When type of the filesystem is autofs, it means the mtab entry
	 * describes the autofs configuration, which means ment->mnt_fsname
	 * is a path to the relevant autofs map, eg. /etc/auto.misc. In this
	 * situation, the following code which analyses ment->mnt_type would
	 * not work. When the filesystem handled by autofs is mounted, there
	 * is another different entry in mtab which contains the real block
	 * special device or remote filesystem in ment->mnt_fsname, and that
	 * will be parsed in a different call of this function.
	 */
	if (!strcmp(ment->mnt_type, "autofs")) {
		return 0;
	}

	if (ment->mnt_fsname == NULL) {
		return 0;
	}

	s = ment->mnt_fsname;
	/* If the fsname begins with "//", it is probably CIFS. */
	if (s[0] == '/' && s[1] == '/')
		return 0;

	/* If there's a ':' in the fsname and it occurs before any
	 * '/', then this is probably NFS and the file system is
	 * considered "remote".
	 */
	s = strpbrk(s, "/:");
	if (s && *s == ':')
		return 0;

	return 1;
#else
	struct stat st;

	/* If the file system is not backed-up by a real file, it is
	   considered remote. A notable exception is "tmpfs" to allow
	   traversal of /tmp et al. */
	if (strcmp(ment->mnt_fsname, "tmpfs") != 0
	    && (stat(ment->mnt_fsname, &st) != 0
		|| !(S_ISBLK(st.st_mode))))
		return 0;
	else
		return 1;
#endif
}

#elif defined(OS_AIX)
static int is_local_fs(struct mntent *ment)
{
	int i;
	struct vfs_ent *e;

	static const int remote_fs_types[] = {
		MNT_NFS,
		MNT_NFS3,
		MNT_AUTOFS, /* remote? */
		MNT_NFS4,
		MNT_RFS4,
		MNT_CIFS,
		MNT_BADVFS  /* end mark */
	};

	for (i = 0; remote_fs_types[i] != MNT_BADVFS; i++) {
		e = getvfsbytype(remote_fs_types[i]);
		if (e != NULL
		    && e->vfsent_name != NULL
		    && strcmp(ment->mnt_type, e->vfsent_name) == 0) {
			return 0;
		}
	}

	return 1;
}

#endif /* OS_AIX */

#if defined(OS_LINUX) || defined(OS_AIX)
static fsdev_t *__fsdev_init(fsdev_t *lfs)
{
	int e;
	FILE *fp;
	size_t i;

	struct mntent *ment;
	struct stat st;

	fp = setmntent(_PATH_MOUNTED, "r");
	if (fp == NULL) {
		e = errno;
		free(lfs);
		errno = e;
		return (NULL);
	}

	lfs->ids = malloc(sizeof(dev_t) * DEVID_ARRAY_SIZE);

	if (lfs->ids == NULL) {
		e = errno;
		free(lfs);
		endmntent(fp);
		errno = e;
		return (NULL);
	}

	lfs->cnt = DEVID_ARRAY_SIZE;
	i = 0;

	while ((ment = getmntent(fp)) != NULL) {
		if (!is_local_fs(ment))
			continue;
		if (stat(ment->mnt_dir, &st) != 0)
			continue;
		if (i >= lfs->cnt) {
			lfs->cnt += DEVID_ARRAY_ADD;
			lfs->ids = realloc(lfs->ids, sizeof(dev_t) * lfs->cnt);
		}
		memcpy(&(lfs->ids[i++]), &st.st_dev, sizeof(dev_t));
	}

	endmntent(fp);

	lfs->ids = realloc(lfs->ids, sizeof(dev_t) * i);
	lfs->cnt = (lfs->ids == NULL ? 0 : i);

	return (lfs);
}
#endif

#elif defined(OS_FREEBSD) || defined(OS_APPLE)
static fsdev_t *__fsdev_init(fsdev_t *lfs)
{
	struct statfs *mntbuf = NULL;
	struct stat st;
	int i;

	lfs->cnt = getmntinfo(&mntbuf, MNT_NOWAIT);
	lfs->ids = malloc(sizeof(dev_t) * lfs->cnt);

	for (i = 0; i < lfs->cnt; ++i) {
		if (stat(mntbuf[i].f_mntonname, &st) != 0)
			continue;

		memcpy(&(lfs->ids[i]), &st.st_dev, sizeof(dev_t));
	}

	if (i != lfs->cnt) {
		lfs->ids = realloc(lfs->ids, sizeof(dev_t) * i);
		lfs->cnt = i;
	}

	return (lfs);
}
#elif defined(OS_SOLARIS)

#define DEVID_ARRAY_SIZE 16
#define DEVID_ARRAY_ADD  8

static fsdev_t *__fsdev_init(fsdev_t *lfs)
{
	int e;
	FILE *fp;
	size_t i;

	struct mnttab mentbuf;
	struct stat st;

	fp = fopen(MNTTAB, "r");
	if (fp == NULL) {
		e = errno;
		free(lfs);
		errno = e;
		return (NULL);
	}

	lfs->ids = malloc(sizeof(dev_t) * DEVID_ARRAY_SIZE);

	if (lfs->ids == NULL) {
		e = errno;
		free(lfs);
                fclose(fp);
		errno = e;
		return (NULL);
	}

	lfs->cnt = DEVID_ARRAY_SIZE;
	i = 0;

	while ((getmntent(fp, &mentbuf)) == 0) {
		/* TODO: Is this check reliable? */
		if (stat(mentbuf.mnt_special, &st) == 0 && (st.st_mode & S_IFCHR)) {

			if (i >= lfs->cnt) {
				lfs->cnt += DEVID_ARRAY_ADD;
				lfs->ids = realloc(lfs->ids, sizeof(dev_t) * lfs->cnt);
			}

			memcpy(&(lfs->ids[i++]), &st.st_dev, sizeof(dev_t));
		}
	}

	fclose(fp);

	lfs->ids = realloc(lfs->ids, sizeof(dev_t) * i);
	lfs->cnt = (lfs->ids == NULL ? 0 : i);

	return (lfs);
}
#endif

fsdev_t *fsdev_init()
{
	fsdev_t *lfs;

	lfs = malloc(sizeof(fsdev_t));

	if (lfs == NULL)
		return (NULL);

	if (__fsdev_init(lfs) == NULL)
		return (NULL);

        if (lfs->ids != NULL && lfs->cnt > 1)
                qsort(lfs->ids, lfs->cnt, sizeof(dev_t), fsdev_cmp);

	return (lfs);
}

void fsdev_free(fsdev_t * lfs)
{
	if (lfs != NULL) {
		free(lfs->ids);
		free(lfs);
	}
	return;
}

int fsdev_search(fsdev_t * lfs, void *id)
{
	uint16_t w, s;
	int cmp;

	if (!lfs)
		return 1;

	w = lfs->cnt;
	s = 0;

	while (w > 0) {
		cmp = fsdev_cmp(id, &(lfs->ids[s + w / 2]));
		if (cmp > 0) {
			s += w / 2 + 1;
			w = w - w / 2 - 1;
		} else if (cmp < 0) {
			w = w / 2;
		} else {
			return (1);
		}
	}

	return (0);
}

int fsdev_path(fsdev_t * lfs, const char *path)
{
	struct stat st;

	if (stat(path, &st) != 0)
		return (-1);

	return fsdev_search(lfs, &st.st_dev);
}

int fsdev_fd(fsdev_t * lfs, int fd)
{
	struct stat st;

	if (fstat(fd, &st) != 0)
		return (-1);

	return fsdev_search(lfs, &st.st_dev);
}
