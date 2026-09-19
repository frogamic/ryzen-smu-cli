#include "voltage.h"
#include "smu.h"

smu_return_val get_core_offset(smu_obj_t *obj, unsigned int core, int *offset) {
	int arg = (((core & 8) << 5) | (core & 7)) << 20;

	return smu_send_mp1(obj, 0x48, arg, offset);
}

smu_return_val set_core_offset(smu_obj_t *obj, unsigned int core, int offset) {
	int arg = ((core & 8) << 5 | (core & 7)) << 20 | (offset & 0xFFFF);

	return smu_send_mp1(obj, 0x35, arg, 0);
}

smu_return_val set_all_core_offset(smu_obj_t *obj, unsigned int core_count, int offset) {
	smu_return_val err;
	for (int i = 0; i < core_count; ++i) {
		err = set_core_offset(obj, i, offset);
		if (err != SMU_Return_OK)
			break;
	}
	return err;
}

smu_return_val reset_all_core_offset(smu_obj_t *obj) { return smu_send_mp1(obj, 0x36, 0, 0); }
