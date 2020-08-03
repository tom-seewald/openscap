/*
 * Copyright 2019 Red Hat Inc., Durham, North Carolina.
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
 *      "Jan Černý" <jcerny@redhat.com>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include "fsdev.h"
#include "fsdev.c"

#if defined(OS_LINUX) || defined(OS_AIX)
#include <mntent.h>

static int test_single_call()
{
	struct mntent ment = {0};
	ment.mnt_type = "autofs";
	int ret = is_local_fs(&ment);
	/* autofs entry is never considered local */
	return (ret == 0);
}

static int test_multiple_calls(const char *fake_mtab)
{
	/*
	 * fake mtab contains only 4 local filesystems:
	 * /, /tmp, /home and /proc
	 */
	FILE *f = setmntent(fake_mtab, "r");
	if (f == NULL) {
		fprintf(stderr, "fake_mtab could not be open\n");
		return 0;
	}
	struct mntent *ment;
	unsigned int locals = 0;
	while ((ment = getmntent(f)) != NULL) {
		if (is_local_fs(ment)) {
			locals++;
		}
	}
	endmntent(f);
	return (locals == 4);
}

int main(int argc, char *argv[])
{
	if (!test_single_call()) {
		fprintf(stderr, "test_single_call has failed\n");
		return 1;
	}
	if (!test_multiple_calls(argv[1])) {
		fprintf(stderr, "test_multiple_calls has failed\n");
		return 1;
	}
	return 0;
}

#else

int main(int argc, char *argv[])
{
	fprintf(stderr, "is_local is only defined and used by Linux and AIX\n");
	return 0;
}

#endif
