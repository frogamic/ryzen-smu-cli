#include <libsmu.h>
#include <string.h>

#include "log.h"

smu_return_val smu_send_mp1(smu_obj_t *obj, unsigned int op, int arg, int *ret) {
	smu_arg_t args;
	smu_return_val err;

	memset(&args, 0, sizeof(args));
	args.args[0] = arg;

	VLOG(LOG_DEBUG, "Sending command to MP1: %x arg: %d", op, arg);
	err = smu_send_command(obj, op, &args, SMU_TYPE_MP1);
	VLOG(LOG_DEBUG, "smu returned status: %s", smu_return_to_str(err));

	if (err != SMU_Return_OK)
		return err;

	VLOG(LOG_DEBUG, "smu returned arg: %d", args.args[0]);

	if (ret != 0)
		*ret = args.args[0];
	else
		VLOG(LOG_DEBUG, "smu return value dropped");

	return err;
}
