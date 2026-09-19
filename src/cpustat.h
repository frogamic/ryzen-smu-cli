#ifndef CPUSTAT_H
#define CPUSTAT_H

#include <libsmu.h>

typedef struct cpu_stat {
	char *name;
	char *codename;
	unsigned int cores;
	unsigned int logical_cores;
	char *smu_fw;
} cpu_stat_t;

void get_cpu_stat(smu_obj_t *obj, cpu_stat_t *stat);

#endif // CPUSTAT_H
