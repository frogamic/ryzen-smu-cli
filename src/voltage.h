#ifndef VOLTAGE_H
#define VOLTAGE_H

#include <libsmu.h>

smu_return_val get_core_offset(smu_obj_t *obj, unsigned int core, int *offset);
smu_return_val set_core_offset(smu_obj_t *obj, unsigned int core, int offset);
smu_return_val set_all_core_offset(smu_obj_t *obj, unsigned int core_count, int offset);
smu_return_val reset_all_core_offset(smu_obj_t *obj);

#endif // VOLTAGE_H
