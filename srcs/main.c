#include "ft_ping.h"

volatile t_state conf;

int rcv_set_timeout(int sock_fd)
{
	struct timeval rcv_tout;
	memset(&rcv_tout, 0, sizeof(rcv_tout));
	rcv_tout.tv_sec = 1;
	if (setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &rcv_tout, sizeof(rcv_tout)) == -1)
		return EXIT_FAILURE;
	return EXIT_SUCCESS;
}

void sig_handler(int signo)
{
	(void)signo;
	conf.s_flag = 0;
	dprintf(STDERR_FILENO, "\tSIGINT intercepted.\n");
}

/* Return the newly created socket or -1 if an error occured */
int init_socket(void)
{
	int sock;

	sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sock == -1) {
		err("socket", strerror(errno));
		return -1;
	}
	printf("Socket created (%d)\n", sock);

	if (rcv_set_timeout(sock)) {
		err("socket", strerror(errno));
		return -1;
	}

	return sock;
}

int name_resolution(const char *name, struct sockaddr_in *host)
{
	struct addrinfo hints;
	struct addrinfo *results;
	int s;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	hints.ai_protocol = IPPROTO_ICMP;

	s = getaddrinfo(name, NULL, &hints, &results);
	if (s != 0) {
		dprintf(STDERR_FILENO, "ft_ping: unknown host\n");
		return EXIT_FAILURE;
	}
	
	*host = *(struct sockaddr_in *)results->ai_addr;

	freeaddrinfo(results);

	if (print_headerline(*host, name))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
	struct sigaction sa;
	int icmp_socket;
	int index_hosts;

	conf.s_flag = 1;
	conf.payload = 56;

	// parsing with argp()
	if (parsing(&index_hosts, argc, argv)) return EXIT_FAILURE;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = sig_handler;
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		err("signal", strerror(errno));
		return EXIT_FAILURE;
	}

	icmp_socket = init_socket();
	if (icmp_socket == -1)
		return EXIT_FAILURE;

	for (char **tmp_h=conf.hosts; *tmp_h != NULL; tmp_h++) {
		struct sockaddr_in curr_host;
		const char *str = *tmp_h;

		if (!conf.s_flag)
			break;
	 	// resolve name
		if (name_resolution(str, &curr_host)) {
			close(icmp_socket);
			return EXIT_FAILURE;
		}

		// BUILD PACKET
		// while loop/finite with -c
		// 		sendto()
		//		loop till we get an answer or timeout
		// 			recvfrom()
		// print status()
	}
	
	// print statistics at the end

	close(icmp_socket);

	return EXIT_SUCCESS; 
}
