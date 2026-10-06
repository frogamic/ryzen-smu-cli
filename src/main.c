#include <argp.h>
#include <stdio.h>
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

#ifndef MAX_CORES
#define MAX_CORES 128
#endif

static struct argp_option options[] = {{"verbose", 'v', 0, 0, "Increase the verbosity level of the output"},
		{"voffset", 'o', "VALUE", 0,
				"Set Curve-Optimizer voltage offset. "
				"VALUE can be an offset applied to all cores, or a core number followed by : and the offset to specify an "
				"offset for a core. "
				"This argument can be passed multiple times to specify an all-core offset plus specific core offsets, e.g. "
				"-o -10 -o 3:-25 -o 7:-25"},
		{"reset", 'r', 0, 0, "Reset the voltage offset for all cores (applied before any other changes)"}, {0}};

struct core_arg {
	int do_voffset, voffset;
};

struct arguments {
	int core_count, do_reset, verbosity;
	struct core_arg base_args;
	struct core_arg core_args[MAX_CORES];
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
	struct arguments *args = state->input;
	int core, offset;
	char junk;

	// Only parse the verbosity on the first pass (before core_count is known)
	if (args->core_count == 0) {
		if (key == 'v')
			args->verbosity += 1;
		return 0;
	}

	switch (key) {
	case 'o':
		VLOG(LOG_DEBUG, "Parsing offset arg %s", arg);
		if (sscanf(arg, "%d:%d%c", &core, &offset, &junk) == 2) {
			VLOG(LOG_TRACE, "Offset for core %d specified: %d", core, offset);
			if (core < args->core_count && core >= 0) {
				args->core_args[core].voffset = offset;
				if (++(args->core_args[core].do_voffset) > 1)
					argp_error(state, "Voltage offset for core %d passed more than once", core);
			} else {
				argp_error(state, "Invalid core %d, should be between 0 and %d", core, args->core_count - 1);
			}
		} else if (sscanf(arg, "%d%c", &offset, &junk) == 1) {
			args->base_args.voffset = offset;
			if (++(args->base_args.do_voffset) > 1)
				argp_error(state, "All-core voltage offset passed more than once");
			VLOG(LOG_TRACE, "All-core offset specified: %d", args->base_args.voffset);
		} else {
			argp_error(state, "Could not parse voffset arg: %s", arg);
		}
		break;

	case 'r':
		if (++(args->do_reset) > 1)
			argp_error(state, "Reset option was passed more than once");
		break;

	case 'v':
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

	// Parse just the verbosity arg early
	argp_parse(&argp, argc, argv, 0, NULL, &args);

	log_set_verbosity(args.verbosity);
	VLOG(LOG_WARN, "Verbosity level set to %s", log_get_verbosity_str());

	// Userspace library requires root permissions to access driver.
	if (getuid() != 0 && geteuid() != 0) {
		VLOG(LOG_ERROR, "Program must be run as root.");
		exit(EXIT_FAILURE);
	}

	// Initialize the library for use with the program.
	VLOG(LOG_DEBUG, "Initialising SMU");
	ret = smu_init(&obj);
	if (ret != SMU_Return_OK) {
		VLOG(LOG_ERROR, "Error initializing userspace library: %s", smu_return_to_str(ret));
		exit(EXIT_FAILURE);
	}

	VLOG(LOG_DEBUG, "Getting CPU stats");
	get_cpu_stat(&obj, &stat);

	if (stat.cores <= 0 || stat.cores > MAX_CORES) {
		VLOG(LOG_ERROR, "Invalid CPU core count %d, must be between 1 and %d", stat.cores, MAX_CORES);
		exit(EXIT_FAILURE);
	}

	args.core_count = stat.cores;
	argp_parse(&argp, argc, argv, 0, NULL, &args);

	printf("%s (%s), %d cores/%d threads\n", stat.name, stat.codename, stat.cores, stat.logical_cores);
	VLOG(LOG_INFO, "SMU FW: %s", stat.smu_fw);

	if (args.do_reset) {
		VLOG(LOG_DEBUG, "Reset all core offset");
		ret = reset_all_core_offset(&obj);
		if (ret != SMU_Return_OK) {
			VLOG(LOG_ERROR, "Error resetting all core offsets: %s", smu_return_to_str(ret));
			exit(EXIT_FAILURE);
		}
	}

	// print output header
	printf("Core  voffset\n");

	for (unsigned int i = 0; i < stat.cores; ++i) {
		// modify core
		int do_voffset = 0, offset = 0;
		if (args.core_args[i].do_voffset) {
			do_voffset = 1;
			offset = args.core_args[i].voffset;
			VLOG(LOG_DEBUG, "Set core %d to core-specific offset of %d", i, offset);
		} else if (args.base_args.do_voffset) {
			do_voffset = 1;
			offset = args.base_args.voffset;
			VLOG(LOG_DEBUG, "Set core %d to all-core offset of %d", i, offset);
		}

		if (do_voffset) {
			ret = set_core_offset(&obj, i, offset);
			if (ret != SMU_Return_OK) {
				VLOG(LOG_ERROR, "Error setting core %d offset to %d: %s", i, offset, smu_return_to_str(ret));
				exit(EXIT_FAILURE);
			}
		}

		// print result
		int foffset = 0;
		ret = get_core_offset(&obj, i, &foffset);
		if (ret != SMU_Return_OK) {
			VLOG(LOG_ERROR, "Error reading core %d offset: %s", i, smu_return_to_str(ret));
			exit(EXIT_FAILURE);
		}
		printf("%3d   %5d\n", i, foffset);
	}

	// Cleanup after library use has ended.
	smu_free(&obj);

	return EXIT_SUCCESS;
}
