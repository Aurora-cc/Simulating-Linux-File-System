#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <fstream>
#include <ctime>
#include <windows.h>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

#define USER_NUM 5
#define EVENT_NAME L"MySharedMemoryEvent"
#define UPDATE_FLAG_NAME L"MySharedMemoryUpdateFlag"
#define PASSWORD "123456"
#define BUFFER_SIZE 1024
#define MAX_PATH_LENGTH		256	//路径的最大长度
#define MAX_NAME_LENGTH		128	//文件名、目录名的最大长度
#define MAX_SUBITEM_NUM		256	//目录包含文件的最大数量
#define MAX_COMMAND_LENGTH	128	//命令的最大长度
#define COMMAND_COUNTS		13	//命令的数量
#define BLOCK_SIZE			1024//盘块大小1KB
#define DIR_SIZE			(sizeof(dir) / BLOCK_SIZE + 1) //目录大小

#define BLOCKS_EACH			1024//每个数据块组的盘块数
#define INODES_EACH			1024//每个数据块组的i结点数
#define BLOCK_GROUPS_NUM	100	//数据块组数
#define BLOCKS_NUM			(BLOCKS_EACH * BLOCK_GROUPS_NUM)//盘块总数
#define INODES_NUM			(INODES_EACH * BLOCK_GROUPS_NUM)//i结点总数
#define USERS_NUM            100 //用户数量
#define USER_RIGHT_COUNT    4
//数据区首地址
#define DATA_AREA_ADDR		(sizeof(blockGroup) * BLOCK_GROUPS_NUM + sizeof(bitmapStatus) * (INODES_NUM + BLOCKS_NUM) + sizeof(inode) * INODES_NUM)

//用户权限
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
	bool ACL[USER_RIGHT_COUNT][1024];//权限管理表
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
	"init", "info", "cd", "dir", "md", "rd", "newfile","cat", "copy", "del", "check", "exit", "help"
};

//文件属性
enum fileAttribute
{
	PROTECTED,//系统保护
	READ_WRITE,//允许读写
	READ_ONLY,//只读
	WRITE_ONLY,//只写
	SHARE//可共享
};

//文件类型
enum fileType
{
	TYPE_FILE = 1,//文件
	TYPE_DIR = 2//目录
};

enum error_num
{
	UNKNOW_ERROR,//未知错误
	INIT_FAIL,//初始化失败
	FATAL_READ,	//无法读取虚拟磁盘
	FATAL_WRITE,//无法写入虚拟磁盘
	DIR_READONLY,//目录只读
	FILE_READONLY,//文件只读
	DIR_WRITEONLY,//目录只写
	FILE_WRITEONLY,//文件只写
	CD_FAILED,//切换目录失败
	DIR_FAILED,//显示目录失败
	MD_FAILED,//创建目录失败
	RD_FAILED,//删除目录失败
	NEW_FILE_FAILED,//创建文件失败
	CAT_FAILED,//显示文件失败
	DEL_FAILED,//删除文件失败
	COPY_FAILED,//复制文件失败
	FILE_EXIST,	//文件已存在
	NOT_BOTH_HOST,//不能均为宿主机文件
	HOST_FILE_NOT_EXIST,//宿主机文件不存在
	HOST_FILE_WRITE_FAILED,//宿主机文件写入失败
	FILE_NOT_EXIST,//文件不存在
	DIR_NOT_EXIST,//目录不存在
	PATH_NOT_FOUND,//找不到路径
	NO_DEL_CUR,//不能删除当前目录
	ILLEGAL_FILENAME,//非法文件名
	SPACE_NOT_ENOUGH,//磁盘空间不足
	INODE_ALLOC_FAILED,	//i结点分配失败
	INVILID_CMD,//无效命令
	TOO_MANY_ARGS,//参数过多
	WRONG_ARGS,	//参数错误
	WRONG_COMMANDLINE,//命令行参数错误
	READ_DIR_ACCESS_DENIED,//用户无权限读目录
	WRITE_DIR_ACCESS_DENIED,//用户无权限更改目录
	EXEC_DIR_ACCESS_DENIED,//用户无权限进入目录
	READ_FILE_ACCESS_DENIED,//用户无权限读文件
	WRITE_FILE_ACCESS_DENIED,//用户无权限写文件
};

// 位图
enum bitmapStatus
{
	NOT_USED = 0,//未使用
	USED = 1//已被使用
};

//时间
class date
{
private:
	unsigned int sec;
	unsigned int min;
	unsigned int hour;
	unsigned int day;
	unsigned int mon;
	unsigned int year;

public:
	void set_date_time(tm t);//设置时间
	tm get_date_time();	//获取时间
};

//目录
class dir
{
public:
	unsigned int inodeNum;	//i结点号
	unsigned int nSize;		//子文件,子目录总数
	unsigned int nSubInode[MAX_SUBITEM_NUM];	//子项目i结点

	long open_file(unsigned int inodeNum, char* strBuffer);	//打开子文件
	void save_file(const char* strFileName, char* strBuffer, unsigned long lFileLen, fileAttribute privilege);//保存子文件
	void delete_file(const char* strFileName);//删除子文件
	void remove_dir(dir dirRemove, unsigned int nIndex);//删除子目录
	bool have_child(const char* strDirName);//是否已存在子文件、子目录
};

//i结点
class inode
{
public:
	unsigned long byte;//大小
	unsigned long diskAddr;//磁盘块起始地址
	unsigned int blockNum;//占用磁盘块数
	char name[MAX_NAME_LENGTH];//目录名或文件名
	date createdTime;//创建时间
	fileAttribute privilege;//读写权限
	fileType type;//类型
};

//超级块
class superBlock
{
public:
	unsigned int totalFreeBlockCount;//空闲块数
	unsigned int totalFreeInodeCount;//空闲i结点数
};

//数据块组信息
class blockInfo
{
public:
	unsigned int blockBmpIndex;	//数据块位图索引
	unsigned int inodeBmpIndex;	//i结点位图索引
	unsigned int inodeTableIndex;//i结点表索引
	unsigned long blockAddr;//所在数据区地址
	unsigned int freeBlockCount;//空闲块数
	unsigned int freeInodeCount;//空闲i结点数
};

//数据块组
class blockGroup
{
public:
	blockInfo block_info;//数据块组信息
	superBlock super_block;//超级块
};

extern char curPath[MAX_PATH_LENGTH];//当前目录路径字符串
extern dir dirCurPath;//当前目录
extern fstream virDisk;//虚拟磁盘
extern blockGroup dataBlockGroups[BLOCK_GROUPS_NUM];//数据块组
extern inode inodeTable[INODES_NUM];//i结点表
extern bitmapStatus dataBlockBitmap[BLOCKS_NUM];	//数据块位图数组
extern bitmapStatus inodeBitmap[INODES_NUM];	//i结点位图数组
extern user userTable[USER_NUM];
extern HANDLE hMapFile;
extern SharedData* pSharedData;
extern HANDLE hUpdateFlag;
const unsigned int iCmdLength = MAX_COMMAND_LENGTH + MAX_PATH_LENGTH * 2 + 2;
//extern HANDLE semaphoreUserName;
//extern HANDLE semaphoreBuffer1;
//extern HANDLE semaphoreBuffer2;
//extern HANDLE semaphoreFile; 
//extern HANDLE semaphoreTip;
extern HANDLE reader;
extern HANDLE writer;
extern int readerNum;
extern int writerNum;

void cmd_init(int index);//初始化虚拟磁盘文件
string cmd_info();//查看磁盘信息
string cmd_cd(int index, const char* strPath);//切换工作目录
void cmd_dir(int index, string& str, dir dirTemp, string prefix, const char* strPath);//查看目录信息
string cmd_md(int index, const char* strPath, fileAttribute privilege);//创建目录
string cmd_rd(int index, const char* strPath);//移除目录
string cmd_newfile(int index, const char* strPath, fileAttribute privilege);//创建新文件
string cmd_cat(int index, const char* strPath);//查看文件
string cmd_copy(int index, const char* strSrcPath, const char* strDesPath);//拷贝文件
string cmd_del(int index, const char* strPath);//删除文件
string cmd_check();//检查信息一致性
void cmd_exit();//退出系统
string cmd_help();//查看所有命令及其相应功能

//加载虚拟磁盘信息
void load();

//命令执行函数
string execute(int index, const char* comm, const char* p1, const char* p2);

//分配以及释放内存的函数
int alloc_inode();//分配i结点
void free_inode(unsigned int nInode);//释放i结点
long alloc_block(unsigned int nLen, unsigned int& nIndex);//分配数据块
void free_block(unsigned int nLen, unsigned int nIndex);//释放数据块

//路径处理函数
void set_cur_path(dir dirTemp);//设置工作路径为dirTemp
bool if_host_path(char* strPath);//检查是否为宿主机下的文件路径
bool get_dir_name(const char* strPath, size_t nPathLen, size_t nPos, char* strFileName);//获取指定位置下的文件名
bool path_to_dir(const char* strPath, size_t nPathLen, size_t& nPos, char* strDirName, dir& dirTemp);//将路径（字符串）转换为目录（dir类）
bool analyse_path(const char* strPath, size_t nPathLen, dir& dirTemp, char* strFileName);//找到strPath路径下的最后一个目录，并返回最后一个文件名
bool analyse_path(const char* strPath, size_t nPathLen, dir& dirTemp);//找到strPath路径下的最后一个目录

//错误提示函数
string error(error_num errNum, const char* strArg1 = "", const char* strArg2 = "");
bool is_hex_num(char chNum);

//延时函数
void wait(double dTime);