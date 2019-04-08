//套接字IO接口
#ifndef HTTPD_IO_H_
#define HTTPD_IO_H_
#include "common.h"
namespace io
{
uint64_t readline(int fd, char* buf, size_t len); //读取一行,返回值为读取字符个数，链接断开则返回0
uint64_t writeline(int fd, const char* buf, size_t len);//写一行,写到\r\n结束，返回值不是正数则出错
uint64_t readn(int fd, char* buf, size_t len);//读取len个字符,0则表示断开链接
uint64_t writen(int fd, const char* buf, size_t len);//写入len个字符,返回值不为正数则出错
void upchar(char* buf, size_t len);
void downchar(char* buf, size_t len);
uint64_t get_file_size(const char* path);
}
#endif // HTTPD_IO_H_