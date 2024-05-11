#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <ctime>
#include <conio.h>
#include <string>
#include "simdisk.h"
using namespace std;

string error(error_num errNum, const char* strArg1, const char* strArg2)
{
	string str = "";
	switch (errNum)
	{
	case UNKNOW_ERROR:
		str = "����δ֪error";
		break;
	case INIT_FAIL:
		str = "��û�г�ʼ��Ȩ�ޣ���ʼ��ʧ��";
		break;
	case FATAL_READ:
		str = "��ȡģ�� Linux �ļ�ϵͳʧ��";
		wait(2);
		exit(0);
		break;
	case FATAL_WRITE:
		str = "д��ģ�� Linux �ļ�ϵͳʧ��";
		wait(2);
		exit(0);
		break;
	case DIR_READONLY:
		str.append("error��Ŀ¼ ");
		str.append(strArg1);
		str.append(" ֻ�����޷��޸�");
		break;
	case FILE_READONLY:
		str.append("error���ļ� ");
		str.append(strArg1);
		str.append(" ֻ�����޷�ɾ��");
		break;
	case DIR_WRITEONLY:
		str.append("error��Ŀ¼ ");
		str.append(strArg1);
		str.append(" ֻд���޷���ȡ");
		break;
	case FILE_WRITEONLY:
		str.append("error���ļ� ");
		str.append(strArg1);
		str.append(" ֻд���޷���ȡ");
		break;
	case CD_FAILED:
		str = "�л�Ŀ¼ʧ��";
		break;
	case DIR_FAILED:
		str = "Ŀ¼��ʾʧ��";
		break;
	case MD_FAILED:
		str = "Ŀ¼����ʧ��";
		break;
	case RD_FAILED:
		str = "Ŀ¼ɾ��ʧ��";
		break;
	case NEW_FILE_FAILED:
		str = "�ļ�����ʧ��";
		break;
	case CAT_FAILED:
		str = "�ļ���ʧ��";
		break;
	case DEL_FAILED:
		str = "�ļ�ɾ��ʧ��";
		break;
	case COPY_FAILED:
		str = "�ļ�����ʧ��";
		break;
	case FILE_EXIST:
		str.append("error��Ŀ¼ ");
		str.append(strArg1);
		str.append(" ���Ѵ�����Ϊ ");
		str.append(strArg2);
		str.append(" ���ļ���Ŀ¼");
		break;
	case NOT_BOTH_HOST:
		str = "error��������Դ��Ŀ�겻�ܾ�Ϊ��������";
		break;
	case HOST_FILE_NOT_EXIST:
		str.append("error���������������ļ� ");
		str.append(strArg1);
		str.append(" ���ļ�����ʧ��");
		break;
	case HOST_FILE_WRITE_FAILED:
		str.append("error��������д���ļ�");
		str.append(strArg1);
		str.append(" �����ļ�����ʧ��");
		break;
	case FILE_NOT_EXIST:
		str.append("error��Ŀ¼ ");
		str.append(strArg1);
		str.append(" �²�������Ϊ ");
		str.append(strArg2);
		str.append(" ���ļ���Ŀ¼");
		break;
	case DIR_NOT_EXIST:
		str.append("error��Ŀ¼ ");
		str.append(strArg1);
		str.append("������");
		break;
	case PATH_NOT_FOUND:
		str = "error���Ҳ���ָ��·��";
		break;
	case NO_DEL_CUR:
		str = "error������ɾ����ǰĿ¼";
		break;
	case ILLEGAL_FILENAME:
		str.append("error���Ƿ��ļ�����\n�ļ����в��ܺ����ַ���/�������Ȳ��ܳ��� ");
		str.append(to_string(MAX_NAME_LENGTH));
		str.append(" ���ַ�\n");
		break;
	case SPACE_NOT_ENOUGH:
		str = "error�����̿ռ䲻�㣡";
		break;
	case INODE_ALLOC_FAILED:
		str = "error��i�ڵ����ʧ�ܣ�";
		break;
	case INVILID_CMD:
		str.append("error����Ч������ ");
		str.append(strArg1);
		break;
	case TOO_MANY_ARGS:
		str = "error��������������";
		break;
	case WRONG_ARGS:
		str = "error����������";
		break;
	case WRONG_COMMANDLINE:
		str = "�����в�������";
		break;
	case READ_DIR_ACCESS_DENIED:
		str = "���û���Ȩ�޶�ȡ��Ŀ¼";
		break;
	case WRITE_DIR_ACCESS_DENIED:
		str = "���û���Ȩ�޴������޸�Ŀ¼";
		break;
	case EXEC_DIR_ACCESS_DENIED:
		str = "���û���Ȩ�޽����Ŀ¼";
		break;
	case READ_FILE_ACCESS_DENIED:
		str = "���û���Ȩ�޶����ļ�";
		break;
	case WRITE_FILE_ACCESS_DENIED:
		str = "���û���Ȩ�޴������޸��ļ�";
		break;
	}
	return str;
}