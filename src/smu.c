#include <libsmu.h>
#include <string.h>

#include "log.h"

#define MAX_SUPPORTED_OPS 10

static const unsigned int feature_matrix_mp1[CODENAME_COUNT][MAX_SUPPORTED_OPS] = {
		[CODENAME_MATISSE] = {0x35, 0x36, 0x48},
		[CODENAME_VERMEER] = {0x35, 0x36, 0x48},
};

static int feature_gate(smu_obj_t *obj, unsigned int op) {
	smu_processor_codename c = obj->codename;

	if (c >= CODENAME_COUNT || c <= CODENAME_UNDEFINED) {
		VLOG(LOG_ERROR, "CPU codename %d is unknown", c);
		return 1;
	}

	for (int i = 0; i < MAX_SUPPORTED_OPS && feature_matrix_mp1[c][i]; ++i)
		if (feature_matrix_mp1[c][i] == op)
			return 0;

	return 1;
}

smu_return_val smu_send_mp1(smu_obj_t *obj, unsigned int op, int arg, int *ret) {
	smu_arg_t args;
	smu_return_val err;

	memset(&args, 0, sizeof(args));
	args.args[0] = arg;

	if (feature_gate(obj, op)) {
		VLOG(LOG_WARN, "Operation %#x not supported on %s", op, smu_codename_to_str(obj));
		return SMU_Return_Unsupported;
	}

	VLOG(LOG_TRACE, "Sending command to MP1: %#x arg: %d", op, arg);
	err = smu_send_command(obj, op, &args, SMU_TYPE_MP1);
	VLOG(LOG_TRACE, "smu returned status: %s", smu_return_to_str(err));

	if (err != SMU_Return_OK)
		return err;

	VLOG(LOG_TRACE, "smu returned arg: %d", args.args[0]);

	if (ret != 0)
		*ret = args.args[0];
	else
		VLOG(LOG_TRACE, "smu return value dropped");

	return err;
}
