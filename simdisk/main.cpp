#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <sstream>
#include <string>
#include <conio.h>
#include <stdio.h>
#include "simdisk.h"
using namespace std;

fstream virDisk;//��������ļ�
char curPath[MAX_PATH_LENGTH];//��ǰĿ¼·��
dir dirCurPath;//��ǰĿ¼
blockGroup dataBlockGroups[BLOCK_GROUPS_NUM];//���ݿ���
inode inodeTable[INODES_NUM];//i����
bitmapStatus dataBlockBitmap[BLOCKS_NUM];//���ݿ�λͼ����
bitmapStatus inodeBitmap[INODES_NUM];//i���λͼ����
SharedData* pSharedData;//����ռ�

int readerNum = 0;
int writerNum = 0;

HANDLE reader = CreateSemaphore(NULL, 1, 1, L"reader");

HANDLE writer = CreateSemaphore(NULL, 1, 1, L"writer");

//ִ������
string execute(int index, const char* comm, const char* p1, const char* p2)
{
	string res = "";
	//��ȡ������
	unsigned int i = 0;
	for (i = 0; i < COMMAND_COUNTS; i++)
	{
		if (strcmp(cmdCommands[i], comm) == 0)//�õ�������֮��ֹͣ����
		{
			break;
		}
	}
	char input_cmd = '\0';

	dir curDir;
	cmd_cd(index, pSharedData->curDirPath[index]);
	char curPathTemp[MAX_PATH_LENGTH];
	strcpy(curPathTemp, curPath);
	dir dirTemp = dirCurPath;
	/*analyse_path(pSharedData->curDirPath[index], strlen(pSharedData->curDirPath[index]), curDir);
	set_cur_path(curDir);*/
	//strcpy(curPath, pSharedData->curDirPath[index]);

	//���ݱ��ִ������
	switch (i)
	{
	case 0:
		//init
		if (p1[0] != '\0')//��Ϊֻ��һ��������p1����Ϊ�����������
		{
			return error(WRONG_ARGS, comm);//��������
		}
		if (pSharedData->userTable[index].ACL[0][0] == false)
		{
			cout << "��û�г�ʼ��Ȩ�ޣ���ʼ��ʧ��" << endl;
			break;

		}
		cmd_init(index);
		break;
	case 1:
		//info
		WaitForSingleObject(reader, INFINITE);
		readerNum++;
		if (readerNum == 1)
		{
			WaitForSingleObject(writer, INFINITE);
		}
		ReleaseSemaphore(reader, 1, NULL);

		if (p1[0] != '\0')
		{
			return error(WRONG_ARGS, comm);
		}
		res = cmd_info();

		dir root;
		analyse_path("root\0", strlen("root\0"), root);

		res.append(inodeTable[root.inodeNum].name);
		res.append(" ��Ŀ¼");
		res.append("\n");

		res.append("����ʱ��");

		res.append("\t\tĿ¼��/�ļ���");

		res.append("\t����");

		res.append("\t��С���ֽڣ�");

		res.append("\t����");

		res.append("\n");
		
		cmd_dir(index, res, root, "", "/s\0");

		cmd_cd(index, curPathTemp);

		WaitForSingleObject(reader, INFINITE);
		readerNum--;
		if (readerNum == 0)
		{
			ReleaseSemaphore(writer, 1, NULL);
		}
		ReleaseSemaphore(reader, 1, NULL);

		break;
	case 2:
		//cd
		if (p2[0] != '\0')
		{
			return error(WRONG_ARGS, comm);
		}

		WaitForSingleObject(writer, INFINITE);
		cmd_cd(index, p1);
		ReleaseSemaphore(writer, 1, NULL);

		break;
	case 3:
		//dir
		if (p2[0] != '\0')
		{
			res = error(WRONG_ARGS, comm);
			return res;
		}
		if (pSharedData->userTable[index].ACL[3][dirTemp.inodeNum] == false)
		{
			res = error(EXEC_DIR_ACCESS_DENIED);
			return res;
		}
		if (inodeTable[dirTemp.inodeNum].privilege == WRITE_ONLY)
		{
			res = error(EXEC_DIR_ACCESS_DENIED);
			return res;
		}



		res.append("\n");
		res.append(inodeTable[dirTemp.inodeNum].name);
		res.append("��Ŀ¼:");
		res.append("\n\n");

		res.append("����ʱ��");

		res.append("\t\tĿ¼��/�ļ���");

		res.append("\t����");

		res.append("\t��С���ֽڣ�");

		res.append("\t����");

		res.append("\t\t�����ַ");

		res.append("\n");

		WaitForSingleObject(writer, INFINITE);
		if (p1[0] == '\0')
			cmd_dir(index, res, dirTemp, "", ".");
		else
			cmd_dir(index, res, dirTemp, "", p1);

		cmd_cd(index, curPathTemp);
		ReleaseSemaphore(writer, 1, NULL);
		
		

		break;
	case 4:
		//md
		if (p1[0] == '\0')
		{
			res = error(WRONG_ARGS, comm);
			return res;
		}

		WaitForSingleObject(writer, INFINITE);
		
		if (p2[0] != '\0')
		{
			if (strcmp("/r", p2) == 0)
			{
				res = cmd_md(index, p1, READ_ONLY);
				break;
			}
			if (strcmp("/w", p2) == 0)
			{
				res = cmd_md(index, p1, WRITE_ONLY);
				break;
			}
			if (strcmp("/a", p2) == 0)
			{
				res = cmd_md(index, p1, READ_WRITE);
				break;
			}
			if (strcmp("/s", p2) == 0)
			{
				res = cmd_md(index, p1, SHARE);
				break;
			}
			res = error(WRONG_ARGS, comm);
		}
		else
		{
			res = cmd_md(index, p1, READ_WRITE);
		}

		ReleaseSemaphore(writer, 1, NULL);

		break;
	case 5:
		//rd
		if (p1[0] == '\0' || p2[0] != '\0')
		{
			res = error(WRONG_ARGS, comm);
			return res;
		}

		WaitForSingleObject(writer, INFINITE);
		res = cmd_rd(index, p1);
		ReleaseSemaphore(writer, 1, NULL);

		break;
	case 6:
		//newfile
		if (p1[0] == '\0')
		{
			res = error(WRONG_ARGS, comm);
			return res;
		}

		WaitForSingleObject(writer, INFINITE);
		if (p2[0] != '\0')
		{
			if (strcmp("/r", p2) == 0)
			{
				res = cmd_newfile(index, p1, READ_ONLY);
				break;
			}
			if (strcmp("/w", p2) == 0)
			{
				res = cmd_newfile(index, p1, WRITE_ONLY);
				break;
			}
			if (strcmp("/a", p2) == 0)
			{
				res = cmd_newfile(index, p1, READ_WRITE);
				break;
			}
			if (strcmp("/s", p2) == 0)
			{
				res = cmd_newfile(index, p1, SHARE);
				break;
			}
			res = error(WRONG_ARGS, comm);
			return res;
		}
		res = cmd_newfile(index, p1, READ_WRITE);
		ReleaseSemaphore(writer, 1, NULL);
		
		break;
	case 7:
		//cat

		if (p1[0] == '\0' || p2[0] != '\0')
		{
			res = error(WRONG_ARGS, comm);
			return res;
		}

		WaitForSingleObject(reader, INFINITE);
		readerNum++;
		if (readerNum == 1)
		{
			WaitForSingleObject(writer, INFINITE);
		}
		ReleaseSemaphore(reader, 1, NULL);
		
		res = cmd_cat(index, p1);

		WaitForSingleObject(reader, INFINITE);
		readerNum--;
		if (readerNum == 0)
		{
			ReleaseSemaphore(writer, 1, NULL);
		}
		ReleaseSemaphore(reader, 1, NULL);
		break;
	case 8:
		//copy
		if (p2[0] == '\0')
		{
			res = error(WRONG_ARGS, comm);
			return res;
		}

		WaitForSingleObject(writer, INFINITE);
		
		res = cmd_copy(index, p1, p2);

		ReleaseSemaphore(writer, 1, NULL);

		break;
	case 9:
		//del
		
		if (p1[0] == '\0' || p2[0] != '\0')
		{
			res = error(WRONG_ARGS, comm);
			return res;
		}

		WaitForSingleObject(writer, INFINITE);

		res = cmd_del(index, p1);

		ReleaseSemaphore(writer, 1, NULL);

		break;
	case 10:
		//check
		if (p1[0] != '\0')
		{
			res = error(WRONG_ARGS, comm);
			return res;
		}

		WaitForSingleObject(writer, INFINITE);

		res = cmd_check();

		ReleaseSemaphore(writer, 1, NULL);

		break;
	case 11:
		//exit
		if (p1[0] != '\0')
		{
			res = error(WRONG_ARGS, comm);
			return res;
		}
		cmd_exit();
		break;
	case 12:
		//help
		res = cmd_help();
		break;
	default:
		res = error(INVILID_CMD, comm);
	}
	return res;
}

 //�̺߳���
DWORD WINAPI ThreadFunction(LPVOID lpParam) {
	string res;
	int threadId = reinterpret_cast<int>(lpParam);
	stringstream ssStream;
	char commLine[MAX_COMMAND_LENGTH + MAX_PATH_LENGTH + MAX_PATH_LENGTH], userName[20];
	char comm[MAX_COMMAND_LENGTH], p1[MAX_PATH_LENGTH], p2[MAX_PATH_LENGTH];

	while (1)
	{
		stringstream ssStream;


		while (pSharedData->updated1[threadId] == false)
		{

		}

		int i = 0;
		for (; i < strlen(pSharedData->buffer1[threadId]); i++)
		{
			commLine[i] = pSharedData->buffer1[threadId][i];
		}
		commLine[i] = '\0';
		pSharedData->buffer1[threadId][0] = '\0';
		pSharedData->updated1[threadId] = false;

		ssStream << commLine;
		ssStream >> comm >> p1 >> p2;

		strcpy(userName, (const char*)pSharedData->userTable[threadId].username);

		cout << endl << endl << endl << endl << "����ִ��" << userName << "���ݵ�����:" << commLine << endl;

		//������Ϊ�մ�
		if (comm[0] == '\0')
		{
			ssStream.clear();
			continue;
		}
		//��������������
		else if (!ssStream.eof())
		{
			error(TOO_MANY_ARGS);
			ssStream.str("");
			ssStream.clear();
			continue;
		}

		res = execute(threadId, comm, p1, p2);

		cout << res << endl;

		i = 0;
		for (; i < res.length(); i++)
		{
			pSharedData->buffer2[threadId][i] = res[i];
		}
		pSharedData->buffer2[threadId][i] = '\0';

		pSharedData->updated2[threadId] = true;

		res.clear();
	}
	return 0;
}

int main()
{
	HANDLE hMapFile;
	string res;

	// �򿪹����ڴ�
	hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,    // �ɶ�д����Ȩ��
		FALSE,                  // ���̳о��
		L"MyShareMemory");      // �����ڴ������

	if (hMapFile == NULL)
	{
		printf("Could not open file mapping object (%d).\n", GetLastError());
		return 1;
	}

	// ӳ�乲���ڴ浽���̵ĵ�ַ�ռ�
	pSharedData = (SharedData*)MapViewOfFile(
		hMapFile,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		sizeof(SharedData));

	if (pSharedData == NULL) {
		std::cout << "Could not map view of file (" << GetLastError() << ")." << std::endl;
		CloseHandle(hMapFile);
		return 1;
	}

	dirCurPath.inodeNum = 0;
	dirCurPath.nSize = 2;//��ǰĿ¼+��һ��Ŀ¼
	dirCurPath.nSubInode[0] = 0;
	dirCurPath.nSubInode[1] = 0;
	strcpy(curPath, "root");
	
	//����
	load();
	cin.sync();

	HANDLE threads[USER_NUM];
	for (int i = 0; i < USER_NUM; i++)
	{
		threads[i] = CreateThread(
			NULL,                   // Ĭ�ϰ�ȫ����
			0,                      // Ĭ�϶�ջ��С
			ThreadFunction,         // �̺߳���
			reinterpret_cast<LPVOID>(i),   // �����̲߳���
			0,                      // Ĭ�ϴ�����־
			NULL                    // �������̱߳�ʶ��
		);
	}

	// �ȴ������߳̽���
	WaitForMultipleObjects(USER_NUM, threads, TRUE, INFINITE);

	// ������Դ
	UnmapViewOfFile(pSharedData);
	CloseHandle(hMapFile);
	//CloseHandle(hUpdateFlag);

	return 0;
}