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

#include "petitinit.hpp"


void list_dir(const char *path, callback_list_dir cb, bool one_dev, dev_t root_dev) {
	struct dirent *entry;
	DIR *dir = opendir(path);

	if (dir == nullptr) {
		return;
	}

	while ((entry = readdir(dir)) != nullptr) {
		char fullpath[PATH_MAX];
		struct stat SB;
		strcpy(fullpath, path);
		strcat(fullpath, "/");
		strcat(fullpath, entry->d_name);

		lstat(fullpath, &SB);

		if (S_ISDIR(SB.st_mode)) {
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
				continue;
			}

			if (one_dev) {
				if (SB.st_dev != root_dev) {
					continue;
				}
			}

			list_dir(fullpath, cb, one_dev, root_dev);
		}

		cb(fullpath, &SB);
	}

	closedir(dir);
}

void list_dir(const char *path, callback_list_dir cb, bool one_dev) {
	struct stat root_stat;

	if (one_dev) {
		if (stat(path, &root_stat) != 0) {
			perror("stat");
			return;
		}
	}

	list_dir(path, cb, one_dev, root_stat.st_dev);
}

void create_hier() {
	for (auto &it : {"/sys", "/dev", "/proc", "/new_root"}) {
		if (mkdir(it, 0755)) {
			if (errno != EEXIST) {
				perror(it);
			}
		}
	}
}

int mount_it(const char *special_file, const char *dir, const char *fstype, int flags, void *data) {
	auto try_mount = [](const char *sf, const char *d, const char *ft, int fl, void *dt) {
		int n;
		int cur_flags = 0;

retry:
		n = mount(sf, d, ft, cur_flags, dt);

		if (n == 0)
			return 0;

		if (errno == EACCES || errno == EROFS) {
			if (!(cur_flags & MS_RDONLY)) {
				cur_flags |= MS_RDONLY;
				goto retry;
			}
		}

		return -1;
	};

	if (fstype) {
		return try_mount(special_file, dir, fstype, flags, data);
	} else {
		const char *fs_types[] = {"squashfs", "jfs", "jffs2", "f2fs", "ext4"};

		for (auto &it : fs_types) {
			if (0 == try_mount(special_file, dir, it, flags, data)) {
				return 0;
			}
		}

		return -1;
	}
}

bool fs_is_mounted(const char *dir_path, unsigned long fs_magic) {
	struct statfs fs_info;

	if (statfs(dir_path, &fs_info) == 0) {
//		printf("magic: %lx\n", fs_info.f_type);
		if (fs_info.f_type == fs_magic) {
			return true;
		}
	}

	return false;
}

int wait_path_exist(const char *path, unsigned msecs) {
	while (msecs) {
		if (access(path, F_OK) == 0)
			return 0;

		usleep(1000);
		msecs--;
	}

	if (access(path, F_OK) == 0)
		return 0;

	return -1;
}

int mount_big_three(const char *path) {
	auto path_len = strlen(path);
	char s[path_len + 64];

	memcpy(s, path, path_len);

	if (path[path_len-1] != '/') {
		s[path_len] = '/';
		path_len++;
	}

	const char *sft_list[] = {"proc", "sysfs", "devtmpfs"};
	const char *dir_list[] = {"proc", "sys", "dev"};
	const unsigned long fm_list[] = {PROC_SUPER_MAGIC, SYSFS_MAGIC, TMPFS_MAGIC};

	for (unsigned i=0; i<3; i++) {
		s[path_len] = 0;
		strcat(s, dir_list[i]);
		if (!fs_is_mounted(s, fm_list[i])) {
			if (mount(sft_list[i], s, sft_list[i], 0, nullptr)) {
				perror(s);
				return -1;
			}
		}
	}

	return 0;
}

int umount_big_three(const char *path) {
	auto path_len = strlen(path);
	char s[path_len + 64];

	memcpy(s, path, path_len);

	if (path[path_len-1] != '/') {
		s[path_len] = '/';
		path_len++;
	}

	const char *dir_list[] = {"proc", "sys", "dev"};

	for (auto &it : dir_list) {
		s[path_len] = 0;
		strcat(s, it);
		if (umount(s)) {
			if (errno != EINVAL) {
				perror(s);
				return -1;
			}
		}
	}

	return 0;
}

unsigned string_split(char *str, unsigned str_len, const char *delim, unsigned delim_len, char **output, unsigned output_len) {
	unsigned cnt = 0;
	unsigned start = 0, end = 0;

	while (cnt < output_len) {
		output[cnt] = str + start;
		cnt++;

		auto *end_ptr = (char *)memmem(str + start, str_len - start, delim, delim_len);
		if (!end_ptr) {
			break;
		} else {
			*end_ptr = 0;
			end = end_ptr - str;
			start = end + delim_len;
		}
	}

	return cnt;
}

unsigned string_split(char *str, const char *delim, char **output, unsigned output_len) {
	return string_split(str, strlen(str), delim, strlen(delim), output, output_len);
}

const char *string_match_prefix(const char *cmd_option, const char *option_prefix) {
	auto op_len = strlen(option_prefix);

	if (0 == strncmp(cmd_option, option_prefix, op_len)) {
		return cmd_option + op_len;
	}

	return nullptr;
}

void string_match_prefix(const char *cmd_option, const char *option_prefix, const char **output) {
	auto *o = string_match_prefix(cmd_option, option_prefix);

	if (o) {
		*output = o;
	}
}

size_t read_all(int fd, void *buf, size_t buf_size) {
	size_t bytes_read = 0;
	ssize_t n;

	while ((n = read(fd, (uint8_t *)buf + bytes_read, buf_size - bytes_read)) > 0) {
		bytes_read += n;
	}

	return bytes_read;
}

size_t write_all(int fd, const void *buf, size_t buf_size) {
	ssize_t bytes_written = 0;

	while (bytes_written < buf_size) {
		ssize_t n = write(fd, (const uint8_t *)buf + bytes_written, buf_size - bytes_written);
		if (n > 0) {
			bytes_written += n;
		} else {
			break;
		}
	}

	return bytes_written;
}

ssize_t read_file(const char *path, void *buf, size_t buf_size) {
	int fd = open(path, O_RDONLY | O_CLOEXEC);

	if (fd < 0) {
		perror(path);
		return fd;
	}

	auto ret = (ssize_t)read_all(fd, buf, buf_size);
	close(fd);
	return ret;
}

ssize_t write_file(const char *path, const void *buf, size_t buf_size, int flags, int mode) {
	int fd = open(path, flags, mode);

	if (fd < 0) {
		perror(path);
		return fd;
	}

	auto ret = (ssize_t)write_all(fd, buf, buf_size);
	close(fd);
	return ret;
}
