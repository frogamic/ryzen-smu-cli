#include <argp.h>
#include <stdlib.h>
#include <unistd.h>

#include <libsmu.h>

#include "cpustat.h"
#include "log.h"
#include "voltage.h"

#ifndef VERSION
#define VERSION "unknown"
#endif

#ifndef TARGET
#define TARGET "rsmuctl"
#endif

static struct argp_option options[] = {{"verbose", 'v', 0, 0, "increase the verbosity level of the output"},
		{"voffset", 'o', "VALUE", 0, "per-core voltage offset, expressed in millivolts"},
		{"reset", 'r', 0, 0, "reset the voltage offset for all cores"}, {0}};

struct arguments {
	int setters, do_reset, do_voffset, voffset, verbosity;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
	struct arguments *args = state->input;

	switch (key) {
	case 'o':
		args->setters += 1;
		args->voffset = atoi(arg);
		args->do_voffset = 1;
		break;

	case 'r':
		args->do_reset = 1;
		break;

	case 'v':
		args->verbosity += 1;
		break;

	case ARGP_KEY_END:
		if (args->do_reset && args->setters >= 1)
			argp_error(state, "--reset/-r cannot be combined with any setter options.");
		break;

	default:
		return ARGP_ERR_UNKNOWN;
	}

	return 0;
};

const char *argp_program_version = TARGET " " VERSION;

static char doc[] = TARGET " -- a cli utility to set smu parameters for Ryzen CPUs";

static struct argp argp = {options, parse_opt, 0, doc};

int main(int argc, char **argv) {
	smu_obj_t obj;
	cpu_stat_t stat;
	smu_return_val ret;
	struct arguments args = {0};

	argp_parse(&argp, argc, argv, 0, 0, &args);

	log_set_verbosity(args.verbosity);
	VLOG(LOG_WARN, "Verbosity level set to %s", log_get_verbosity_str());

	// Userspace library requires root permissions to access driver.
	if (getuid() != 0 && geteuid() != 0) {
		VLOG(LOG_ERROR, "Program must be run as root.");
		exit(-1);
	}

	// Initialize the library for use with the program.
	VLOG(LOG_DEBUG, "Initialising SMU");
	ret = smu_init(&obj);
	if (ret != SMU_Return_OK) {
		VLOG(LOG_ERROR, "Error initializing userspace library: %s", smu_return_to_str(ret));
		exit(-2);
	}

	VLOG(LOG_DEBUG, "Getting CPU stats");
	get_cpu_stat(&obj, &stat);

	printf("%s (%s), %d cores/%d threads\n", stat.name, stat.codename, stat.cores, stat.logical_cores);
	VLOG(LOG_INFO, "SMU FW: %s", stat.smu_fw);

	if (args.do_reset) {
		VLOG(LOG_DEBUG, "Reset all core offset");
		reset_all_core_offset(&obj);
	} else {
		if (args.do_voffset) {
			VLOG(LOG_DEBUG, "Set all core offset to %d", args.voffset);
			set_all_core_offset(&obj, stat.cores, args.voffset);
		}
	}

	// print output header
	printf("Core  voffset\n");
	for (int i = 0; i < stat.cores; ++i) {
		int offset = 0;
		ret = get_core_offset(&obj, i, &offset);
		if (ret != SMU_Return_OK) {
			VLOG(LOG_ERROR, "Error reading core %d offset: %s", i, smu_return_to_str(ret));
			exit(-2);
		}
		printf("%3d   %5d\n", i, offset);
	}

	// Cleanup after library use has ended.
	smu_free(&obj);

	return 0;
}
