#include "voltage.h"
#include "log.h"
#include "smu.h"

smu_return_val get_core_offset(smu_obj_t *obj, unsigned int core, int *offset) {
	int arg = (((core & 8) << 5) | (core & 7)) << 20;

	VLOG(LOG_DEBUG, "Getting core offset from core %u", core);
	return smu_send_mp1(obj, 0x48, arg, offset);
}

smu_return_val set_core_offset(smu_obj_t *obj, unsigned int core, int offset) {
	int arg = ((core & 8) << 5 | (core & 7)) << 20 | (offset & 0xFFFF);

	VLOG(LOG_DEBUG, "Setting core offset on core %u to %d", core, offset);
	return smu_send_mp1(obj, 0x35, arg, 0);
}

smu_return_val set_all_core_offset(smu_obj_t *obj, unsigned int core_count, int offset) {
	if (core_count == 0) {
		VLOG(LOG_ERROR, "Core count must be greater than 0");
		return SMU_Return_InvalidArgument;
	}

	smu_return_val err;

	for (int i = 0; i < core_count; ++i) {
		err = set_core_offset(obj, i, offset);
		if (err != SMU_Return_OK) {
			VLOG(LOG_DEBUG, "Error setting core offset on core %d", i);
			break;
		}
	}
	return err;
}

smu_return_val reset_all_core_offset(smu_obj_t *obj) {
	VLOG(LOG_DEBUG, "Resetting core offset on all cores");
	return smu_send_mp1(obj, 0x36, 0, 0);
}
