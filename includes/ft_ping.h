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

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sysexits.h>
#include <argp.h>

#include <signal.h>

#define IPV4_LENGTH 4

typedef struct s_state {
	sig_atomic_t s_flag;
	char **hosts;
	int verbose;
	size_t payload;
} t_state;

extern volatile t_state conf;

int parsing(int *retval, int argc, char *argv[]);
int err(const char *str, const char *err_str);
int print_headerline(struct sockaddr_in host, const char *str);
void print_help(void);

#endif
