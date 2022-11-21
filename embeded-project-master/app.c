#include<stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#define NOFILL 4
#define BLINK 8

int main(void)
{
    char fl = 0;
    int dev, data, key,rdata;
    time_t now, before,cap;
    struct tm *nt;
    int mode = 0;
    int sub_mode = 0;
    int add[2] = { 0,0 };
    int tmp[2] = { 0, };
    int alarm[2] = { 0, };
    char flag = 0;
    char select = 0;
    char sys = 0;
    char num1[3];
    char num2[3];
    char num3[3];
    char go[100] = "date -s \"";
    clock_t start;
    dev = open("/dev/hex", O_RDWR);
    key = open("/dev/key", O_RDWR);
    if (dev < 0) {
        fprintf(stderr, "cannot open hex device\n");
        return 1;
    }
    if (key < 0) {
        fprintf(stderr, "cannot open key device\n");
        return 1;
    }
    now = before = time(0);
    nt = localtime(&now);
    while (1) {
        rdata = 0;
        read(key, &rdata, 4);
        switch (rdata)
        {
        case 1:
            select = 1;
            break;
        case 2://왼쪽
            flag = 1;
            break;
        case 4://오른쪽
            sub_mode = ++sub_mode % 3;
            break;
        case 8://모드변경
            flag = 0;
            select = 0;
            sys = 0;
            mode = (mode + 1) % 4;
            cap = time(0);
            sub_mode = 0;
            tmp[0] = 0;
            tmp[1] = 0;
            break;
            
        default:
            break;
        }
        if(mode == 0)
        {
            now = time(0);
            if (now != before)
            {
                int data = 0;
                nt = localtime(&now);
                if (sub_mode == 0)
                    data = (nt->tm_hour + add[0]) % 24 * 10000 + (nt->tm_min + add[1]) % 60 * 100 + nt->tm_sec;
                else if (sub_mode == 1)
                    data = nt->tm_year % 100 * 10000 + (nt->tm_mon + 1) * 100 + nt->tm_mday;
                else
                    sub_mode = 0;
                write(dev, &data, 4);
                before = now;
            }
            if (alarm[0] == (nt->tm_hour + add[0]) % 24 && alarm[1]==(nt->tm_min + add[1]) % 60 && nt->tm_sec < 59&&fl ==0) {
                ioctl(dev, BLINK, NULL);
                fl = 1;
            }
            else if (fl == 1) {
                fl = 0;
                ioctl(dev, 0, NULL);
            }
        }
        else if(mode == 1)
        {
            if (sub_mode == 0)
            {
                data = 0;
                start = clock();
                write(dev, &data, 4);
            }
            else if (sub_mode == 1) {
                clock_t end = clock();
                int time = (end - start / CLOCKS_PER_SEC)/10000;
                write(dev, &time, 4);
            }
        }
        else if(mode == 2)
        {
            nt = localtime(&cap);
            now = time(0);
            if (sub_mode == 0)
            {
                tmp[0] += flag;
                flag = 0;
                data = (nt->tm_hour + tmp[0]) % 24 * 10000 + (nt->tm_min + tmp[1]) % 60 * 100 + nt->tm_sec;
                write(dev, &data, 4);
            }
            else if (sub_mode == 1)
            {
                tmp[1] += flag;
                flag = 0;
                data = (nt->tm_hour + tmp[0]) % 24 * 10000 + (nt->tm_min + tmp[1]) % 60 * 100 + nt->tm_sec;
                write(dev, &data, 4);
            }
            if (select == 1)
            {
                add[0] = (add[0] + tmp[0]) % 24;
                add[1] = (add[1] + tmp[1]) % 60;
                tmp[0] = 0;
                tmp[1] = 0;
                select = 0;
                sprintf(num1,"%d",(nt->tm_hour + add[0]) % 24);
                sprintf(num2, "%d", (add[1] + tmp[1]) % 60);
                sprintf(num3, "%d", nt->tm_sec);
                strcat(go,num1);
                strcat(go, ":");
                strcat(go, num2);
                strcat(go, ":");
                strcat(go, num3);
                strcat(go, "\"");
                system(go);
                add[0] = 0;
                add[1] = 0;
            }
            
        }
        else if (mode == 3)
        {
            nt = localtime(&cap);
            data = (nt->tm_hour + tmp[0]) % 24 * 10000 + (nt->tm_min + tmp[1]) % 60 * 100 + nt->tm_sec;
            write(dev, &data, 4);
            now = time(0);
            if (sub_mode == 0)
            {
                tmp[0] +=flag;
                flag = 0;
            }
            else if (sub_mode == 1) 
            {
                tmp[1] += flag;
                flag = 0;
            }
            if (select == 1) 
            {
                alarm[0] = (nt->tm_hour + add[0] + tmp[0]) % 24;
                alarm[1] = (nt->tm_min + add[1] + tmp[1]) % 60;
                tmp[0] = 0;
                tmp[1] = 0;
                select = 0;
            }
        }
    }
    return 0;
}