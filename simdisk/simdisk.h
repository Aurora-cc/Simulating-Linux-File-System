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
#define MAX_PATH_LENGTH		256	//·������󳤶�
#define MAX_NAME_LENGTH		128	//�ļ�����Ŀ¼������󳤶�
#define MAX_SUBITEM_NUM		256	//Ŀ¼�����ļ����������
#define MAX_COMMAND_LENGTH	128	//�������󳤶�
#define COMMAND_COUNTS		13	//���������
#define BLOCK_SIZE			1024//�̿��С1KB
#define DIR_SIZE			(sizeof(dir) / BLOCK_SIZE + 1) //Ŀ¼��С

#define BLOCKS_EACH			1024//ÿ�����ݿ�����̿���
#define INODES_EACH			1024//ÿ�����ݿ����i�����
#define BLOCK_GROUPS_NUM	100	//���ݿ�����
#define BLOCKS_NUM			(BLOCKS_EACH * BLOCK_GROUPS_NUM)//�̿�����
#define INODES_NUM			(INODES_EACH * BLOCK_GROUPS_NUM)//i�������
#define USERS_NUM            100 //�û�����
#define USER_RIGHT_COUNT    4
//�������׵�ַ
#define DATA_AREA_ADDR		(sizeof(blockGroup) * BLOCK_GROUPS_NUM + sizeof(bitmapStatus) * (INODES_NUM + BLOCKS_NUM) + sizeof(inode) * INODES_NUM)

//�û�Ȩ��
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
	bool ACL[USER_RIGHT_COUNT][1024];//Ȩ�޹����
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
	"init", "info", "cd", "dir", "md", "rd", "newfile","cat", "copy", "del", "check", "exit", "help"
};

//�ļ�����
enum fileAttribute
{
	PROTECTED,//ϵͳ����
	READ_WRITE,//�����д
	READ_ONLY,//ֻ��
	WRITE_ONLY,//ֻд
	SHARE//�ɹ���
};

//�ļ�����
enum fileType
{
	TYPE_FILE = 1,//�ļ�
	TYPE_DIR = 2//Ŀ¼
};

enum error_num
{
	UNKNOW_ERROR,//δ֪����
	INIT_FAIL,//��ʼ��ʧ��
	FATAL_READ,	//�޷���ȡ�������
	FATAL_WRITE,//�޷�д���������
	DIR_READONLY,//Ŀ¼ֻ��
	FILE_READONLY,//�ļ�ֻ��
	DIR_WRITEONLY,//Ŀ¼ֻд
	FILE_WRITEONLY,//�ļ�ֻд
	CD_FAILED,//�л�Ŀ¼ʧ��
	DIR_FAILED,//��ʾĿ¼ʧ��
	MD_FAILED,//����Ŀ¼ʧ��
	RD_FAILED,//ɾ��Ŀ¼ʧ��
	NEW_FILE_FAILED,//�����ļ�ʧ��
	CAT_FAILED,//��ʾ�ļ�ʧ��
	DEL_FAILED,//ɾ���ļ�ʧ��
	COPY_FAILED,//�����ļ�ʧ��
	FILE_EXIST,	//�ļ��Ѵ���
	NOT_BOTH_HOST,//���ܾ�Ϊ�������ļ�
	HOST_FILE_NOT_EXIST,//�������ļ�������
	HOST_FILE_WRITE_FAILED,//�������ļ�д��ʧ��
	FILE_NOT_EXIST,//�ļ�������
	DIR_NOT_EXIST,//Ŀ¼������
	PATH_NOT_FOUND,//�Ҳ���·��
	NO_DEL_CUR,//����ɾ����ǰĿ¼
	ILLEGAL_FILENAME,//�Ƿ��ļ���
	SPACE_NOT_ENOUGH,//���̿ռ䲻��
	INODE_ALLOC_FAILED,	//i������ʧ��
	INVILID_CMD,//��Ч����
	TOO_MANY_ARGS,//��������
	WRONG_ARGS,	//��������
	WRONG_COMMANDLINE,//�����в�������
	READ_DIR_ACCESS_DENIED,//�û���Ȩ�޶�Ŀ¼
	WRITE_DIR_ACCESS_DENIED,//�û���Ȩ�޸���Ŀ¼
	EXEC_DIR_ACCESS_DENIED,//�û���Ȩ�޽���Ŀ¼
	READ_FILE_ACCESS_DENIED,//�û���Ȩ�޶��ļ�
	WRITE_FILE_ACCESS_DENIED,//�û���Ȩ��д�ļ�
};

// λͼ
enum bitmapStatus
{
	NOT_USED = 0,//δʹ��
	USED = 1//�ѱ�ʹ��
};

//ʱ��
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
	void set_date_time(tm t);//����ʱ��
	tm get_date_time();	//��ȡʱ��
};

//Ŀ¼
class dir
{
public:
	unsigned int inodeNum;	//i����
	unsigned int nSize;		//���ļ�,��Ŀ¼����
	unsigned int nSubInode[MAX_SUBITEM_NUM];	//����Ŀi���

	long open_file(unsigned int inodeNum, char* strBuffer);	//�����ļ�
	void save_file(const char* strFileName, char* strBuffer, unsigned long lFileLen, fileAttribute privilege);//�������ļ�
	void delete_file(const char* strFileName);//ɾ�����ļ�
	void remove_dir(dir dirRemove, unsigned int nIndex);//ɾ����Ŀ¼
	bool have_child(const char* strDirName);//�Ƿ��Ѵ������ļ�����Ŀ¼
};

//i���
class inode
{
public:
	unsigned long byte;//��С
	unsigned long diskAddr;//���̿���ʼ��ַ
	unsigned int blockNum;//ռ�ô��̿���
	char name[MAX_NAME_LENGTH];//Ŀ¼�����ļ���
	date createdTime;//����ʱ��
	fileAttribute privilege;//��дȨ��
	fileType type;//����
};

//������
class superBlock
{
public:
	unsigned int totalFreeBlockCount;//���п���
	unsigned int totalFreeInodeCount;//����i�����
};

//���ݿ�����Ϣ
class blockInfo
{
public:
	unsigned int blockBmpIndex;	//���ݿ�λͼ����
	unsigned int inodeBmpIndex;	//i���λͼ����
	unsigned int inodeTableIndex;//i��������
	unsigned long blockAddr;//������������ַ
	unsigned int freeBlockCount;//���п���
	unsigned int freeInodeCount;//����i�����
};

//���ݿ���
class blockGroup
{
public:
	blockInfo block_info;//���ݿ�����Ϣ
	superBlock super_block;//������
};

extern char curPath[MAX_PATH_LENGTH];//��ǰĿ¼·���ַ���
extern dir dirCurPath;//��ǰĿ¼
extern fstream virDisk;//�������
extern blockGroup dataBlockGroups[BLOCK_GROUPS_NUM];//���ݿ���
extern inode inodeTable[INODES_NUM];//i����
extern bitmapStatus dataBlockBitmap[BLOCKS_NUM];	//���ݿ�λͼ����
extern bitmapStatus inodeBitmap[INODES_NUM];	//i���λͼ����
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

void cmd_init(int index);//��ʼ����������ļ�
string cmd_info();//�鿴������Ϣ
string cmd_cd(int index, const char* strPath);//�л�����Ŀ¼
void cmd_dir(int index, string& str, dir dirTemp, string prefix, const char* strPath);//�鿴Ŀ¼��Ϣ
string cmd_md(int index, const char* strPath, fileAttribute privilege);//����Ŀ¼
string cmd_rd(int index, const char* strPath);//�Ƴ�Ŀ¼
string cmd_newfile(int index, const char* strPath, fileAttribute privilege);//�������ļ�
string cmd_cat(int index, const char* strPath);//�鿴�ļ�
string cmd_copy(int index, const char* strSrcPath, const char* strDesPath);//�����ļ�
string cmd_del(int index, const char* strPath);//ɾ���ļ�
string cmd_check();//�����Ϣһ����
void cmd_exit();//�˳�ϵͳ
string cmd_help();//�鿴�����������Ӧ����

//�������������Ϣ
void load();

//����ִ�к���
string execute(int index, const char* comm, const char* p1, const char* p2);

//�����Լ��ͷ��ڴ�ĺ���
int alloc_inode();//����i���
void free_inode(unsigned int nInode);//�ͷ�i���
long alloc_block(unsigned int nLen, unsigned int& nIndex);//�������ݿ�
void free_block(unsigned int nLen, unsigned int nIndex);//�ͷ����ݿ�

//·��������
void set_cur_path(dir dirTemp);//���ù���·��ΪdirTemp
bool if_host_path(char* strPath);//����Ƿ�Ϊ�������µ��ļ�·��
bool get_dir_name(const char* strPath, size_t nPathLen, size_t nPos, char* strFileName);//��ȡָ��λ���µ��ļ���
bool path_to_dir(const char* strPath, size_t nPathLen, size_t& nPos, char* strDirName, dir& dirTemp);//��·�����ַ�����ת��ΪĿ¼��dir�ࣩ
bool analyse_path(const char* strPath, size_t nPathLen, dir& dirTemp, char* strFileName);//�ҵ�strPath·���µ����һ��Ŀ¼�����������һ���ļ���
bool analyse_path(const char* strPath, size_t nPathLen, dir& dirTemp);//�ҵ�strPath·���µ����һ��Ŀ¼

//������ʾ����
string error(error_num errNum, const char* strArg1 = "", const char* strArg2 = "");
bool is_hex_num(char chNum);

//��ʱ����
void wait(double dTime);