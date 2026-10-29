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

void build_packet(t_packet *pkt, t_pkt_conf p_conf)
{
	pkt->h.type = ICMP_ECHO;
	pkt->h.code = 0;
	pkt->h.un.echo.id = htons(p_conf.pid);
	pkt->h.un.echo.sequence = htons(p_conf.seq);
	pkt->h.checksum = 0;
	memset(pkt->data + 16, 'i', (conf.payload - 16));
}

void build_next(t_packet *pkt, t_pkt_conf p_conf)
{
	(void)pkt;
	(void)p_conf;
}

uint16_t checksum(const void *pkt, size_t len)
{
	uint32_t acc = 0;
	const uint16_t *p = pkt;

	while (len > 1) {
		acc += *p++;
		len -= 2;
	}

	const uint8_t *remain;
	if (len == 1) {
		remain = (const uint8_t *)p;
		acc += *remain;
	}

	while (acc >> 16) 
		acc = (acc & 0xFFFF) + (acc >> 16);
	
	return (uint16_t)~acc;	
}

void handle_packet(int fd)
{
	(void)fd;
}

int main(int argc, char *argv[])
{
	struct sigaction sa;
	int icmp_socket;
	int index_hosts;
	t_pkt_conf p_conf;

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

	p_conf.pid = getpid() & 0xFFFF;

	icmp_socket = init_socket();
	if (icmp_socket == -1)
		return EXIT_FAILURE;

	for (char **tmp_h=conf.hosts; *tmp_h != NULL; tmp_h++) {
		struct sockaddr_in curr_host;
		const char *str = *tmp_h;
		t_packet send_buff;


		if (!conf.s_flag)
			break;
	 	// resolve name
		if (name_resolution(str, &curr_host)) {
			close(icmp_socket);
			return EXIT_FAILURE;
		}

		p_conf.seq = 0;
		memset(&send_buff, 0, sizeof(send_buff));
		build_packet(&send_buff, p_conf);

		struct pollfd pfd = {.fd = icmp_socket, .events = POLLIN};
		const long long iv_us = 1000000;

		while (conf.s_flag) {
			const long long start = now_us();

			if (p_conf.seq != 0)
				build_next(&send_buff, p_conf); // to be done

			gettimeofday(&p_conf.rtt, NULL);
			memcpy(send_buff.data, &p_conf.rtt, sizeof(p_conf.rtt));
			send_buff.h.checksum = checksum(&send_buff, sizeof(struct icmphdr) + conf.payload);
			sendto(icmp_socket, &send_buff, sizeof(struct icmphdr) + conf.payload, 0, (struct sockaddr *)&curr_host, sizeof(curr_host));
			//check sendto return code
			
			long long deadline = start + iv_us;
			while (conf.s_flag) {
				long long left_us = deadline - now_us();
				if (left_us <= 0)
					break;
				
				int left_ms = (int)((left_us + 999) / 1000);
				int ret = poll(&pfd, 1, left_ms);

				if (ret == -1) {
					if (errno == EINTR)
						break;
					err("poll", strerror(errno));
					close(icmp_socket);
					return EXIT_FAILURE;
				}
				if (ret == 0)
					break;
				if (pfd.revents & POLLIN)
					handle_packet(icmp_socket); // to be done 
			}
		}

		// refacto and put in function to clean up the main
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
