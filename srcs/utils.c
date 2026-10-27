#include "ft_ping.h"

int err(const char *str, const char *err_str)
{
	printf("ft_ping: %s: %s.\n", str, err_str);
	return EXIT_FAILURE;
}

void print_help(void)
{
	printf("Usage: sudo ./ft_ping [OPTION...] HOST ...\n"
			"Send ICMP ECHO_REQUEST packets to network hosts.\n\n"
			" Options valid for all request types:\n\n"
			"  -v, --verbose              verbose output\n\n"
			"Mandatory or optional arguments to long options are also mandatory or optional for any corresponding short options.\n\n"
		);
}
