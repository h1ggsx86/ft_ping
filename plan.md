## Plan

- Parse options with `getopt_long()`
- Create socket
- Send imcp request (build it from scratch/learn to do it)
- Wait in `poll()` for reply
	- If no reply procede to send the next
- Print status line along the way