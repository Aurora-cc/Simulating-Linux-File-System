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
#define MAX_PATH_LENGTH		256	//·������󳤶�
#define MAX_NAME_LENGTH		128	//�ļ�����Ŀ¼������󳤶�
#define MAX_SUBITEM_NUM		256	//Ŀ¼�����ļ����������
#define MAX_COMMAND_LENGTH	128	//�������󳤶�
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
	init,//��ʼ��
	r,//�ɶ�
	w,//��д
	x, //��ִ��
};

struct user {
	char username[10];//�û���
	int uid;//�û�ID��
	int password;//�û�����
	bool ACL[USER_RIGHT_COUNT][1024];//Ȩ�޹����S
};

struct SharedData {
	//int user_count = 0;
	bool updated1[USER_NUM];//shell�Ƿ���smidisk��������
	bool updated2[USER_NUM];//smidisk�Ƿ���shell���ͳ������н��
	char if_tip[USER_NUM][5];//smidisk��shell������ʾ��Ϣ
	char if_continue[USER_NUM][5];//shell��smidisk�����Ƿ������������ִ��
	bool file[USER_NUM];//��¼shell�Ƿ���Ҫ�������ļ�
	char curDirPath[USER_NUM][BUFFER_SIZE];//��¼�����û���ǰ��·��
	char buffer1[USER_NUM][BUFFER_SIZE];//����shell��smidisk���͵�����
	char buffer2[USER_NUM][BUFFER_SIZE];//����smidisk��shell���͵ĳ������н��
	char fileContent[USER_NUM][BUFFER_SIZE];//��¼shellд����ļ�����
	user userTable[USER_NUM];//��¼�û���Ϣ��Ȩ��
};

static const char* cmdCommands[COMMAND_COUNTS] =
{
    "init", "info", "cd", "dir", "md", "rd", "newfile","cat", "copy", "del", "check", "exit"
};