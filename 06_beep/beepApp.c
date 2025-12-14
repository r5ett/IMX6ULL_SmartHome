#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

/*
 * Usage:
 *   ./ledApp  /dev/beep  0   // 关闭蜂鸣器
 *   ./ledApp  /dev/beep  1   // 开启蜂鸣器
 */

int main(int argc, char *argv[])
{
    int fd;
    unsigned char cmd;

    if (argc != 3) {
        printf("Usage: %s <dev> <0|1>\n", argv[0]);
        return -1;
    }

    /* 打开设备文件 */
    fd = open(argv[1], O_RDWR);
    if (fd < 0) {
        perror("open device failed");
        return -1;
    }

    /* 判断输入 0 / 1 */
    if (!strcmp(argv[2], "1")) {
        cmd = 0;   // 驱动中定义 beepON=0
    } else if (!strcmp(argv[2], "0")) {
        cmd = 1;   // beepOFF=1
    } else {
        printf("Invalid parameter: must be 0 or 1\n");
        close(fd);
        return -1;
    }

    /* 写入到驱动 */
    if (write(fd, &cmd, 1) < 0) {
        perror("write error");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}
