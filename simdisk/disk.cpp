#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <conio.h>
#include "simdisk.h"
using namespace std;

//加载
void load()
{
	system("cls");
	virDisk.open("virtual_disk.bin", ios::in | ios::binary);
	if (!virDisk.is_open())
	{
		while (strlen(pSharedData->if_continue[0]) == 0)
		{

		}
		if (strcmp(pSharedData->if_continue[0], "T\0") == 0)
		{
			virDisk.clear();
			cmd_init(0);
			return;
		}
		else
		{
			error(FATAL_READ);
		}
	}
	unsigned int i;
	for (i = 0; i < BLOCK_GROUPS_NUM; i++)
		virDisk.read(reinterpret_cast<char*>(&dataBlockGroups[i]), sizeof(blockGroup));
	for (i = 0; i < INODES_NUM; i++)
		virDisk.read(reinterpret_cast<char*>(&inodeBitmap[i]), sizeof(bitmapStatus));
	for (i = 0; i < INODES_NUM; i++)
		virDisk.read(reinterpret_cast<char*>(&inodeTable[i]), sizeof(inode));
	for (int i = 0; i < BLOCKS_NUM; i++)
		virDisk.read(reinterpret_cast<char*>(&dataBlockBitmap[i]), sizeof(bitmapStatus));
	virDisk.read(reinterpret_cast<char*>(&dirCurPath), sizeof(dir));
	virDisk.close();
	//复制当前路径
	strcpy(curPath, inodeTable[dirCurPath.inodeNum].name);

	fstream userInfo;
	userInfo.open("user.bin", ios::in | ios::binary);
	for (int i = 0; i < USER_NUM; i++)
	{
		userInfo.read(reinterpret_cast<char*>(&pSharedData->userTable[i]), sizeof(user));
	}
	userInfo.close();

}

//退出程序
void cmd_exit()
{
	system("cls");
	exit(0);
}

//设置日期时间
void date::set_date_time(tm t)
{
	this->year = t.tm_year;
	this->mon = t.tm_mon;
	this->day = t.tm_mday;
	this->hour = t.tm_hour;
	this->min = t.tm_min;
	this->sec = t.tm_sec;
}

//获取日期时间
tm date::get_date_time()
{
	tm t;
	t.tm_year = this->year;
	t.tm_mon = this->mon;
	t.tm_mday = this->day;
	t.tm_hour = this->hour;
	t.tm_min = this->min;
	t.tm_sec = this->sec;
	return t;
}