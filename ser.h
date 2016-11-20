#include "common.h"


class Ser {
		//friend void do_accept(Ser& ser);//�������ӽ���ʱ����
		//friend void do_in(Ser& ser, int fd);//�пɶ��¼�����ʱ
	public:
		Ser();
		Ser(const char* ip, unsigned int port);
		~Ser() {}
	public:
		void go();
		int wait_event();//����һ�������׽ӿ�
		void do_accept();//�������ӽ���ʱ����
		void do_in(int fd);//�пɶ��¼�����ʱ
		void do_close(int fd);//�Ͽ�����
	public:
		/*void do_out(int fd);//�п�д�¼�����ʱ*/

	private://���Ĵ�����
		unsigned int readline(int fd, char* buf, size_t len); //��ȡһ��,����ֵΪ��ȡ�ַ����������ӶϿ��򷵻�0
		unsigned int writeline(int fd, const char* buf, size_t len);//дһ��,д��\r\n����������ֵ�������������
		unsigned int readn(int fd, char* buf, size_t len);//��ȡlen���ַ�,0���ʾ�Ͽ�����
		unsigned int writen(int fd, const char* buf, size_t len);//д��len���ַ�,����ֵ��Ϊ���������
		void upchar(char* buf, size_t len);
		void downchar(char* buf, size_t len);
		void do_get(int fd, const char* uri, size_t len);
		void do_post(int fd, const char* uri, size_t urilen, const char* text, size_t textlen);
		unsigned long get_file_size(const char* path);
	private://˽�÷���
		void do_conf(const char*filename);
		void Bind(int sockfd, struct sockaddr_in* addr);//��ʼ������
		void Listen(int sockfd, unsigned int num);//��ʼ������
		void add_event(int fd, int state);//���һ���¼���epoll
		void delete_event(int fd, int state);
		void modify_event(int fd, int state);
	private://����
		int m_listenfd;//�����׽���
		struct sockaddr_in my_addr;//�����׽��ֵĵ�ַ�ṹ
		int epoll_fd;
		socklen_t addr_len;//��ַ����
	
		char confpath[1024];//Ĭ��·������
		
		list<int> m_connfd;//�������׽��ֶ���
		vector<struct epoll_event> m_epoll_event;//�������¼�����
};





/*

int main()
{
	Ser ser("119.29.4.18", 80);
	while ()
	{
		epoll_wait();
		switch()
		{
			case listen_fd:
			thread();
			case conn_fd:
			thread();
		}
	}
	return 0;
}

do_accept()
{
	accept();
	epoll_ctl(add);
}

do_read()
{
	do_work();
	epoll_ctl(toout);
}

do_write()
{
	work_end();
	epoll_ctl(toin);
}

do_work()
{
	//��
	//��������
	//�洢
}

work_end()
{
	//ʹ�ô洢
	//������Ӧ����
}*/
