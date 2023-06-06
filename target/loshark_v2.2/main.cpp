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
		exit(2);
	}

	create_hier();

	assert(0 == mount_big_three("/"));

	write_file("/sys/class/leds/red:indicator/brightness", "255\n", 4, O_WRONLY);

	const char *cl_root = "/dev/mmcblk0p1", *cl_init = "/linuxrc";

	assert(0 == wait_path_exist(cl_root, 10000));

	{
		pid_t pid;

		pid = fork(); // create a new process

		if (pid < 0) { // check for errors
			return 2;
		} else if (pid == 0) { // child process
			execl("/jfs_fsck", "jfs_fsck", cl_root, nullptr);
		}
		else { // parent process
			wait(nullptr);
		}
	}

	assert(0 == mount_it(cl_root, "/new_root", "jfs", MS_RDONLY));

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

	perror("fdp");

	return 43;
}
