#ifndef CPUSTAT_H
#define CPUSTAT_H

#include <libsmu.h>

typedef struct cpu_stat {
	const char *name;
	const char *codename;
	unsigned int cores;
	unsigned int logical_cores;
	const char *smu_fw;
} cpu_stat_t;

void get_cpu_stat(smu_obj_t *obj, cpu_stat_t *stat);

#endif // CPUSTAT_H
