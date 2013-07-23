/* librepo - A library providing (libcURL like) API to downloading repository
 * Copyright (C) 2012  Tomas Mlcoch
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

#define _XOPEN_SOURCE 600
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "rcodes.h"
#include "util.h"
#include "repomd.h"
#include "yum.h"
#include "handle.h"
#include "result.h"

int
lr_repoutil_yum_check_repo(const char *path, GError **err)
{
    int rc;
    lr_Handle h;
    lr_Result *result;

    assert(path);
    assert(!err || *err == NULL);

    h = lr_handle_init();
    result = lr_result_init();

    if ((rc = lr_handle_setopt(h, LRO_REPOTYPE, LR_YUMREPO)) != LRE_OK) {
        g_set_error(err, LR_REPOUTIL_YUM_ERROR, rc,
                    "lr_handle_setopt(, LRO_REPOTYPE, LR_YUMREPO) error: %s",
                    lr_strerror(rc));
        return rc;
    }

    if ((rc = lr_handle_setopt(h, LRO_URL, path)) != LRE_OK) {
        g_set_error(err, LR_REPOUTIL_YUM_ERROR, rc,
                    "lr_handle_setopt(, LRO_URL, %s) error: %s",
                    path, lr_strerror(rc));
        return rc;
    }

    if ((rc = lr_handle_setopt(h, LRO_CHECKSUM, 1)) != LRE_OK) {
        g_set_error(err, LR_REPOUTIL_YUM_ERROR, rc,
                    "lr_handle_setopt(, LRO_CHECKSUM, 1) error: %s",
                    lr_strerror(rc));
        return rc;
    }

    if ((rc = lr_handle_setopt(h, LRO_LOCAL, 1)) != LRE_OK) {
        g_set_error(err, LR_REPOUTIL_YUM_ERROR, rc,
                    "lr_handle_setopt(, LRO_LOCAL, 1) error: %s",
                    lr_strerror(rc));
        return rc;
    }

    rc = lr_handle_perform(h, result, err);

    lr_result_free(result);
    lr_handle_free(h);

    return rc;
}

int
lr_repoutil_yum_parse_repomd(const char *in_path,
                             lr_YumRepoMd *repomd,
                             GError **err)
{
    int fd, rc;
    struct stat st;
    char *path;

    assert(in_path);
    assert(!err || *err == NULL);

    if (stat(in_path, &st) != 0) {
        g_set_error(err, LR_REPOUTIL_YUM_ERROR, LRE_IO,
                    "stat(%s,) error: %s", in_path, strerror(errno));
        return LRE_IO;
    }

    if (st.st_mode & S_IFDIR)
        path = lr_pathconcat(in_path, "repodata/repomd.xml", NULL);
    else
        path = lr_strdup(in_path);

    fd = open(path, O_RDONLY);
    if (fd < 0) {
        g_set_error(err, LR_REPOUTIL_YUM_ERROR, LRE_IO,
                    "open(%s, O_RDONLY) error: %s", path, strerror(errno));
        lr_free(path);
        return LRE_IO;
    }

    lr_free(path);

    rc = lr_yum_repomd_parse_file(repomd, fd, NULL, NULL, err);
    close(fd);

    return rc;
}
