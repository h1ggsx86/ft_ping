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

#include <signal.h>

#define WANTED_BITS (IFF_UP | IFF_RUNNING)
#define UNWANTED_BITS (IFF_LOOPBACK | IFF_NOARP)
#define BUFF_SIZE 60
#define HW_TYPE 1
#define IPV4_LENGTH 4

typedef int (*t_handler)(const char *arg);

typedef struct s_state {
	sig_atomic_t s_flag;
	int verbose;
	int help;
} t_state;

typedef struct s_opt_entry {
	int key;
	t_handler fn;
} t_opt_entry;

typedef struct s_hosts {
	char *data;
	struct s_hosts *next;
} t_hosts;

extern volatile t_state conf;

int parsing(int *retval, int argc, char *argv[]);
int err(const char *str, const char *err_str);
void print_help(void);

#endif
