#include <cpuid.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpustat.h"
#include "log.h"

static void append_u32_to_str(char *buffer, unsigned int val) {
	char tmp[12] = {0};

	sprintf(tmp, "%c%c%c%c", val & 0xff, val >> 8 & 0xff, val >> 16 & 0xff, val >> 24 & 0xff);
	strcat(buffer, tmp);
}

static const char *get_cpu_name() {
	unsigned int eax, ebx, ecx, edx, l;
	static char buffer[50] = {0}, *p;

	VLOG(LOG_DEBUG, "Getting CPU name from cpuid");
	for (int i = 0x80000002; i <= 0x80000004; i += 1) {
		VLOG(LOG_TRACE, "Reading cpuid leaf: %x", i);
		if (!__get_cpuid(i, &eax, &ebx, &ecx, &edx)) {
			VLOG(LOG_ERROR, "Could not read cpuid leaf: %x", i);
		} else {
			append_u32_to_str(buffer, eax);
			append_u32_to_str(buffer, ebx);
			append_u32_to_str(buffer, ecx);
			append_u32_to_str(buffer, edx);
		}
	}

	// Trim whitespaces
	p = buffer;
	l = strlen(p);
	while (isspace(p[l - 1]))
		p[--l] = 0;
	while (*p && isspace(*p))
		++p, --l;

	return buffer;
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
