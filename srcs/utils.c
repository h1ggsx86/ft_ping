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

long long now_us(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	return (long long)tv.tv_sec * 1000000LL + tv.tv_usec;
}

void print_stat(const char *name, t_data stat)
{
	if (stat.pkt_send <= 0)
		return;
	int p = ((stat.pkt_send - stat.pkt_recv) / stat.pkt_send) * 100;
	double avg = stat.rtt_sum / stat.pkt_recv;

	printf("--- %s ping statistics ---\n", name);
	printf("%d packets transmitted, "
			"%d packets received, "
			"%d%% packet loss\n", stat.pkt_send, stat.pkt_recv, p);
	printf("round-trip min/avg/max/stddev = ");
	printf("%.3f/%.3f/%.3f/%.3f ms\n", stat.rtt_min, avg,
									stat.rtt_max, sqrt(stat.rtt_sum_sqr / stat.pkt_recv - (avg * avg)));
}

double sqrt(double num)
{
	double next;
	double diff;
	double x;

	if (num <= 0.0)
		return 0.0;
	x = num;
	for (int i=0; i < 100; i++) {
		next = (x + num / x) / 2.0;
		diff = next - x;
		if (diff < 0)
			diff = -diff;
		x = next;
		if (diff < 1e-9)
			return x;
	}
	return x;
}