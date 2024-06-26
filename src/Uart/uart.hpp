#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
/**
 *串口发送数据函数
 *fd：串口描述符
 *data：待发送数据
 *datalen：数据长度
 */
int serial_write(int fd, char *data, int datalen)
{
    int len = 0;
    // 获取实际传输数据的长度
    len = write(fd, data, datalen);
    // printf("send data OK! datalen=%d\n", len);
    return len;
}

/**
 *串口接收数据
 *要求启动后，在pc端发送ascii文件
 */
int serial_read(int fd, char buff[], int datalen)
{
    int nread = 0;
    printf("Ready for receiving data...");
    nread = read(fd, buff, datalen);
    if (nread > 0)
    {
        printf("readlength=%d\n", nread);
        buff[nread] = '\0';
        printf("%s\n", buff);
    }
    return nread;
}

int set_serial(int fd, int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newttys1, oldttys1;

    /*保存原有串口配置*/
    if (tcgetattr(fd, &oldttys1) != 0)
    {
        perror("Setupserial 1");
        return -1;
    }
    bzero(&newttys1, sizeof(newttys1));
    newttys1.c_cflag |= (CLOCAL | CREAD); /*CREAD 开启串行数据接收，CLOCAL并打开本地连接模式*/

    newttys1.c_cflag &= ~CSIZE; /*设置数据位*/
    /*数据位选择*/
    switch (nBits)
    {
    case 7:
        newttys1.c_cflag |= CS7;
        break;
    case 8:
        newttys1.c_cflag |= CS8;
        break;
    }
    /*设置奇偶校验位*/
    switch (nEvent)
    {
    case '0':                                 /*奇校验*/
        newttys1.c_cflag |= PARENB;           /*开启奇偶校验*/
        newttys1.c_iflag |= (INPCK | ISTRIP); /*INPCK打开输入奇偶校验；ISTRIP去除字符的第八个比特  */
        newttys1.c_cflag |= PARODD;           /*启用奇校验(默认为偶校验)*/
        break;
    case 'E':                                 /*偶校验*/
        newttys1.c_cflag |= PARENB;           /*开启奇偶校验  */
        newttys1.c_iflag |= (INPCK | ISTRIP); /*打开输入奇偶校验并去除字符第八个比特*/
        newttys1.c_cflag &= ~PARODD;          /*启用偶校验*/
        break;
    case 'N': /*无奇偶校验*/
        newttys1.c_cflag &= ~PARENB;
        break;
    }
    /*设置波特率*/
    switch (nSpeed)
    {
    case 2400:
        cfsetispeed(&newttys1, B2400);
        cfsetospeed(&newttys1, B2400);
        break;
    case 4800:
        cfsetispeed(&newttys1, B4800);
        cfsetospeed(&newttys1, B4800);
        break;
    case 9600:
        cfsetispeed(&newttys1, B9600);
        cfsetospeed(&newttys1, B9600);
        break;
    case 115200:
        cfsetispeed(&newttys1, B115200);
        cfsetospeed(&newttys1, B115200);
        break;
    default:
        cfsetispeed(&newttys1, B9600);
        cfsetospeed(&newttys1, B9600);
        break;
    }
    /*设置停止位*/
    if (nStop == 1) /*设置停止位；若停止位为1，则清除CSTOPB，若停止位为2，则激活CSTOPB*/
    {
        newttys1.c_cflag &= ~CSTOPB; /*默认为一位停止位； */
    }
    else if (nStop == 2)
    {
        newttys1.c_cflag |= CSTOPB; /*CSTOPB表示送两位停止位*/
    }

    /*设置最少字符和等待时间，对于接收字符和等待时间没有特别的要求时*/
    newttys1.c_cc[VTIME] = 0; /*非规范模式读取时的超时时间；*/
    newttys1.c_cc[VMIN] = 0;  /*非规范模式读取时的最小字符数*/
    tcflush(fd, TCIFLUSH);    /*tcflush清空终端未完成的输入/输出请求及数据；TCIFLUSH表示清空正收到的数据，且不读取出来 */

    /*激活配置使其生效*/
    if ((tcsetattr(fd, TCSANOW, &newttys1)) != 0)
    {
        perror("com set error");
        return -1;
    }

    return 0;
}