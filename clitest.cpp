#include "common.h"

int main()
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	  cerr << "sock" << endl;
	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(80);
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	if (connect(sock, (SA*)&addr, sizeof(addr)) < 0)
	  cerr << "connect" << endl;

	if (write(sock, "sss", 3) != 3)
	  cerr << "write" << endl;
	return 0;
}
