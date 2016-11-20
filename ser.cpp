#include "ser.h"

Ser::Ser()
{
	if ((m_listenfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	  ERR_EXIT("socket");
	int on = 1;
	if (setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on)) < 0)
		ERR_EXIT("setsockopt");
	addr_len = sizeof(my_addr);
	bzero(&my_addr, addr_len);
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(80);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	Bind(m_listenfd, &my_addr);
	Listen(m_listenfd, LISTENQ);
	
	if ((epoll_fd = epoll_create1(0)) < 0)
	  ERR_EXIT("epoll_create1");
	add_event(m_listenfd, EPOLLIN);
	
	struct epoll_event one;
	m_epoll_event.push_back(one);
}

Ser::Ser(const char* ip, unsigned int port)
{
	if ((m_listenfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	  ERR_EXIT("socket");
	
	int on = 1;
     if (setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on)) <     0)
         ERR_EXIT("setsockopt");


	addr_len = sizeof(my_addr);
	bzero(&my_addr, addr_len);
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
	if (inet_pton(AF_INET, ip, &my_addr) < 0)
	  ERR_EXIT("inet_pton");

	Bind(m_listenfd, &my_addr);
	Listen(m_listenfd, LISTENQ);

	if ((epoll_fd = epoll_create1(0)) < 0)
	  ERR_EXIT("epoll_create1");
	add_event(m_listenfd, EPOLLIN);
	
	struct epoll_event one;
	bzero(&one, sizeof(one));
	m_epoll_event.push_back(one);
}

void Ser::Bind(int sockfd, struct sockaddr_in* addr)
{
	if (bind(sockfd, (SA*)addr, addr_len) < 0)
		ERR_EXIT("bind");
}

void Ser::Listen(int sockfd, unsigned int num)
{
	if (listen(sockfd, num) < 0)
	  ERR_EXIT("listen");
}

int  Ser::wait_event()
{
	int ret = epoll_wait(epoll_fd, &*m_epoll_event.begin(), m_epoll_event.size()+1, -1);
	if (ret < 0)
	{
		ERR_EXIT("epoll_wait");;
	}
	return ret;
}

void Ser::add_event(int fd, int state)
{
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev);	
}
void Ser::delete_event(int fd, int state)
{
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;
	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &ev);
}
void Ser::modify_event(int fd, int state)
{
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;
	epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
}

void Ser::do_accept()
{
	int fd = accept(m_listenfd, nullptr, nullptr);
	/*
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	bzero(&addr, len);
	getsockname(m_listenfd, (SA*)&addr, &len);
	cout << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << " :: ";
	
	bzero(&addr, len);
	getpeername(m_listenfd, (SA*)&addr, &len);
	cout << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << endl;
	
	bzero(&addr, len);
	getsockname(fd, (SA*)&addr, &len);
	cout << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << " :: ";
	bzero(&addr, len);
	getpeername(fd, (SA*)&addr, &len);
	cout << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << endl;
	//*/
	
	if (fd == -1)
	{
		cerr << "accept" << endl;
		return;
	}
	cout << "new fd:" << fd << endl;

	m_connfd.push_back(fd);//Ìí¼Óµ½ÒÑÁ¬½Ó¶ÓÁÐ
	add_event(fd, EPOLLIN);//Ìí¼Óµ½epoll¼àÌý

	struct epoll_event ev;
	bzero(&ev, sizeof(ev));
	m_epoll_event.push_back(ev);//À©´óÊÂ¼þ´¦Àí¶ÓÁÐÈÝÁ¿

	//do_in(fd);//Æô¶¯Ïß³Ì´¦Àí´ËÇëÇóÊÂ¼þ
}

void Ser::do_in(int fd)
{
	char head[HEADSIZE];
	//¶Á±¨ÎÄÍ·
	int headlen;
	if ((headlen = readline(fd, head, HEADSIZE)) == 0)
	{
		do_close(fd);
		return;
	}
	head[headlen] = 0;
	//read·µ»Ø0ÔòÁ´½Ó¶Ï¿ª,´¦ÀíÒÑÁ´½ÓÁÐ±í£¬´¦Àíepoll¼àÌý¶ÓÁÐ,¼õÐ¡Êý×é´óÐ¡
	//½âÎö´æ´¢
	char way[16] = {0};
	char uri[1024] = {0};
	char version[16] = {0};
	
	cout << head;

	sscanf(head, "%s %s %s\r\n", way, uri, version);
	upchar(way, sizeof(way));//Í³Ò»´óÐ¡Ð´
	downchar(uri, sizeof(uri));
	upchar(version, sizeof(version));

	if (strcmp(way, "GET") == 0)//get·½·¨ÇëÇó
	{
		int ret;
		char bodybuf[BODYSIZE] = {0};
		while (1)//Ö±µ½ÇëÇóÍ·½áÊø
		{
			ret = readline(fd, bodybuf, BODYSIZE);
			if (ret == 0)
			{
				do_close(fd);
				return;
			}
			if ((ret == 2) && (bodybuf[0] == '\r') && (bodybuf[1] == '\n'))
			  break;
			//cout << bodybuf;
			bzero(bodybuf, BODYSIZE);
		}
		do_get(fd, uri, sizeof(uri));//½âÎöÊý¾Ý²¢Íê³É·¢ËÍÈÎÎñ
	}
	else if (strcmp(way, "POST") == 0)//post·½·¨ÇëÇó
	{
		char bodybuf[BODYSIZE] = {0};
		char keybuf[32] = {0};
		char valuebuf[512] = {0};
		size_t textlen = 0;
		int ret;
		while (1)//Ö±µ½ÇëÇóÍ·½áÊø
		{
			ret = readline(fd, bodybuf, BODYSIZE);
			if (ret == 0)
			{
				do_close(fd);
				return;
			}
			if ((ret == 2) && (bodybuf[0] == '\r') && (bodybuf[1] == '\n'))
			  break;
		
			cout << bodybuf;

			downchar(bodybuf, BODYSIZE);//Í³Ò»×ªÎªÐ¡Ð´
			bzero(keybuf, sizeof(keybuf));
			bzero(valuebuf, sizeof(valuebuf));
			sscanf(bodybuf, "%s: %s\r\n", keybuf, valuebuf);//·Ö½â¼üÖµ¶Ô
			if (strcmp(keybuf, "content-length") == 0)
			{
				sscanf(valuebuf, "%lu", &textlen);//¶ÁÈ¡ÎÄ±¾³¤¶È
			}
			bzero(bodybuf, BODYSIZE);
		}

		char *ptext = new char[textlen+1];
		bzero(ptext, textlen+1);
		if (readn(fd, ptext, textlen) == 0)//¶ÁÈ¡ÎÄ±¾
		{
			delete[] ptext;
			do_close(fd);
			return;
		}
		ptext[textlen] = '\0';
		do_post(fd, uri, sizeof(uri), ptext, textlen+1);//½âÎöÊý¾Ý²¢Íê³É·¢ËÍÈÎÎñ
		delete[] ptext;
	}
	return;
}

void Ser::do_close(int fd)
{
	//close()
	close(fd);
	cout << "close fd:" << fd << endl;
	//´ÓÒÑÁ¬½Ó¶ÓÁÐÖÐÉ¾³ý
	m_connfd.remove(fd);
	//´ÓepollÖÐÉ¾³ý
	delete_event(fd, EPOLLIN);
	//½áÊøµ±Ç°Ïß³Ì
	//pthread_exit(NULL);
	
}

void Ser::do_conf(const char* filename)
{
	bzero(confpath, sizeof(confpath));
	FILE* pconf = fopen(filename, "rb");
	char line[1024] = {0};
	while (fgets(line, sizeof(line), pconf) != NULL)
	{
		char key[32] = {0};
		char path[1024] = {0};
		//sscanf(line, "%s=%s\n", key, path);
		char* p = line;
		while (*(p++) != '=');
		p[-1] = '\0';
		p[strlen(p)-1] = '\0';//È¥µô»»ÐÐ·û
		strcpy(key, line);
		strcpy(path, p);

		upchar(key, sizeof(key));
		//cout << key << path << endl;
		if (strcmp(key, "PATH") == 0)
		{
			strncpy(confpath, path, sizeof(path));
			if (confpath[strlen(confpath)-1] == '/')//È¥µôºó×º/
			{
				confpath[strlen(confpath)-1] = '\0';
			}
			//cout << confpath << endl;
		}
		
	}
	return;
}

void Ser::go()
{
	do_conf("./.httpd.conf");//´ÓÅäÖÃÎÄþ ¼ÓÔØÅäÖÃÏî
	//cout << "confpath:" << confpath << endl;
	while (1)
	{
		int num = wait_event();
		for (int i = 0; i < num; ++i)
		{
			int fd = m_epoll_event[i].data.fd;
			int event = m_epoll_event[i].events;
			
			if ((fd == m_listenfd) && (event & EPOLLIN))
			{
				do_accept();
				//thread(&Ser::do_accept, *this).detach();//Æô¶¯·ÖÀëµÄÎÞÃûÏß³Ì´¦ÀíÐÂÁ¬½Ó
			}
			else if (event & EPOLLIN)
			{
				do_in(fd);
				//thread(&Ser::do_in, *this, fd).detach();//Æô¶¯Ïß³Ì´¦Àí´ËÇëÇóÊÂ¼þ
			}
			/*else if (event & EPOLLOUT)
				//thread out(do_out, fd);
				do_out(fd);*/
		}
		//ÊÊµ±¼õÐ¡ÊÂ¼þ´¦ÀíÈÝÆ÷
		size_t sizelist = m_connfd.size();
		size_t sizearr = m_epoll_event.size()/2;
		if ((sizelist > 100) && (sizearr > sizelist))
			m_epoll_event.resize(sizearr);
	}
}

unsigned int Ser::readline(int fd, char* buf, size_t len)
{
	unsigned int i = 0;
	for (i = 0; i < len; ++i)
	{
		if (read(fd, buf+i, 1) != 1)
		{
			if (errno == EINTR)
				continue;
			else return 0;
		}
		if (*(buf+i) == '\n' && *(buf+i-1) == '\r')
		  break;
	}
	return i+1;
}
unsigned int Ser::writeline(int fd, const char* buf, size_t len)
{
	unsigned int i = 0;
	for (i = 0; i < len; ++i)
	{
		if (write(fd, buf+i, 1) != 1)
		{
			if (errno == EINTR)
			{
				--i;
				continue;
			}
			else return 0;
		}
		if (*(buf+i) == '\n' && *(buf+i-1) == '\r')
		  break;
	}
	return i+1;
}
unsigned int Ser::readn(int fd, char* buf, size_t len)
{
	char* now = buf;
	size_t oready = 0;
	size_t ward = len;
	int ret = 0;
	while (ward)
	{
		do {//ÖÐ¶Ï»Ö¸´
			ret = read(fd, now, ward);
		} while ((ret == -1) && (errno == EINTR));
		
		if (ret == -1)
		  ERR_EXIT("readn");
		else if (ret == 0)//Á¬½ÓÖÐ¶ÏÔÙÒ²Ã»ÓÐÊý¾Ý
		  return 0;
		
		now += ret;
		oready += ret;
		ward -= ret;
	}
	return oready;
}
unsigned int Ser::writen(int fd, const char* buf, size_t len)
{
	const char* now = buf;
	size_t oready = 0;
	size_t ward = len;
	int ret = 0;
	while (ward)
	{
		do {
			ret = write(fd, buf, ward);
		} while ((ret == -1) && (errno == EINTR));

		if (ret <= 0)
		  ERR_EXIT("writen");

		now += ret;
		oready += ret;
		ward -= len;
	}
	return oready;
}
void Ser::upchar(char* buf, size_t len)
{
	for (size_t i = 0; i < len; ++i)
	  *(buf+i) = toupper(*(buf+i));
}
void Ser::downchar(char* buf, size_t len)
{
	for (size_t i = 0; i < len; ++i)
	  *(buf+i) = tolower(*(buf+i));
}
unsigned long Ser::get_file_size(const char* path)
{
	unsigned long sizefile = -1;
	struct stat statbuf;
	if (stat(path, &statbuf) < 0)
	  return sizefile;
	else
	  sizefile = statbuf.st_size;
	return sizefile;
}

void Ser::do_get(int fd, const char* uri, size_t len)
{
	//½âÎöuriÖ¸¶¨µÄÎÄ¼þ¼°²ÎÊý
	//cout << "do_get" << endl;
	//¾²Ì¬ÎÄ¼þÔò·¢ËÍÎÄ¼þ
	char filepath[1024] = {0};
	snprintf(filepath, sizeof(filepath), "%s%s", confpath, uri);
	if (filepath[strlen(filepath)-1] == '/')
	  strcat(filepath, "demo.html");
	else return;
	//cout << filepath << endl;
	if (!writeline(fd, "HTTP/1.1 200 OK\r\n", sizeof("HTTP/1.1 200 OK\r\n")))
	{
		do_close(fd);
		return;
	}

	if (!writeline(fd, "Content-Type: text/html\r\n", sizeof("Content-Type: text/html\r\n")))
	{
		do_close(fd);
		return;
	}
	unsigned long filesize = get_file_size(filepath);
	char contentlength_buf[128] = {0};
	sprintf(contentlength_buf, "content-length: %lu\r\n", filesize);
	if (writeline(fd, contentlength_buf, sizeof(contentlength_buf)) == 0)
	{
		do_close(fd);
		return;
	}
	if (writeline(fd, "\r\n", sizeof("\r\n")) == 0)
	{
		do_close(fd);
		return;
	}
	//cout << "begin send file:" << filesize << endl;
	FILE* pfile = fopen(filepath, "rb");
	char buf[4096] = {0};
	int ret = 0;
	while (1)
	{
		ret = fread(buf, 1, sizeof(buf), pfile);
		buf[ret] = 0;
		if (ret < 0)
		{
			if (errno == EINTR)
			  continue;
			else do_close(fd);
		}
		if (ret == 0)
		  break;
		if (!writen(fd, buf, ret))
		{
			do_close(fd);
			return;
		}
		
	}
	//¶¯Ì¬ÇëÇóÔòÖ´ÐÐ³ÌÐò²¢·¢ËÍÊä³ö
}
void Ser::do_post(int fd, const char* uri, size_t urilen, const char* text, size_t textlen)
{
	cout << "do_post" << endl;
	//½âÎöuri
	//½âÎöÊ¹ÓÃtext±¨ÎÄÌå
	//·¢ËÍÏàÓ¦ÄÚÈÝ
}
