#include <argp.h>
#include <stdlib.h>
#include <unistd.h>

#include <libsmu.h>

#include "./cpustat.h"

#ifndef VERSION
#define VERSION "unversioned-build"
#endif

#ifndef TARGET
#define TARGET "rsmuctl"
#endif

static struct argp_option options[] = {{"voffset", 'v', "VALUE", 0, "per-core voltage offset, expressed in millivolts"},
		{"reset", 'r', 0, 0, "reset the voltage offset for all cores"}};

struct arguments {
	int count, do_reset, do_voffset, voffset;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
	struct arguments *arguments = state->input;

	// count the arguments processed, excluding special flags
	if (isascii(key))
		arguments->count += 1;

	switch (key) {
	case 'v':
		arguments->voffset = atoi(arg);
		arguments->do_voffset = 1;
		break;

	case 'r':
		arguments->do_reset = 1;
		break;

	case ARGP_KEY_END:
		if (arguments->do_reset && arguments->count > 1)
			argp_error(state, "--reset/-r cannot be combined with other options.");
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
	struct arguments arguments = {0};

	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	// Userspace library requires root permissions to access driver.
	if (getuid() != 0 && geteuid() != 0) {
		fprintf(stderr, "Program must be run as root.\n");
		exit(-1);
	}

	// Initialize the library for use with the program.
	smu_return_val ret = smu_init(&obj);
	if (ret != SMU_Return_OK) {
		fprintf(stderr, "Error initializing userspace library: %s\n", smu_return_to_str(ret));
		exit(-2);
	}

	get_cpu_stat(&obj, &stat);

	printf("%s (%s), %d cores/%d threads\n", stat.name, stat.codename, stat.cores, stat.logical_cores);
	printf("SMU FW: %s\n", stat.smu_fw);

	// Cleanup after library use has ended.
	smu_free(&obj);

	return 0;
}
