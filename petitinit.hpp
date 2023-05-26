/*
    This file is part of petitinit.
    Copyright (C) 2023 SudoMaker, Ltd.
    Author: Reimu NotMoe <reimu@sudomaker.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/*
    SudoMaker, Ltd. hereby grants you additional permissions under section 7 of
    the GNU Affero General Public License version 3 ("AGPLv3") in accordance
    with the following terms:

    1. You are permitted to modify and distribute this program or any
    covered work without being required to distribute the source code of your
    modifications, provided that the resulting executable (binary code) is
    exclusively designed and intended to run on (a) hardware devices
    manufactured by SudoMaker, Ltd., (b) hardware components manufactured by
    SudoMaker, Ltd. which are parts of a larger hardware device, or (c)
    hardware devices that are certified through the "Resonance Certification
    Program" by SudoMaker, Ltd.

    2. This additional permission is granted solely for the purpose of enabling
    the use of this program on the aforementioned hardware devices or
    components, and does not extend to any other use or distribution of this
    program.

    3. If you distribute or make available any executable (binary code) that
    was modified under the terms of this additional permission, you must state
    that it is distributed or made available solely for use on the
    aforementioned hardware devices or components.

    4. These additional permissions are granted to you in conjunction with the
    AGPLv3 and does not affect other rights or obligations under that license.

    By exercising any of the additional permissions granted here, you agree to
    be bound by the terms and conditions mentioned above.
*/

#pragma once

#include <cassert>
#include <cinttypes>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <initializer_list>

#include <fcntl.h>
#include <unistd.h>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/mount.h>

#include <linux/magic.h>

using callback_list_dir = void(*)(const char *, struct stat *);

extern void list_dir(const char *path, callback_list_dir cb, bool one_dev, dev_t root_dev);
extern void list_dir(const char *path, callback_list_dir cb, bool one_dev = false);

extern int wait_path_exist(const char *path, unsigned msecs);

extern void create_hier();

extern int mount_it(const char *sf, const char *d, const char *ft = nullptr, int fl = 0, void *dt = nullptr);
extern bool fs_is_mounted(const char *dir_path, unsigned long fs_magic);
extern int mount_big_three(const char *path);
extern int umount_big_three(const char *path);

extern unsigned string_split(char *str, unsigned str_len, const char *delim, unsigned delim_len, char **output, unsigned output_len);
extern unsigned string_split(char *str, const char *delim, char **output, unsigned output_len);
extern const char *string_match_prefix(const char *cmd_option, const char *option_prefix);
extern void string_match_prefix(const char *cmd_option, const char *option_prefix, const char **output);

extern size_t read_all(int fd, void *buf, size_t buf_size);
extern size_t write_all(int fd, const void *buf, size_t buf_size);
extern ssize_t read_file(const char *path, void *buf, size_t buf_size);
extern ssize_t write_file(const char *path, const void *buf, size_t buf_size, int flags = O_WRONLY|O_CREAT|O_TRUNC, int mode = 0600);
