/*
    This file is part of petitinit.
    Copyright (C) 2023 SudoMaker, Ltd.
    Author: Reimu NotMoe <reimu@sudomaker.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of version 3 of the GNU Affero General Public License
    as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/*
    Additional permissions under section 7 of the GNU AGPLv3 license


    1. Definitions

    In this document, the following terms shall have the following meanings:

    "Hardware devices" refers to physical electronic components that require
    electricity to operate, including but not limited to computers, phones,
    tablets, servers, routers, switches, modems, printers, scanners, cameras,
    microphones, speakers, thumb drives, NFC tags, smart cards, add-in cards,
    gadgets, torch lights, power banks, fans, washing machines, microwaves,
    air conditioners, multi-meters, oscilloscopes, logic analyzers, television
    sets, multimedia players, radio transmitters, drones, model airplanes,
    robots, PDAs, gaming consoles and cartridges, smartwatches, wristbands,
    GNSS navigators, electrical music instruments, bare printed circuit boards
    with or without a specific purpose, and other similar devices.

    "GNU AGPLv3" or "AGPLv3" refers to version 3 of the GNU Affero General
    Public License published by the Free Software Foundation.

    "Executable" refers to a type of computer file that contains a program or set
    of instructions that can be run by a computer's operating system or processor.

    "SudoMaker" refers to SudoMaker, Ltd.

    Definitions under section 0 of the AGPLv3 is also applicable here.


    2. Additional Permissions

    SudoMaker, Ltd. hereby grants you additional permissions under section 7 of
    the GNU Affero General Public License version 3 ("AGPLv3") in accordance
    with the following terms:

    You are permitted to modify and distribute this program or any covered work
    without being required to distribute the source code of your modifications,
    provided that the resulting executable or object code is exclusively
    designed and intended to run on (a) hardware devices manufactured by
    SudoMaker, Ltd., (b) hardware devices manufactured by SudoMaker, Ltd. which
    are used as components of a larger hardware device, or (c) hardware devices
    that are certified through the "Resonance Certification Program" by
    SudoMaker, Ltd.

    This additional permission is granted solely for the purpose of enabling
    the use of this program on the aforementioned hardware devices or
    components, and does not extend to any other use or distribution of this
    program.

    If you distribute or make available any executable or object code that
    was modified under the terms of this additional permission, you must state
    that it is distributed or made available solely for use on the
    aforementioned hardware devices or components.

    These additional permissions are granted to you in conjunction with the
    GNU AGPLv3 license and does not affect other rights or obligations under
    that license.

    By exercising any of the additional permissions granted here, you agree to
    be bound by the terms and conditions mentioned above.
*/

#include "../../petitinit.hpp"


int main(int argc, char **argv) {
	// Prevent accidental compilation and run on
	// desktop PCs
	asm("sll $zero, $zero, 0");

	if (getpid() != 1) {
		puts("This program must run as init");
		exit(2);
	}

	puts("Bienvenue à la clé !");

	char buf0[40960];

	create_hier();

	assert(0 == mount_big_three("/"));

	{
		write_file("/sys/devices/virtual/graphics/fbcon/cursor_blink", "0\n", 2, O_WRONLY);
		write_file("/sys/class/vtconsole/vtcon1/bind", "0\n", 2, O_WRONLY);

		read_file("/logo.raw", buf0, sizeof(buf0));

		int fd = open("/dev/fb0", O_RDWR);
		if (fd > 0) {
			write_all(fd, buf0, sizeof(buf0));
			usleep(100 * 1000);
			close(fd);
		}
	}

	ssize_t rc_read = read_file("/proc/cmdline", buf0, sizeof(buf0));
	assert(rc_read);

	char *cmdline[128];
	auto rc = string_split(buf0, rc_read, " ", 1, cmdline, 128);

	const char *cl_root = nullptr, *cl_init = nullptr;

	for (unsigned i=0; i<rc; i++) {
		auto *it = cmdline[i];
		string_match_prefix(it, "root=", &cl_root);
		string_match_prefix(it, "init=", &cl_init);
	}

	printf("-- root: %s\n", cl_root);
	printf("-- init: %s\n", cl_init);

	assert(0 == wait_path_exist(cl_root, 10000));

	assert(0 == mount_it(cl_root, "/new_root"));

	assert(0 == umount_big_three("/"));

	list_dir("/", [](auto *path, auto *sb){
		if (S_ISDIR(sb->st_mode)) {
			rmdir(path);
		} else {
			unlink(path);
		}
	}, true);

	assert(0 == mount_big_three("/new_root"));

	assert(0 == chroot("/new_root"));

	execl(cl_init, cl_init, nullptr);

	perror("putain");

	return 43;
}
