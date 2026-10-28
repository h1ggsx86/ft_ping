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

int print_headerline(struct sockaddr_in host, const char *str)
{
	char ip[INET_ADDRSTRLEN];

	if (inet_ntop(AF_INET, &(host.sin_addr), ip, sizeof ip) == NULL)
	{
		err("inet_ntop", strerror(errno));
		return EXIT_FAILURE;
	}

	printf("PING %s (%s): %zu data bytes\n", str, ip, conf.payload);
	return EXIT_SUCCESS;
}
