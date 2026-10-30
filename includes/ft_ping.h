# ifndef FT_PING_H
#define FT_PING_H

#define _GNU_SOURCE
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <linux/if_packet.h>
#include <net/if_arp.h>
#include <net/ethernet.h>
#include <linux/if_link.h>
#include <errno.h>
#include <getopt.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sysexits.h>
#include <argp.h>
#include <poll.h>

#include <signal.h>

#define IPV4_LENGTH 4

enum e_recv {
	RECV_OK = 0,
	RECV_INTR,
	RECV_ERROR,
	RECV_IGNORED
};

typedef struct s_state {
	sig_atomic_t s_flag;
	char **hosts;
	int verbose;
	size_t payload;
	int count;
} t_state;

typedef struct s_packet {
	struct icmphdr h;
	uint8_t data[IP_MAXPACKET - 8];
} t_packet;

typedef struct s_pkt_conf {
	uint16_t pid;
	uint16_t seq;
	struct timeval rtt;
} t_pkt_conf;

typedef struct s_data {
	int pkt_send, pkt_recv;
	double rtt_min, rtt_max, rtt_sum, rtt_sum_sqr;
} t_data;

extern volatile t_state conf;

int parsing(int argc, char *argv[]);
int err(const char *str, const char *err_str);
int print_headerline(struct sockaddr_in host, const char *str);
long long now_us(void);
void print_help(void);
void print_stat(const char *name, t_data stat);
double sqrt(double num);

#endif
