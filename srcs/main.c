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

void build_next(t_packet *pkt, t_pkt_conf *p_conf)
{
	pkt->h.un.echo.sequence = htons(p_conf->seq);
	pkt->h.checksum = 0;
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

int handle_packet(int sock_fd, uint16_t pid, t_data *stat)
{
	_Alignas(struct iphdr) uint8_t recv_buf[IP_MAXPACKET];
	struct sockaddr_in from;
	socklen_t fromlen = sizeof(from);
	struct timeval now;

	ssize_t ret = recvfrom(sock_fd, recv_buf, sizeof(recv_buf), 0,
						   (struct sockaddr *)&from, &fromlen);
	gettimeofday(&now, NULL);
	if (ret == -1)
		return (errno == EINTR) ? RECV_INTR : RECV_ERROR;

	size_t bytes = (size_t)ret;
	if (bytes < sizeof(struct iphdr))
		return RECV_IGNORED;

	const struct iphdr *ip = (const struct iphdr *)recv_buf;
	size_t offset = (size_t)ip->ihl * 4;
	if (ip->ihl < 5 || bytes < offset + sizeof(struct icmphdr))
		return RECV_IGNORED;

	struct icmphdr *icmp = (struct icmphdr *)(recv_buf + offset);
	size_t icmp_len = bytes - offset;

	if (icmp->type != ICMP_ECHOREPLY)
		return RECV_IGNORED;
	if (ntohs(icmp->un.echo.id) != pid)
		return RECV_IGNORED;
	if (checksum(icmp, icmp_len) != 0)
		return RECV_IGNORED;

	char ip_str[INET_ADDRSTRLEN];
	if (inet_ntop(AF_INET, &from.sin_addr, ip_str, sizeof(ip_str)) == NULL)
		return RECV_ERROR;

	printf("%zu bytes from %s: icmp_seq=%u ttl=%u", icmp_len, ip_str,
		   ntohs(icmp->un.echo.sequence), ip->ttl);

	if (icmp_len >= sizeof(struct icmphdr) + sizeof(struct timeval))
	{
		struct timeval sent;
		struct timeval diff;

		memcpy(&sent, (uint8_t *)icmp + sizeof(struct icmphdr), sizeof(sent));
		timersub(&now, &sent, &diff);
		double rtt = diff.tv_sec * 1000.0 + diff.tv_usec / 1000.0;
		printf(" time=%.3f ms", rtt);

		/* update statistics here: received++, min, max, sum, sum of squares */
		stat->pkt_recv++;
		stat->rtt_max = (stat->rtt_max < rtt) ? rtt : stat->rtt_max;
		stat->rtt_min = (stat->rtt_min > rtt) ? rtt : stat->rtt_min;
		stat->rtt_sum += rtt;
		stat->rtt_sum_sqr += rtt * rtt;

		if (icmp->un.echo.sequence == 0)
			stat->rtt_min = rtt;
	}
	printf("\n");
	return RECV_OK;
}

int recv_loop(long long deadline, struct pollfd *pfd, int sock_fd, uint16_t pid, t_data *stat)
{
	while (conf.s_flag)
	{
		long long left_us = deadline - now_us();
		if (left_us <= 0)
			break;

		int left_ms = (int)((left_us + 999) / 1000);
		int ret = poll(pfd, 1, left_ms);

		if (ret == -1)
		{
			if (errno == EINTR)
				break;
			err("poll", strerror(errno));
			close(sock_fd);
			return EXIT_FAILURE;
		}
		if (ret == 0)
			break;
		if (pfd->revents & POLLIN) {
			int ret = handle_packet(sock_fd, pid, stat); // to be done
			switch (ret)
			{
				case RECV_ERROR:
					err("recvfrom", strerror(errno));
					close(sock_fd);
					return EXIT_FAILURE;
				case RECV_INTR:
					conf.s_flag = 0;
					break;
				case RECV_IGNORED:
					continue;
					break;
			}
		}
	}

	return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
	struct sigaction sa;
	int icmp_socket;
	t_pkt_conf p_conf;
	t_packet send_buff;
	t_data stat;
	
	conf.s_flag = 1;
	conf.payload = 56;

	// parsing with argp()
	if (parsing(argc, argv)) return EXIT_FAILURE;

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

	for (; *conf.hosts != NULL; conf.hosts++) {
		struct sockaddr_in cur_host;
		const char *str = *conf.hosts;
		memset(&stat, 0, sizeof(stat));

		if (!conf.s_flag)
			break;
	 	// resolve name
		if (name_resolution(str, &cur_host)) {
			close(icmp_socket);
			return EXIT_FAILURE;
		}

		p_conf.seq = 0;
		memset(&send_buff, 0, sizeof(send_buff));
		build_packet(&send_buff, p_conf);

		struct pollfd pfd = {.fd = icmp_socket, .events = POLLIN};
		const long long inter_us = 1000000;

		while (conf.s_flag) {
			const long long start = now_us();

			build_next(&send_buff, &p_conf);

			gettimeofday(&p_conf.rtt, NULL);
			memcpy(send_buff.data, &p_conf.rtt, sizeof(p_conf.rtt));
			send_buff.h.checksum = checksum(&send_buff, sizeof(struct icmphdr) + conf.payload);
			sendto(icmp_socket, &send_buff, sizeof(struct icmphdr) + conf.payload,
						0, (struct sockaddr *)&cur_host, sizeof(cur_host));
			p_conf.seq++;
			stat.pkt_send++;
			//check sendto return code
			
			long long deadline = start + inter_us;
			if (recv_loop(deadline, &pfd, icmp_socket, p_conf.pid, &stat) < 0) return EXIT_FAILURE;
		}

		print_stat(str, stat);
	}
	
	// print statistics at the end
	while (*conf.hosts != NULL)
		print_stat(*conf.hosts++, stat);
	printf("process <%d> terminated (SIGINT)\n", getpid());

	close(icmp_socket);

	return EXIT_SUCCESS; 
}
