#ifndef SMU_H
#define SMU_H

#include <libsmu.h>

smu_return_val smu_send_mp1(smu_obj_t *obj, unsigned int op, int arg, int *ret);

#endif // SMU_H
