#include "ft_ping.h"

static int opt_verbose(const char *arg)
{
	(void)arg;
	conf.verbose = 1;
	return 0;
}

static int opt_help(const char *arg)
{
	(void)arg;
	conf.help = 1;
	return 0;
}

static int dispatch(t_opt_entry table[], int key, const char *arg)
{
	for (int i=0; table[i].fn != NULL; i++)
		if (table[i].key == key)
			return table[i].fn(arg);
	return -1;
}

int parsing(int *retval, int argc, char *argv[])
{
	static t_opt_entry table[] = {
		{'v', opt_verbose},
		{'?', opt_help},
		{0, NULL}
	};
	static struct option longopts[] = {
		{"verbose", no_argument, NULL, 'v'},
		{"help", no_argument, NULL, '?'},
		{NULL, 0, NULL, 0}
	};	

	int c;
	while ((c = getopt_long(argc, argv, "v?", longopts, 0)) != -1) {
		if (dispatch(table, c, optarg) < 0)
			return 1;
	}

	printf("Arguments: %s\n", argv[optind]);

	*retval = optind;
	
	return EXIT_SUCCESS;
}