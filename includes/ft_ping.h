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
#define MAX_BFFL 65507

typedef struct s_state {
	sig_atomic_t s_flag;
	char **hosts;
	int verbose;
	size_t payload;
} t_state;

typedef struct s_packet {
	struct icmphdr h;
	uint8_t data[MAX_BFFL - 8];
} t_packet;

typedef struct s_pkt_conf {
	uint16_t pid;
	uint16_t seq;
	struct timeval rtt;
} t_pkt_conf;

extern volatile t_state conf;

int parsing(int *retval, int argc, char *argv[]);
int err(const char *str, const char *err_str);
int print_headerline(struct sockaddr_in host, const char *str);
long long now_us(void);
void print_help(void);

#endif
