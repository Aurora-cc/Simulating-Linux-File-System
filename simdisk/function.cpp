#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <conio.h>
#include "simdisk.h"
using namespace std;

extern SharedData* pSharedData;
extern user userTable[USER_NUM];

//init�����ʼ��
void cmd_init(int index)
{	
	char input_cmd = '\0';
	
	//��ESCȡ����ASCII
	while (strlen(pSharedData->if_continue[index]) == 0)
	{
		
	}
	if (strcmp(pSharedData->if_continue[index], "T\0") == 0)
	{
		virDisk.clear();//���
		
		//���������Ϣ
		virDisk.open("virtual_disk.bin", ios::out | ios::binary);

		//���� 100M �Ŀռ�
		//long lFileSize = BLOCKS_NUM * BLOCK_GROUPS_NUM;

		char buf[4096];
		memset(buf, 0, sizeof(buf));

		for (int i = 0; i < 25600; i++) {
			virDisk.write(buf, sizeof(buf));
		}

		virDisk.seekp(0, virDisk.beg);

		int i;
		for (i = 0; i < BLOCK_GROUPS_NUM; i++)
		{
			dataBlockGroups[i].super_block.totalFreeBlockCount = BLOCKS_NUM - DIR_SIZE;
			dataBlockGroups[i].super_block.totalFreeInodeCount = INODES_NUM - 1;
			dataBlockGroups[i].block_info.blockBmpIndex = i * BLOCKS_EACH;
			dataBlockGroups[i].block_info.inodeBmpIndex = i * INODES_EACH;
			dataBlockGroups[i].block_info.inodeTableIndex = i * INODES_EACH;
			dataBlockGroups[i].block_info.blockAddr = DATA_AREA_ADDR + i * BLOCKS_EACH * BLOCK_SIZE;
			dataBlockGroups[i].block_info.freeBlockCount = BLOCKS_EACH;
			dataBlockGroups[i].block_info.freeInodeCount = INODES_EACH;
		}
		for (i = 0; i < BLOCKS_NUM; i++)
		{
			dataBlockBitmap[i] = NOT_USED;
		}
		for (i = 0; i < INODES_NUM; i++)
		{
			inodeBitmap[i] = NOT_USED;
		}
		for (i = 0; i < INODES_NUM; i++)
		{
			inodeTable[i].privilege = READ_WRITE;
			inodeTable[i].type = TYPE_DIR;
			inodeTable[i].byte = 0;
			inodeTable[i].blockNum = 0;
			inodeTable[i].diskAddr = -1;
		}

		//Ĭ��·��Ϊ��Ŀ¼�����û�����Ϣ��Ĭ�ϣ�
		dataBlockGroups[0].block_info.freeBlockCount = BLOCKS_EACH - DIR_SIZE;
		dataBlockGroups[0].block_info.freeInodeCount = INODES_EACH - 1;
		for (i = 0; i < DIR_SIZE; i++) dataBlockBitmap[i] = USED;
		inodeBitmap[0] = USED;
		inodeTable[0].privilege = PROTECTED;
		inodeTable[0].type = TYPE_DIR;
		inodeTable[0].byte = sizeof(dir);
		inodeTable[0].blockNum = DIR_SIZE;
		inodeTable[0].diskAddr = DATA_AREA_ADDR;

		strcpy(inodeTable[0].name, "root");
		time_t lTime;
		time(&lTime);
		tm tmCreatedTime = *localtime(&lTime);
		inodeTable[0].createdTime.set_date_time(tmCreatedTime);

		dirCurPath.inodeNum = 0;
		dirCurPath.nSize = 2;//��ǰĿ¼+��һ��Ŀ¼
		dirCurPath.nSubInode[0] = 0;
		dirCurPath.nSubInode[1] = 0;
		strcpy(curPath, "root");

		if (!virDisk.is_open()) error(FATAL_WRITE);
		for (i = 0; i < BLOCK_GROUPS_NUM; i++)
		{
			virDisk.write(reinterpret_cast<char*>(&dataBlockGroups[i]), sizeof(blockGroup));
		}
		for (i = 0; i < INODES_NUM; i++)
		{
			virDisk.write(reinterpret_cast<char*>(&inodeBitmap[i]), sizeof(bitmapStatus));
		}
		for (i = 0; i < INODES_NUM; i++)
		{
			virDisk.write(reinterpret_cast<char*>(&inodeTable[i]), sizeof(inode));
		}
		for (i = 0; i < BLOCKS_NUM; i++)
		{
			virDisk.write(reinterpret_cast<char*>(&dataBlockBitmap[i]), sizeof(bitmapStatus));
		}
		virDisk.seekp(inodeTable[dirCurPath.inodeNum].diskAddr, ios::beg);
		virDisk.write(reinterpret_cast<char*>(&dirCurPath), sizeof(dir));

		virDisk.close();

		//���ɹ̶���С�ļ���ɣ�׼��������Ϣ
		//delete[](buf);
		//��ʼ����ɣ�����
		//cout << "��ʼ����ɣ�" << endl;
		wait(5);
		//system("cls");

		cin.sync();//���������
		for (int i = 0; i < USER_NUM; i++)
		{
			cmd_cd(i, "/");
		}

		system("cls");//�����Ļ
	}
	pSharedData->if_continue[index][0] = '\0';
}

//��ʾϵͳ��Ϣ
string cmd_info()
{
	string str = "";

	str.append("ģ��Linux�ļ�ϵͳInfo��\n");
	str.append("\t");
	str.append("����������\t");
	str.append(to_string(BLOCKS_NUM * BLOCK_SIZE));
	str.append(" �ֽ�\t");
	str.append(to_string((float)BLOCKS_NUM / 1024));
	str.append(" MB\n");

	str.append("\t");
	str.append("���ÿռ䣺\t");
	str.append(to_string((BLOCKS_NUM - dataBlockGroups[0].super_block.totalFreeBlockCount) * BLOCK_SIZE));
	str.append(" �ֽ�\t");
	str.append(to_string((float)(BLOCKS_NUM - dataBlockGroups[0].super_block.totalFreeBlockCount) / 1024));
	str.append(" MB\n");

	str.append("\t");
	str.append("���ÿռ䣺\t");
	str.append(to_string(dataBlockGroups[0].super_block.totalFreeBlockCount * BLOCK_SIZE));
	str.append(" �ֽ�\t");
	str.append(to_string((float)dataBlockGroups[0].super_block.totalFreeBlockCount / 1024));
	str.append(" MB\n");

	str.append("\t");
	str.append("���ÿռ������\t");
	str.append(to_string(((float)dataBlockGroups[0].super_block.totalFreeBlockCount / (float)BLOCKS_NUM) * 100));
	str.append("%\n");

	str.append("\t");
	str.append("�̿��С��\t");
	str.append(to_string(BLOCK_SIZE));
	str.append(" �ֽ�\n");

	str.append("\t");
	str.append("ÿ���̿�����\t");
	str.append(to_string(BLOCKS_EACH));
	str.append(" ��\n");

	str.append("\t");
	str.append("ÿ��i�������\t");
	str.append(to_string(INODES_EACH));
	str.append(" ��\n");

	str.append("\t");
	str.append("�̿�������\t");
	str.append(to_string(BLOCKS_NUM));
	str.append(" ��\n");

	str.append("\t");
	str.append("i���������\t");
	str.append(to_string(INODES_NUM));
	str.append(" ��\n");

	str.append("\t");
	str.append("���п�������\t");
	str.append(to_string(dataBlockGroups[0].super_block.totalFreeBlockCount));
	str.append(" ��\n");

	str.append("\t");
	str.append("����i���������\t");
	str.append(to_string(dataBlockGroups[0].super_block.totalFreeInodeCount));
	str.append(" ��\n\n");

	return str;
}

// cd����л�Ŀ¼
string cmd_cd(int index, const char* strPath)
{
	string str = "";
	dir dirTemp;
	size_t nPathLen = strlen(strPath);
	//����·������Ч·�����л�����Ч�򱨴�
	if (analyse_path(strPath, nPathLen, dirTemp))
	{
		if (inodeTable[dirTemp.inodeNum].privilege == WRITE_ONLY)
		{
			str = error(DIR_WRITEONLY, strPath);
		}
		dirCurPath = dirTemp;
		set_cur_path(dirCurPath);
		//strcpy(pSharedData->curDirPath[pSharedData->user_count - 1], curPath);

		int i = 0;
		for (; curPath[i] != '\0'; i++)
		{
			pSharedData->curDirPath[index][i] = curPath[i];
		}
		pSharedData->curDirPath[index][i] = '\0';

		strcpy(pSharedData->buffer2[index], " \0");
		//pSharedData->updated2[index] = true;
	}
	else
	{
		str = error(CD_FAILED);
	}
	return str;
}

//dir���������ʾĿ¼��Ϣ
void cmd_dir(int index, string& str, dir dirTemp, string prefix, const char* strPath)
{

	//cout << endl << str << endl;

	if (strcmp(strPath, "/s\0") == 0)
	{
		for (int i = 0; i < dirTemp.nSize; i++)
		{
			if (i == 0 || i == 1)
				continue;
			tm tmCreatedTime = inodeTable[dirTemp.nSubInode[i]].createdTime.get_date_time();
			str.append(prefix);
			str.append(to_string(tmCreatedTime.tm_year + 1900));
			str.append("/");
			str.append(to_string(tmCreatedTime.tm_mon + 1));
			str.append("/");
			str.append(to_string(tmCreatedTime.tm_mday));
			str.append("/");
			str.append(to_string(tmCreatedTime.tm_hour));
			str.append(":");
			str.append(to_string(tmCreatedTime.tm_min));
			str.append(":");
			str.append(to_string(tmCreatedTime.tm_sec));
			str.append("\t");

			str.append(inodeTable[dirTemp.nSubInode[i]].name);
			str.append("\t\t");

			if (inodeTable[dirTemp.nSubInode[i]].type == TYPE_DIR)
			{
				str.append("Ŀ¼\t\t");
			}
			else
			{
				str.append("�ļ�");
				str.append("\t");
				str.append(to_string(inodeTable[dirTemp.nSubInode[i]].byte));
				str.append("\t");

			}
			switch (inodeTable[dirTemp.nSubInode[i]].privilege)
			{
				case PROTECTED:
					str.append("\tϵͳ");
					//cout << left << "ϵͳ";
					break;
				case READ_WRITE:
					str.append("\t��д");
					//cout << left << "��д";
					break;
				case READ_ONLY:
					str.append("\tֻ��");
					//cout << left << "ֻ��";
					break;
				case WRITE_ONLY:
					str.append("\tֻд");
					//cout << left << "ֻд";
					break;
				case SHARE:
					str.append("\t����");
					//cout << left << "����";
					break;
			}
			str.append("\t\t");

			char hex[100] = {0};
			int n = inodeTable[dirTemp.nSubInode[i]].diskAddr;
			sprintf(hex,  "%09X", n);

			str.append("0x");
			str.append(hex);
			str.append("\n\n");


			if (i<=1 || inodeTable[dirTemp.nSubInode[i]].type == TYPE_FILE)
			{
				continue;
			}
			else
			{
				cmd_cd(index, inodeTable[dirTemp.nSubInode[i]].name);

				if (dirCurPath.nSize == 2)
				{
					//return;
				}
				else
				{
					prefix.append("---");
					str.append("\n");
					str.append(prefix);
					str.append(inodeTable[dirCurPath.nSubInode[0]].name);
					str.append("����Ŀ¼���£�\n");
				}
				
				cmd_dir(index, str, dirCurPath, prefix, strPath);
			}
			cmd_cd(index, "..\0");
			prefix = prefix.substr(0, prefix.length() - 3);
			//str.append("\n");
		}
	}
	else
	{
		for (unsigned int i = 0; i < dirTemp.nSize; i++)
		{
			//cout << "name: " << inodeTable[dirTemp.nSubInode[i]].name << " size: " << dirTemp.nSize << endl;
			
			if (i == 0 || i==1)
				continue;

			tm tmCreatedTime = inodeTable[dirTemp.nSubInode[i]].createdTime.get_date_time();
			str.append(to_string(tmCreatedTime.tm_year + 1900));
			str.append("/");
			str.append(to_string(tmCreatedTime.tm_mon + 1));
			str.append("/");
			str.append(to_string(tmCreatedTime.tm_mday));
			str.append("/");
			str.append(to_string(tmCreatedTime.tm_hour));
			str.append(":");
			str.append(to_string(tmCreatedTime.tm_min));
			str.append(":");
			str.append(to_string(tmCreatedTime.tm_sec));
			str.append("\t");

			str.append(inodeTable[dirTemp.nSubInode[i]].name);
			str.append("\t\t");

			/*cout.fill('0');
			cout.width(4);
			cout << right << tmCreatedTime.tm_year + 1900;
			cout << "/";
			cout.width(2);
			cout << tmCreatedTime.tm_mon + 1;
			cout << "/";
			cout.width(2);
			cout << tmCreatedTime.tm_mday;
			cout << " ";
			cout.width(2);
			cout << tmCreatedTime.tm_hour;
			cout << ":";
			cout.width(2);
			cout << tmCreatedTime.tm_min;
			cout << ":";
			cout.width(2);
			cout << tmCreatedTime.tm_sec;
			cout << " ";
			cout.fill('\0');
			cout.width(20);
			cout << left;*/
			

			if (inodeTable[dirTemp.nSubInode[i]].type == TYPE_DIR)
			{
				//nDirCount++;
				/*cout.width(4);
				cout << "Ŀ¼";
				cout.width(18);
				cout << "";*/

				str.append("Ŀ¼\t\t");
			}
			else
			{
				//nFileCount++;
				/*cout.width(4);
				cout << "�ļ�";
				cout.width(18);
				cout << right << inodeTable[dirTemp.nSubInode[i]].byte;*/

				str.append("�ļ�");
				str.append("\t");
				str.append(to_string(inodeTable[dirTemp.nSubInode[i]].byte));
				str.append("\t");

			}
			switch (inodeTable[dirTemp.nSubInode[i]].privilege)
			{
			case PROTECTED:
				str.append("\tϵͳ");
				//cout << left << "ϵͳ";
				break;
			case READ_WRITE:
				str.append("\t��д");
				//cout << left << "��д";
				break;
			case READ_ONLY:
				str.append("\tֻ��");
				//cout << left << "ֻ��";
				break;
			case WRITE_ONLY:
				str.append("\tֻд");
				//cout << left << "ֻд";
				break;
			case SHARE:
				str.append("\t����");
				//cout << left << "����";
				break;
			}
			
		
		str.append("\t\t");

		char hex[100] = { 0 };
		int n = inodeTable[dirTemp.nSubInode[i]].diskAddr;
		sprintf(hex, "%09X", n);

		str.append("0x");
		str.append(hex);
		str.append("\n");
		//cout << endl;
		str.append("\n");
	}
	}
	
	return;
}

// md���������Ŀ¼
string cmd_md(int index, const char* strPath, fileAttribute privilege)
{
	string str = "";
	dir dirTemp;
	size_t nPathLen = strlen(strPath);
	char strDirName[MAX_NAME_LENGTH];
	if (analyse_path(strPath, nPathLen, dirTemp, strDirName))
	{
		if (inodeTable[dirTemp.inodeNum].privilege == READ_ONLY)
		{
			str = error(WRITE_DIR_ACCESS_DENIED);
			return str;
		}

		if (pSharedData->userTable[index].ACL[2][dirTemp.inodeNum] == false)
		{
			str = error(WRITE_DIR_ACCESS_DENIED);
			return str;
		}

		long lAddr = -1;
		int inodeNum = -1;
		unsigned int nIndex;
		//�Ƿ��Ѵ��ڸ����ֵ�����
		if (dirTemp.have_child(strDirName))
		{
			str = error(FILE_EXIST, inodeTable[dirTemp.inodeNum].name, strDirName);
			return str;
		}
		//����Ŀ¼�Ĵ洢�ռ�
		lAddr = alloc_block(DIR_SIZE, nIndex);
		if (lAddr < 0)	//�ռ䲻��
		{
			str = error(SPACE_NOT_ENOUGH);
			return str;
		}
		else
		{
			//����i���
			inodeNum = alloc_inode();
			if (inodeNum < 0)
			{
				str = error(INODE_ALLOC_FAILED);
				return str;
			}

			//��ʼ����Ŀ¼�����������Ӧ��Ϣ
			dir dirNew;
			dirNew.inodeNum = (unsigned int)inodeNum;
			dirNew.nSize = 2;
			strcpy(inodeTable[dirNew.inodeNum].name, strDirName);
			dirNew.nSubInode[0] = (unsigned int)inodeNum;
			dirNew.nSubInode[1] = dirTemp.inodeNum;
			inodeTable[inodeNum].type = TYPE_DIR;
			inodeTable[inodeNum].privilege = privilege;
			inodeTable[inodeNum].byte = sizeof(dir);
			inodeTable[inodeNum].diskAddr = lAddr;
			inodeTable[inodeNum].blockNum = DIR_SIZE;
			time_t lTime;
			time(&lTime);
			tm tmCreatedTime = *localtime(&lTime);
			inodeTable[inodeNum].createdTime.set_date_time(tmCreatedTime);

			//�ڸ�Ŀ¼�������Ϣ
			dirTemp.nSubInode[dirTemp.nSize] = (unsigned int)inodeNum;
			dirTemp.nSize++;
			if (dirTemp.inodeNum == dirCurPath.inodeNum)
			{
				dirCurPath = dirTemp;
			}
			//����
			virDisk.open("virtual_disk.bin", ios::out | ios::binary | ios::_Nocreate);
			if (!virDisk.is_open()) str = error(FATAL_WRITE);
			int i;
			for (i = 0; i < BLOCK_GROUPS_NUM; i++)
			{
				virDisk.write(reinterpret_cast<char*>(&dataBlockGroups[i]), sizeof(blockGroup));
			}
			virDisk.seekp((sizeof(blockGroup) * BLOCK_GROUPS_NUM + sizeof(bitmapStatus) * inodeNum), ios::beg);
			virDisk.write(reinterpret_cast<char*>(&inodeBitmap[inodeNum]), sizeof(bitmapStatus));
			virDisk.seekp((sizeof(blockGroup) * BLOCK_GROUPS_NUM + sizeof(bitmapStatus) * INODES_NUM
				+ sizeof(inode) * inodeNum), ios::beg);
			virDisk.write(reinterpret_cast<char*>(&inodeTable[inodeNum]), sizeof(inode));
			virDisk.seekp((sizeof(blockGroup) * BLOCK_GROUPS_NUM + sizeof(bitmapStatus) * INODES_NUM
				+ sizeof(inode) * INODES_NUM + sizeof(bitmapStatus) * nIndex), ios::beg);
			for (i = 0; i < DIR_SIZE; i++)
			{
				virDisk.write(reinterpret_cast<char*>(&dataBlockBitmap[nIndex]), sizeof(bitmapStatus));
			}
			virDisk.seekp(lAddr, ios::beg);
			virDisk.write(reinterpret_cast<char*>(&dirNew), sizeof(dir));
			virDisk.seekp(inodeTable[dirTemp.inodeNum].diskAddr, ios::beg);
			virDisk.write(reinterpret_cast<char*>(&dirTemp), sizeof(dir));
			virDisk.close();
		}
	}
	else
	{
		str = error(MD_FAILED);
	}
	return str;
}

// rd���ɾ��Ŀ¼
string cmd_rd(int index, const char* strPath)
{
	//cout << strPath << endl;
	string str = "";
	dir dirTemp;
	size_t nPathLen = strlen(strPath);
	char strRmDirName[MAX_NAME_LENGTH];
	if (analyse_path(strPath, nPathLen, dirTemp, strRmDirName))
	{
		unsigned int i;
		unsigned int nPos = 0, inodeNum;
		//����Ŀ¼
		for (i = 2; i < dirTemp.nSize; i++)
		{
			if (strcmp(inodeTable[dirTemp.nSubInode[i]].name, strRmDirName) == 0 &&
				inodeTable[dirTemp.nSubInode[i]].type == TYPE_DIR)
			{
				nPos = i;
				inodeNum = dirTemp.nSubInode[i];
				break;
			}
		}
		if (i == dirTemp.nSize)	//�Ҳ���Ŀ¼
		{
			strcpy(pSharedData->if_tip[index], "F\0");
			str = error(DIR_NOT_EXIST, strPath);
			return str;
		}
		else	//�ҵ�
		{
			dir dirRemove;
			//��ȡ��Ϣ
			virDisk.open("virtual_disk.bin", ios::in | ios::binary);
			if (!virDisk.is_open()) error(FATAL_READ);
			virDisk.seekg(inodeTable[inodeNum].diskAddr, ios::beg);
			virDisk.read(reinterpret_cast<char*>(&dirRemove), sizeof(dir));
			virDisk.close();

			if (inodeTable[dirRemove.inodeNum].privilege == PROTECTED || inodeTable[dirRemove.inodeNum].privilege == READ_ONLY)
			{
				strcpy(pSharedData->if_tip[index], "F\0");
				str = error(WRITE_DIR_ACCESS_DENIED);
				return str;
			}

			if (pSharedData->userTable[index].ACL[2][dirRemove.inodeNum] == false)
			{
				strcpy(pSharedData->if_tip[index], "F\0");
				str = error(WRITE_DIR_ACCESS_DENIED);
				return str;
			}

			//��ֹɾ����ǰĿ¼
			if (dirRemove.inodeNum == dirCurPath.inodeNum)
			{
				str = error(NO_DEL_CUR);
				return str;
			}

			if (dirRemove.nSize > 2)	//��������
			{
				strcpy(pSharedData->if_tip[index], "T\0");
				while (strlen(pSharedData->if_continue[index]) == 0)
				{

				}
				if (strcmp(pSharedData->if_continue[index], "T\0") == 0)
				{
					dirTemp.remove_dir(dirRemove, nPos);
				}
				pSharedData->if_continue[index][0] = '\0';
			}
			else	//Ŀ¼Ϊ�գ�ֱ��ɾ��
			{
				strcpy(pSharedData->if_tip[index], "F\0");
				//���������Ϣ
				free_block(inodeTable[inodeNum].blockNum, ((inodeTable[inodeNum].diskAddr - DATA_AREA_ADDR) / BLOCK_SIZE));
				free_inode(inodeNum);
				//��Ŀ¼��Ϣ
				for (i = nPos; i < dirTemp.nSize; i++)
					dirTemp.nSubInode[i] = dirTemp.nSubInode[i + 1];
				dirTemp.nSize--;
				if (dirTemp.inodeNum == dirCurPath.inodeNum) dirCurPath = dirTemp;
				//����
				virDisk.open("virtual_disk.bin", ios::out | ios::binary | ios::_Nocreate);
				if (!virDisk.is_open()) error(FATAL_WRITE);
				for (i = 0; i < BLOCK_GROUPS_NUM; i++)
				{
					virDisk.write(reinterpret_cast<char*>(&dataBlockGroups[i]), sizeof(blockGroup));
				}
				virDisk.seekp((sizeof(blockGroup) * BLOCK_GROUPS_NUM + sizeof(bitmapStatus) * inodeNum), ios::beg);
				virDisk.write(reinterpret_cast<char*>(inodeBitmap), sizeof(bitmapStatus));
				virDisk.seekp((sizeof(blockGroup) * BLOCK_GROUPS_NUM + (sizeof(bitmapStatus) + sizeof(inode)) * INODES_NUM
					+ (inodeTable[inodeNum].diskAddr - DATA_AREA_ADDR) / BLOCK_SIZE), ios::beg);
				for (i = 0; i < (int)inodeTable[inodeNum].blockNum; i++)
				{
					virDisk.write(reinterpret_cast<char*>(dataBlockBitmap), sizeof(bitmapStatus));
				}
				virDisk.seekp(inodeTable[dirTemp.inodeNum].diskAddr, ios::beg);
				virDisk.write(reinterpret_cast<char*>(&dirTemp), sizeof(dir));
				virDisk.close();
			}
		}
	}
	else
	{
		strcpy(pSharedData->if_tip[index], "F\0");
		str = error(RD_FAILED);
	}
	return str;
}

//newfile����������ļ�
string cmd_newfile(int index, const char* strPath, fileAttribute privilege)
{
	string str = "";
	dir dirTemp;
	size_t nPathLen = strlen(strPath);
	char strFileName[MAX_NAME_LENGTH];
	if (analyse_path(strPath, nPathLen, dirTemp, strFileName))
	{
		if (inodeTable[dirTemp.inodeNum].privilege == READ_ONLY)
		{
			str = error(WRITE_FILE_ACCESS_DENIED, inodeTable[dirTemp.inodeNum].name);
		}
		if (pSharedData->userTable[index].ACL[2][dirTemp.inodeNum] == false)
		{
			str = error(WRITE_FILE_ACCESS_DENIED);
			return str;
		}
		//Ŀ¼���Ѵ��ڸ�����
		if (dirTemp.have_child(strFileName))
		{
			str = error(FILE_EXIST, inodeTable[dirTemp.inodeNum].name, strFileName);
			return str;
		}
		while (!pSharedData->file[index])
		{

		}

		pSharedData->file[index] = true;

		//�����ļ�
		dirTemp.save_file(strFileName, pSharedData->fileContent[index], strlen(pSharedData->fileContent[index]), privilege);
	}
	else
	{
		str = error(NEW_FILE_FAILED);
		return str;
	}
	return str;
}

//cat�����ʾ�ļ�����
string cmd_cat(int index, const char* strPath)
{
	string str = "";
	dir dirTemp;
	size_t nPathLen = strlen(strPath);
	char strFileName[MAX_NAME_LENGTH];
	if (analyse_path(strPath, nPathLen, dirTemp, strFileName))
	{
		unsigned int i;
		unsigned int nInode;
		//�����ļ�
		for (i = 2; i < dirTemp.nSize; i++)
		{
			if (strcmp(inodeTable[dirTemp.nSubInode[i]].name, strFileName) == 0 &&
				inodeTable[dirTemp.nSubInode[i]].type == TYPE_FILE)
			{
				nInode = dirTemp.nSubInode[i];
				break;
			}
		}
		if (i == dirTemp.nSize)	//�Ҳ����ļ�
		{
			str = error(FILE_NOT_EXIST, inodeTable[dirTemp.inodeNum].name, strFileName);
		}
		else	//�ҵ�
		{
			if (inodeTable[nInode].privilege == WRITE_ONLY)
			{
				str = error(READ_FILE_ACCESS_DENIED);
			}
			if (pSharedData->userTable[index].ACL[1][nInode] == false)
			{
				str = error(READ_FILE_ACCESS_DENIED);
				return str;
			}
			//���ļ�
			char* strBuffer = new char[inodeTable[nInode].byte];
			dirTemp.open_file(nInode, strBuffer);
			//��ʾ�ļ�����
			str.append("�ļ� ");
			str.append(strFileName);
			str.append(" ���������£�\n");
			//cout << "�ļ� " << strFileName << " ���������£�" << endl;
			//cout << strBuffer << endl;
			str.append(strBuffer);
			delete[](strBuffer);
			strBuffer = NULL;
		}
	}
	else
	{
		str = error(CAT_FAILED);
	}
	return str;
}

//copy���ģ������и����ļ�
string cmd_copy(int index, const char* strSrcPath, const char* strDesPath)
{
	string str = "";
	char strFileName[MAX_NAME_LENGTH];
	char* strBuffer;
	dir dirTemp;
	char strDiv;
	long nLen = 0;
	size_t nSrcLen = strlen(strSrcPath);
	size_t nDesLen = strlen(strDesPath);
	//����·���������޸�
	char* strSrcFinalPath = new char[nSrcLen];
	char* strDesFinalPath = new char[nDesLen];
	strcpy(strSrcFinalPath, strSrcPath);
	strcpy(strDesFinalPath, strDesPath);

	if (if_host_path(strSrcFinalPath))
	{
		if (if_host_path(strDesFinalPath))
		{
			str = error(NOT_BOTH_HOST);
			return str;
		}

		fstream fsHostIn;
		fsHostIn.open(strSrcFinalPath, ios::in | ios::binary);
		if (!fsHostIn.is_open())
		{
			str = error(HOST_FILE_NOT_EXIST, strSrcFinalPath);
			return str;
		}
		//�����ļ�����
		fsHostIn.seekg(0, ios::end);
		nLen = fsHostIn.tellg();
		//����洢�ռ�
		strBuffer = new char[nLen];
		strBuffer[nLen - 1] = 0;
		fsHostIn.seekg(0, ios::beg);
		fsHostIn.read(reinterpret_cast<char*>(strBuffer), nLen);
		fsHostIn.close();
		//��ȡ�ļ���
		strDiv = '\\';
		//strcpy(strFileName, strrchr(strSrcFinalPath, strDiv) + 1);
		//����Ŀ��·��
		if (analyse_path(strDesFinalPath, nDesLen, dirTemp, strFileName))
		{
			if (inodeTable[dirTemp.inodeNum].privilege == READ_ONLY)
			{
				str = error(WRITE_FILE_ACCESS_DENIED);
				delete[](strBuffer);
				strBuffer = NULL;
				return str;
			}

			if (pSharedData->userTable[index].ACL[2][dirTemp.inodeNum] == false)
			{
				str = error(WRITE_FILE_ACCESS_DENIED);
				delete[](strBuffer);
				strBuffer = NULL;
				return str;
			}
			//�ж�Ŀ¼�Ƿ��Ѵ���ͬ������
			if (dirTemp.have_child(strFileName))
			{
				delete[](strBuffer);
				strBuffer = NULL;
				str = error(FILE_EXIST, inodeTable[dirTemp.inodeNum].name, strFileName);
				return str;
			}
			//���浽����
			dirTemp.save_file(strFileName, strBuffer, nLen + 1, READ_WRITE);
			delete[](strBuffer);
			strBuffer = NULL;
			str.append("�ļ�������ɣ�\n");
		}
		else
		{
			str = error(COPY_FAILED);
			return str;
		}
	}
	else	//��һ������������ <host>
	{
		if (if_host_path(strDesFinalPath))	//ģ������ļ����Ƶ� host ��
		{
			//�ָ�·�����õ��ļ���
			if (analyse_path(strSrcFinalPath, nSrcLen, dirTemp, strFileName))
			{
				unsigned int nInode;
				unsigned int i;
				for (i = 2; i < dirTemp.nSize; i++)
				{
					if (strcmp(inodeTable[dirTemp.nSubInode[i]].name, strFileName) == 0 &&
						inodeTable[dirTemp.nSubInode[i]].type == TYPE_FILE)
					{
						nInode = dirTemp.nSubInode[i];
						break;
					}
				}
				if (inodeTable[nInode].privilege == WRITE_ONLY)
				{
					str = error(READ_FILE_ACCESS_DENIED);
					return str;
				}
				if (pSharedData->userTable[index].ACL[1][nInode] == false)
				{
					str = error(READ_FILE_ACCESS_DENIED);
					return str;
				}
				if (i == dirTemp.nSize)	//�Ҳ����ļ�
				{
					str = error(FILE_NOT_EXIST, inodeTable[dirTemp.inodeNum].name, strFileName);
					return str;
				}
				else
				{
					//"C:\\Users\\Aurora\\Desktop\\test.txt"
					//��ȡ�ļ����ڴ�
					strBuffer = new char[inodeTable[nInode].byte];
					nLen = dirTemp.open_file(nInode, strBuffer);
					//�ϲ�Ϊ������ȫ·��
					char* strFullPath = new char[nDesLen * 2];
					//strFullPath = strtok(strDesFinalPath, "<host>");
					int i = 0;
					for (; strDesFinalPath[i] != '\0'; i++)
					{
						strFullPath[i] = strDesFinalPath[i];
						//cout << strDesFinalPath[i] << endl;
					}
					strFullPath[i] = '\0';

					fstream fsHostOut;
					fsHostOut.open(strFullPath, ios::out | ios::binary);
					if (!fsHostOut.is_open())
					{
						str = error(HOST_FILE_WRITE_FAILED, strFullPath);
						delete[](strBuffer);
						delete[](strFullPath);
						strBuffer = NULL;
						strFullPath = NULL;
						return str;
					}
					fsHostOut.write(reinterpret_cast<char*>(strBuffer), nLen);
					fsHostOut.close();
					delete[](strFullPath);
					delete[](strBuffer);
					strBuffer = NULL;
					strFullPath = NULL;
					str.append("�ļ�������ɣ�\n");
				}
			}
			else
			{
				str = error(COPY_FAILED);
			}
		}
		else	//ģ��������ļ�����
		{
			//�ָ�·�����õ��ļ���
			if (analyse_path(strSrcFinalPath, nSrcLen, dirTemp, strFileName))
			{
				unsigned int nInode;
				//�����ļ�
				unsigned int i;
				for (i = 2; i < dirTemp.nSize; i++)
				{
					if (strcmp(inodeTable[dirTemp.nSubInode[i]].name, strFileName) == 0 &&
						inodeTable[dirTemp.nSubInode[i]].type == TYPE_FILE)
					{
						nInode = dirTemp.nSubInode[i];
						break;
					}
				}
				if (i == dirTemp.nSize)	//�Ҳ����ļ�
				{
					str = error(FILE_NOT_EXIST, inodeTable[dirTemp.inodeNum].name, strFileName);
				}
				else	//�ҵ�
				{
					if (inodeTable[nInode].privilege == FILE_WRITEONLY)
					{
						str = error(FILE_WRITEONLY, strSrcFinalPath);
						return str;
					}
					fileAttribute privilege = inodeTable[nInode].privilege;
					strBuffer = new char[inodeTable[nInode].byte];
					nLen = dirTemp.open_file(nInode, strBuffer);
					//cout << "strBuffer: " << strBuffer << endl;
					if (analyse_path(strDesFinalPath, nDesLen, dirTemp, strFileName))
					{
						if (inodeTable[dirTemp.inodeNum].privilege == READ_ONLY)
						{
							str = error(DIR_READONLY, strDesFinalPath);
							delete[](strBuffer);
							strBuffer = NULL;
							return str;
						}
						//�ж�Ŀ¼�Ƿ��Ѵ���ͬ������
						if (dirTemp.have_child(strFileName))
						{
							delete[](strBuffer);
							strBuffer = NULL;
							str = error(FILE_EXIST, inodeTable[dirTemp.inodeNum].name, strFileName);
							return str;
						}
						//�����ļ�
						dirTemp.save_file(strFileName, strBuffer, nLen, privilege);
						str.append("�ļ�������ɣ�\n");
					}
					else
					{
						str = error(COPY_FAILED);
					}
					//cout << "strFileName" << strFileName << endl;
					delete[](strBuffer);
					strBuffer = NULL;
				}
			}
			else
			{
				str = error(COPY_FAILED);
			}
		}
	}
	return str;
}

//del���ɾ���ļ�
string cmd_del(int index, const char* strPath)
{
	string str = "";
	dir dirTemp;
	char strFileName[MAX_NAME_LENGTH];
	size_t nPathLen = strlen(strPath);
	if (analyse_path(strPath, nPathLen, dirTemp, strFileName))
	{
		if (inodeTable[dirTemp.inodeNum].privilege == READ_ONLY)
		{
			str = error(WRITE_FILE_ACCESS_DENIED);
			return str;
		}
		if (pSharedData->userTable[index].ACL[2][dirTemp.inodeNum] == false)
		{
			str = error(WRITE_FILE_ACCESS_DENIED);
			return str;
		}
		dirTemp.delete_file(strFileName);
		str.append("�ļ� ");
		str.append(curPath);
		str.append("/");
		str.append(strFileName);
		str.append(" ɾ���ɹ���\n");
	}
	else
	{
		str = error(DEL_FAILED);
	}
	return str;
}

//check�����Ⲣ�ָ��ļ�ϵͳ�����ļ�ϵͳ�е�����һ���Խ��м�⣬���Զ������ļ�ϵͳ�Ľṹ����Ϣ��������������
string cmd_check()
{
	string str = "";
	int i, j;
	int nStart;				//��ʼ��ַ
	bool bException = false;	//�Ƿ����쳣�ı�־
	unsigned int nFreeBlockNum, nFreeInodeNum;			//���п��i���
	unsigned int nFreeBlockAll = 0, nFreeInodeAll = 0;	//���п��п��i���
	cout << "����ļ�ϵͳing����" << endl;
	for (i = 0; i < BLOCK_GROUPS_NUM; i++)
	{
		nFreeBlockNum = 0;
		nFreeInodeNum = 0;
		nStart = i * BLOCKS_EACH;
		//������п�Ϳ���i����ܺ�
		for (j = 0; j < BLOCKS_EACH; j++)
		{
			if (dataBlockBitmap[nStart + j] == NOT_USED) nFreeBlockNum++;
			if (inodeBitmap[nStart + j] == NOT_USED) nFreeInodeNum++;
		}
		//�������ʹ��̼�¼��ͬ���������쳣
		if (dataBlockGroups[i].block_info.freeBlockCount != nFreeBlockNum)
		{
			bException = true;
			dataBlockGroups[i].block_info.freeBlockCount = nFreeBlockNum;
		}
		if (dataBlockGroups[i].block_info.freeInodeCount != nFreeInodeNum)
		{
			bException = true;
			dataBlockGroups[i].block_info.freeInodeCount = nFreeInodeNum;
		}
		//�����ܺ�
		nFreeBlockAll += dataBlockGroups[i].block_info.freeBlockCount;
		nFreeInodeAll += dataBlockGroups[i].block_info.freeInodeCount;
	}
	//�������ʹ��̼�¼��ͬ���������쳣
	if (dataBlockGroups[0].super_block.totalFreeBlockCount != nFreeBlockAll)
	{
		bException = true;
		for (i = 0; i < BLOCKS_EACH; i++)
			dataBlockGroups[0].super_block.totalFreeBlockCount = nFreeBlockAll;
	}
	if (dataBlockGroups[0].super_block.totalFreeInodeCount != nFreeInodeAll)
	{
		bException = true;
		for (i = 0; i < BLOCKS_EACH; i++)
			dataBlockGroups[0].super_block.totalFreeInodeCount = nFreeInodeAll;
	}
	if (!bException)
	{
		str.append("�����ɣ�û�з����ļ�ϵͳ�쳣");
		str.append("\n");
	}
	else	//����Ķ�
	{
		//cout << "��鷢���ļ�ϵͳ�����쳣�������޸��С���" << endl;
		virDisk.open("virtual_disk.bin", ios::out | ios::binary | ios::_Nocreate);
		if (!virDisk.is_open()) error(FATAL_WRITE);
		for (int i = 0; i < BLOCK_GROUPS_NUM; i++)
			virDisk.write(reinterpret_cast<char*>(&dataBlockGroups[i]), sizeof(blockGroup));
		for (int j = 0; j < INODES_NUM; j++)
			virDisk.write(reinterpret_cast<char*>(&inodeBitmap[j]), sizeof(bitmapStatus));
		for (int k = 0; k < INODES_NUM; k++)
			virDisk.write(reinterpret_cast<char*>(&inodeTable[k]), sizeof(blockGroup));
		for (int l = 0; l < BLOCKS_NUM; l++)
			virDisk.write(reinterpret_cast<char*>(&dataBlockBitmap[l]), sizeof(bitmapStatus));
		/*for (int i = 0; i < USER_NUM; i++)
			virDisk.read(reinterpret_cast<char*>(&userTable[i]), sizeof(user));*/
		virDisk.seekp(inodeTable[dirCurPath.inodeNum].diskAddr, ios::beg);
		virDisk.write(reinterpret_cast<char*>(&dirCurPath), sizeof(dir));
		virDisk.close();
		str.append("�����ɣ��ļ�ϵͳ�޸����");
		str.append("\n");
		//cout << "�ļ�ϵͳ�޸����" << endl;
	}
	return str;
}

string cmd_help()
{
	string str = "";
	str.append("\ninit\t");
	str.append("��ʼ��������Ϣ\n");

	str.append("info\t");
	str.append("�鿴������Ϣ\n");

	str.append("cd\t");
	str.append("�л�Ŀ¼\n");

	str.append("dir\t");
	str.append("�鿴Ŀ¼\n");

	str.append("md\t");
	str.append("�½�Ŀ¼\n");

	str.append("rd\t");
	str.append("�Ƴ�Ŀ¼\n");

	str.append("\tnewfile\t");
	str.append("�½��ļ�\n");

	str.append("cat\t");
	str.append("�鿴�ļ�\n");

	str.append("copy\t");
	str.append("�����ļ�\n");

	str.append("del\t");
	str.append("ɾ���ļ�\n");

	str.append("check\t");
	str.append("��Ⲣ�ָ��ļ�ϵͳ\n");

	str.append("exit\t");
	str.append("�˳�ϵͳ\n");

	return str;
}

//�ȴ� dTime ��ĺ��������ڵȴ��û��鿴��Ϣ
void wait(double dTime)
{
	clock_t start;
	clock_t end;
	start = clock(); //��ʼ
	while (1)
	{
		end = clock();
		if (double(end - start) / 1000 >= dTime / 2)
			//�������dTime��
			break;
	}
}

//�ж��ַ��Ƿ����ʮ�����ƹ淶
bool is_hex_num(char chNum)
{
	if ((chNum >= '0' && chNum <= '9') || (chNum >= 'a' && chNum <= 'f') || (chNum >= 'A' && chNum <= 'F'))
		return true;
	else
		return false;
}