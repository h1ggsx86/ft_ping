#include "ft_ping.h"

const char *argp_program_version = "ft_ping (42 Paris) 0.5\n\nWritten by tnedel.";
const char *argp_program_bug_address = "Report bugs to <tnedel>";

static char args_doc[] = "HOST ...";
static char doc[] = "Send ICMP ECHO_REQUEST packets to network hosts.";

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	(void)arg;
	switch (key)
	{
		case 'v':
			printf("Parsing: %c: verbose option.\n", key);
			conf.verbose = 1;
			break;

		case ARGP_KEY_ARGS:
			conf.hosts = state->argv + state->next;
			state->next = state->argc;
			break;
		case ARGP_KEY_NO_ARGS:
			argp_error(state, "missing host operand");
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}

	return 0;

}

int parsing(int *retval, int argc, char *argv[])
{

	static struct argp_option options[] = {
		{"verbose", 'v', NULL, 0, "verbose output", 0},
		{0}
	};

	struct argp _argp;
	memset(&_argp, 0, sizeof(_argp));
	_argp.options = options;
	_argp.parser = parse_opt;
	_argp.args_doc = args_doc;
	_argp.doc = doc;

	if (argp_parse(&_argp, argc, argv, 0, 0, NULL) != 0)
		return EXIT_FAILURE;


	(void)retval;

	return EXIT_SUCCESS;
}