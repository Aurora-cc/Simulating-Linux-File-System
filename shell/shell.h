#pragma once
#pragma once
#include <sstream>
#include <string.h>
#include <conio.h>
#include<thread>
#include<mutex>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<time.h>
#include <fstream>

using namespace std;
#define USER_NUM 5
#define BUFFER_SIZE 1024
#define PROCESS_NUM 10
#define EVENT_NAME L"MySharedMemoryEvent"
#define EVENT_NAME L"MySharedMemoryEvent"
#define UPDATE_FLAG_NAME L"MySharedMemoryUpdateFlag"
#define COMMAND_COUNTS 12
#define MAX_PATH_LENGTH		256	//路径的最大长度
#define MAX_NAME_LENGTH		128	//文件名、目录名的最大长度
#define MAX_SUBITEM_NUM		256	//目录包含文件的最大数量
#define MAX_COMMAND_LENGTH	128	//命令的最大长度
#define INODES_NUM          1024
#define USER_RIGHT_COUNT    4

int readers = 0;
int readerIndex = 0;
int writerIndex = 0;
HANDLE readMutex;
HANDLE writeMutex;
HANDLE resourceMutex;
string path = "root";
int userIndex = -1;
const char mutex1[] = "MyMutex1";

enum userRight
{
	init,//初始化
	r,//可读
	w,//可写
	x, //可执行
};

struct user {
	char username[10];//用户名
	int uid;//用户ID号
	int password;//用户密码
	bool ACL[USER_RIGHT_COUNT][1024];//权限管理表S
};

struct SharedData {
	//int user_count = 0;
	bool updated1[USER_NUM];//shell是否向smidisk发送命令
	bool updated2[USER_NUM];//smidisk是否向shell发送程序运行结果
	char if_tip[USER_NUM][5];//smidisk向shell发送提示信息
	char if_continue[USER_NUM][5];//shell向smidisk发送是否继续进行命令执行
	bool file[USER_NUM];//记录shell是否需要创建新文件
	char curDirPath[USER_NUM][BUFFER_SIZE];//记录各个用户当前的路径
	char buffer1[USER_NUM][BUFFER_SIZE];//储存shell向smidisk发送的命令
	char buffer2[USER_NUM][BUFFER_SIZE];//储存smidisk向shell发送的程序运行结果
	char fileContent[USER_NUM][BUFFER_SIZE];//记录shell写入的文件内容
	user userTable[USER_NUM];//记录用户信息与权限
};

static const char* cmdCommands[COMMAND_COUNTS] =
{
    "init", "info", "cd", "dir", "md", "rd", "newfile","cat", "copy", "del", "check", "exit"
};