// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * Ryzen SMU Userspace Sensor Monitor
 * Copyright (C) 2020 Leonardo Gates <leogatesx9r@protonmail.com>
 * Copyright (C) 2026 Dominic Shelton <frogamic@protonmail.com>
 *
 * Modified 2026 by Dominic Shelton: adapted from userspace/monitor_cpu.c in
 * ryzen_smu for use in ryzen-smu-cli.
 *
 * This program is free software: you can redistribute it &&/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include <cpuid.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpustat.h"
#include "log.h"

static unsigned int append_u32_to_str(char *buffer, unsigned int buflen, unsigned int pos, unsigned int val) {
	return pos
				 + snprintf(
						 buffer + pos, buflen - pos, "%c%c%c%c", val & 0xff, val >> 8 & 0xff, val >> 16 & 0xff, val >> 24 & 0xff);
}

static const char *get_cpu_name() {
	unsigned int eax, ebx, ecx, edx, l;
	static char buffer[50] = {0}, *p;
	unsigned int pos = 0;

	VLOG(LOG_DEBUG, "Getting CPU name from cpuid");
	for (int i = 0x80000002; i <= 0x80000004; i += 1) {
		VLOG(LOG_TRACE, "Reading cpuid leaf: %#x", i);
		if (!__get_cpuid(i, &eax, &ebx, &ecx, &edx)) {
			VLOG(LOG_ERROR, "Could not read cpuid leaf: %#x", i);
		} else {
			pos = append_u32_to_str(buffer, sizeof(buffer), pos, eax);
			pos = append_u32_to_str(buffer, sizeof(buffer), pos, ebx);
			pos = append_u32_to_str(buffer, sizeof(buffer), pos, ecx);
			pos = append_u32_to_str(buffer, sizeof(buffer), pos, edx);
		}
	}
	VLOG(LOG_TRACE, "Read %u characters from cpu name leaves", pos);

	// Trim whitespaces
	p = buffer;
	l = strlen(p);
	while (isspace(p[l - 1]))
		p[--l] = 0;
	while (*p && isspace(*p))
		++p;

	return p;
}

static void get_cpu_cores(smu_obj_t *obj, unsigned int *cores, unsigned int *logical_cores) {
	unsigned int smt, eax, ebx, ecx, edx, fam, model, ccds_present, ccds_disabled, ccds_down, core_fuse, core_fuse_addr,
			ccd_fuse1, ccd_fuse2;

	VLOG(LOG_TRACE, "Reading cpuid core count at leaf 0x00000001");
	if (!__get_cpuid(0x00000001, &eax, &ebx, &ecx, &edx)) {
		VLOG(LOG_ERROR, "Could not read cpuid info at leaf 0x00000001");
		exit(-1);
	}
	fam = ((eax & 0xf00) >> 8) + ((eax & 0xff00000) >> 20);
	model = ((eax & 0xf0000) >> 12) + ((eax & 0xf0) >> 4);
	*logical_cores = (ebx >> 16) & 0xFF;

	ccd_fuse1 = 0x5D218;
	ccd_fuse2 = 0x5D21C;

	if (fam == 0x17 && model != 0x71) {
		ccd_fuse1 += 0x40;
		ccd_fuse2 += 0x40;
	}

	VLOG(LOG_TRACE, "Reading CPU CCD fuses");
	if (smu_read_smn_addr(obj, ccd_fuse1, &ccds_present) != SMU_Return_OK
			|| smu_read_smn_addr(obj, ccd_fuse2, &ccds_down) != SMU_Return_OK) {
		VLOG(LOG_ERROR, "Failed to read CCD fuses");
		exit(-1);
	}

	ccds_disabled = ((ccds_down & 0x3F) << 2) | ((ccds_present >> 30) & 0x3);

	ccds_present = (ccds_present >> 22) & 0xFF;

	if (fam == 0x19)
		core_fuse_addr = (0x30081800 + 0x598) | ((((ccds_disabled & ccds_present) & 1) == 1) ? 0x2000000 : 0);
	else
		core_fuse_addr = (0x30081800 + 0x238) | (((ccds_present & 1) == 0) ? 0x2000000 : 0);

	VLOG(LOG_TRACE, "Reading CPU core fuse");
	if (smu_read_smn_addr(obj, core_fuse_addr, &core_fuse) != SMU_Return_OK) {
		VLOG(LOG_ERROR, "Failed to read core fuse");
		exit(-1);
	}

	smt = (core_fuse & (1 << 8)) != 0;

	*cores = *logical_cores;
	if (smt)
		*cores /= 2;
}

void get_cpu_stat(smu_obj_t *obj, cpu_stat_t *stat) {
	get_cpu_cores(obj, &(stat->cores), &(stat->logical_cores));
	stat->name = get_cpu_name();
	stat->codename = smu_codename_to_str(obj);
	stat->smu_fw = smu_get_fw_version(obj);
}
