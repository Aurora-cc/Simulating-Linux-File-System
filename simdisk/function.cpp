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

//init命令，初始化
void cmd_init(int index)
{	
	char input_cmd = '\0';
	
	//按ESC取消，ASCII
	while (strlen(pSharedData->if_continue[index]) == 0)
	{
		
	}
	if (strcmp(pSharedData->if_continue[index], "T\0") == 0)
	{
		virDisk.clear();//清空
		
		//保存基本信息
		virDisk.open("virtual_disk.bin", ios::out | ios::binary);

		//分配 100M 的空间
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

		//默认路径为根目录，设置基本信息（默认）
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
		dirCurPath.nSize = 2;//当前目录+上一级目录
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

		//生成固定大小文件完成，准备运行信息
		//delete[](buf);
		//初始化完成，返回
		//cout << "初始化完成！" << endl;
		wait(5);
		//system("cls");

		cin.sync();//清除缓存区
		for (int i = 0; i < USER_NUM; i++)
		{
			cmd_cd(i, "/");
		}

		system("cls");//清空屏幕
	}
	pSharedData->if_continue[index][0] = '\0';
}

//显示系统信息
string cmd_info()
{
	string str = "";

	str.append("模拟Linux文件系统Info：\n");
	str.append("\t");
	str.append("磁盘容量：\t");
	str.append(to_string(BLOCKS_NUM * BLOCK_SIZE));
	str.append(" 字节\t");
	str.append(to_string((float)BLOCKS_NUM / 1024));
	str.append(" MB\n");

	str.append("\t");
	str.append("已用空间：\t");
	str.append(to_string((BLOCKS_NUM - dataBlockGroups[0].super_block.totalFreeBlockCount) * BLOCK_SIZE));
	str.append(" 字节\t");
	str.append(to_string((float)(BLOCKS_NUM - dataBlockGroups[0].super_block.totalFreeBlockCount) / 1024));
	str.append(" MB\n");

	str.append("\t");
	str.append("可用空间：\t");
	str.append(to_string(dataBlockGroups[0].super_block.totalFreeBlockCount * BLOCK_SIZE));
	str.append(" 字节\t");
	str.append(to_string((float)dataBlockGroups[0].super_block.totalFreeBlockCount / 1024));
	str.append(" MB\n");

	str.append("\t");
	str.append("可用空间比例：\t");
	str.append(to_string(((float)dataBlockGroups[0].super_block.totalFreeBlockCount / (float)BLOCKS_NUM) * 100));
	str.append("%\n");

	str.append("\t");
	str.append("盘块大小：\t");
	str.append(to_string(BLOCK_SIZE));
	str.append(" 字节\n");

	str.append("\t");
	str.append("每组盘块数：\t");
	str.append(to_string(BLOCKS_EACH));
	str.append(" 块\n");

	str.append("\t");
	str.append("每组i结点数：\t");
	str.append(to_string(INODES_EACH));
	str.append(" 个\n");

	str.append("\t");
	str.append("盘块总数：\t");
	str.append(to_string(BLOCKS_NUM));
	str.append(" 块\n");

	str.append("\t");
	str.append("i结点总数：\t");
	str.append(to_string(INODES_NUM));
	str.append(" 个\n");

	str.append("\t");
	str.append("空闲块总数：\t");
	str.append(to_string(dataBlockGroups[0].super_block.totalFreeBlockCount));
	str.append(" 块\n");

	str.append("\t");
	str.append("空闲i结点总数：\t");
	str.append(to_string(dataBlockGroups[0].super_block.totalFreeInodeCount));
	str.append(" 个\n\n");

	return str;
}

// cd命令，切换目录
string cmd_cd(int index, const char* strPath)
{
	string str = "";
	dir dirTemp;
	size_t nPathLen = strlen(strPath);
	//分析路径，有效路径则切换，无效则报错
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

//dir命令，用于显示目录信息
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
				str.append("目录\t\t");
			}
			else
			{
				str.append("文件");
				str.append("\t");
				str.append(to_string(inodeTable[dirTemp.nSubInode[i]].byte));
				str.append("\t");

			}
			switch (inodeTable[dirTemp.nSubInode[i]].privilege)
			{
				case PROTECTED:
					str.append("\t系统");
					//cout << left << "系统";
					break;
				case READ_WRITE:
					str.append("\t读写");
					//cout << left << "读写";
					break;
				case READ_ONLY:
					str.append("\t只读");
					//cout << left << "只读";
					break;
				case WRITE_ONLY:
					str.append("\t只写");
					//cout << left << "只写";
					break;
				case SHARE:
					str.append("\t共享");
					//cout << left << "共享";
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
					str.append("的子目录如下：\n");
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
				cout << "目录";
				cout.width(18);
				cout << "";*/

				str.append("目录\t\t");
			}
			else
			{
				//nFileCount++;
				/*cout.width(4);
				cout << "文件";
				cout.width(18);
				cout << right << inodeTable[dirTemp.nSubInode[i]].byte;*/

				str.append("文件");
				str.append("\t");
				str.append(to_string(inodeTable[dirTemp.nSubInode[i]].byte));
				str.append("\t");

			}
			switch (inodeTable[dirTemp.nSubInode[i]].privilege)
			{
			case PROTECTED:
				str.append("\t系统");
				//cout << left << "系统";
				break;
			case READ_WRITE:
				str.append("\t读写");
				//cout << left << "读写";
				break;
			case READ_ONLY:
				str.append("\t只读");
				//cout << left << "只读";
				break;
			case WRITE_ONLY:
				str.append("\t只写");
				//cout << left << "只写";
				break;
			case SHARE:
				str.append("\t共享");
				//cout << left << "共享";
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

// md命令，创建新目录
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
		//是否已存在该名字的子项
		if (dirTemp.have_child(strDirName))
		{
			str = error(FILE_EXIST, inodeTable[dirTemp.inodeNum].name, strDirName);
			return str;
		}
		//分配目录的存储空间
		lAddr = alloc_block(DIR_SIZE, nIndex);
		if (lAddr < 0)	//空间不足
		{
			str = error(SPACE_NOT_ENOUGH);
			return str;
		}
		else
		{
			//分配i结点
			inodeNum = alloc_inode();
			if (inodeNum < 0)
			{
				str = error(INODE_ALLOC_FAILED);
				return str;
			}

			//开始创建目录，添加设置相应信息
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

			//在父目录中添加信息
			dirTemp.nSubInode[dirTemp.nSize] = (unsigned int)inodeNum;
			dirTemp.nSize++;
			if (dirTemp.inodeNum == dirCurPath.inodeNum)
			{
				dirCurPath = dirTemp;
			}
			//保存
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

// rd命令，删除目录
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
		//查找目录
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
		if (i == dirTemp.nSize)	//找不到目录
		{
			strcpy(pSharedData->if_tip[index], "F\0");
			str = error(DIR_NOT_EXIST, strPath);
			return str;
		}
		else	//找到
		{
			dir dirRemove;
			//读取信息
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

			//禁止删除当前目录
			if (dirRemove.inodeNum == dirCurPath.inodeNum)
			{
				str = error(NO_DEL_CUR);
				return str;
			}

			if (dirRemove.nSize > 2)	//存在子项
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
			else	//目录为空，直接删除
			{
				strcpy(pSharedData->if_tip[index], "F\0");
				//清除磁盘信息
				free_block(inodeTable[inodeNum].blockNum, ((inodeTable[inodeNum].diskAddr - DATA_AREA_ADDR) / BLOCK_SIZE));
				free_inode(inodeNum);
				//父目录信息
				for (i = nPos; i < dirTemp.nSize; i++)
					dirTemp.nSubInode[i] = dirTemp.nSubInode[i + 1];
				dirTemp.nSize--;
				if (dirTemp.inodeNum == dirCurPath.inodeNum) dirCurPath = dirTemp;
				//保存
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

//newfile命令，创建新文件
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
		//目录中已存在该子项
		if (dirTemp.have_child(strFileName))
		{
			str = error(FILE_EXIST, inodeTable[dirTemp.inodeNum].name, strFileName);
			return str;
		}
		while (!pSharedData->file[index])
		{

		}

		pSharedData->file[index] = true;

		//保存文件
		dirTemp.save_file(strFileName, pSharedData->fileContent[index], strlen(pSharedData->fileContent[index]), privilege);
	}
	else
	{
		str = error(NEW_FILE_FAILED);
		return str;
	}
	return str;
}

//cat命令，显示文件内容
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
		//查找文件
		for (i = 2; i < dirTemp.nSize; i++)
		{
			if (strcmp(inodeTable[dirTemp.nSubInode[i]].name, strFileName) == 0 &&
				inodeTable[dirTemp.nSubInode[i]].type == TYPE_FILE)
			{
				nInode = dirTemp.nSubInode[i];
				break;
			}
		}
		if (i == dirTemp.nSize)	//找不到文件
		{
			str = error(FILE_NOT_EXIST, inodeTable[dirTemp.inodeNum].name, strFileName);
		}
		else	//找到
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
			//打开文件
			char* strBuffer = new char[inodeTable[nInode].byte];
			dirTemp.open_file(nInode, strBuffer);
			//显示文件内容
			str.append("文件 ");
			str.append(strFileName);
			str.append(" 的内容如下：\n");
			//cout << "文件 " << strFileName << " 的内容如下：" << endl;
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

//copy命令，模拟磁盘中复制文件
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
	//复制路径，用于修改
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
		//计算文件长度
		fsHostIn.seekg(0, ios::end);
		nLen = fsHostIn.tellg();
		//分配存储空间
		strBuffer = new char[nLen];
		strBuffer[nLen - 1] = 0;
		fsHostIn.seekg(0, ios::beg);
		fsHostIn.read(reinterpret_cast<char*>(strBuffer), nLen);
		fsHostIn.close();
		//提取文件名
		strDiv = '\\';
		//strcpy(strFileName, strrchr(strSrcFinalPath, strDiv) + 1);
		//分析目标路径
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
			//判断目录是否已存在同名子项
			if (dirTemp.have_child(strFileName))
			{
				delete[](strBuffer);
				strBuffer = NULL;
				str = error(FILE_EXIST, inodeTable[dirTemp.inodeNum].name, strFileName);
				return str;
			}
			//保存到磁盘
			dirTemp.save_file(strFileName, strBuffer, nLen + 1, READ_WRITE);
			delete[](strBuffer);
			strBuffer = NULL;
			str.append("文件复制完成！\n");
		}
		else
		{
			str = error(COPY_FAILED);
			return str;
		}
	}
	else	//第一个参数不带有 <host>
	{
		if (if_host_path(strDesFinalPath))	//模拟磁盘文件复制到 host 中
		{
			//分割路径，得到文件名
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
				if (i == dirTemp.nSize)	//找不到文件
				{
					str = error(FILE_NOT_EXIST, inodeTable[dirTemp.inodeNum].name, strFileName);
					return str;
				}
				else
				{
					//"C:\\Users\\Aurora\\Desktop\\test.txt"
					//读取文件到内存
					strBuffer = new char[inodeTable[nInode].byte];
					nLen = dirTemp.open_file(nInode, strBuffer);
					//合并为宿主机全路径
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
					str.append("文件复制完成！\n");
				}
			}
			else
			{
				str = error(COPY_FAILED);
			}
		}
		else	//模拟磁盘中文件复制
		{
			//分割路径，得到文件名
			if (analyse_path(strSrcFinalPath, nSrcLen, dirTemp, strFileName))
			{
				unsigned int nInode;
				//查找文件
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
				if (i == dirTemp.nSize)	//找不到文件
				{
					str = error(FILE_NOT_EXIST, inodeTable[dirTemp.inodeNum].name, strFileName);
				}
				else	//找到
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
						//判断目录是否已存在同名子项
						if (dirTemp.have_child(strFileName))
						{
							delete[](strBuffer);
							strBuffer = NULL;
							str = error(FILE_EXIST, inodeTable[dirTemp.inodeNum].name, strFileName);
							return str;
						}
						//保存文件
						dirTemp.save_file(strFileName, strBuffer, nLen, privilege);
						str.append("文件复制完成！\n");
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

//del命令，删除文件
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
		str.append("文件 ");
		str.append(curPath);
		str.append("/");
		str.append(strFileName);
		str.append(" 删除成功！\n");
	}
	else
	{
		str = error(DEL_FAILED);
	}
	return str;
}

//check命令，检测并恢复文件系统，对文件系统中的数据一致性进行检测，并自动根据文件系统的结构和信息进行数据再整理
string cmd_check()
{
	string str = "";
	int i, j;
	int nStart;				//起始地址
	bool bException = false;	//是否有异常的标志
	unsigned int nFreeBlockNum, nFreeInodeNum;			//空闲块和i结点
	unsigned int nFreeBlockAll = 0, nFreeInodeAll = 0;	//所有空闲块和i结点
	cout << "检查文件系统ing……" << endl;
	for (i = 0; i < BLOCK_GROUPS_NUM; i++)
	{
		nFreeBlockNum = 0;
		nFreeInodeNum = 0;
		nStart = i * BLOCKS_EACH;
		//计算空闲块和空闲i结点总和
		for (j = 0; j < BLOCKS_EACH; j++)
		{
			if (dataBlockBitmap[nStart + j] == NOT_USED) nFreeBlockNum++;
			if (inodeBitmap[nStart + j] == NOT_USED) nFreeInodeNum++;
		}
		//计算结果和磁盘记录不同，则发生了异常
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
		//加入总和
		nFreeBlockAll += dataBlockGroups[i].block_info.freeBlockCount;
		nFreeInodeAll += dataBlockGroups[i].block_info.freeInodeCount;
	}
	//计算结果和磁盘记录不同，则发生了异常
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
		str.append("检查完成，没有发现文件系统异常");
		str.append("\n");
	}
	else	//保存改动
	{
		//cout << "检查发现文件系统出现异常，正在修复中……" << endl;
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
		str.append("检查完成，文件系统修复完成");
		str.append("\n");
		//cout << "文件系统修复完成" << endl;
	}
	return str;
}

string cmd_help()
{
	string str = "";
	str.append("\ninit\t");
	str.append("初始化磁盘信息\n");

	str.append("info\t");
	str.append("查看磁盘信息\n");

	str.append("cd\t");
	str.append("切换目录\n");

	str.append("dir\t");
	str.append("查看目录\n");

	str.append("md\t");
	str.append("新建目录\n");

	str.append("rd\t");
	str.append("移除目录\n");

	str.append("\tnewfile\t");
	str.append("新建文件\n");

	str.append("cat\t");
	str.append("查看文件\n");

	str.append("copy\t");
	str.append("复制文件\n");

	str.append("del\t");
	str.append("删除文件\n");

	str.append("check\t");
	str.append("检测并恢复文件系统\n");

	str.append("exit\t");
	str.append("退出系统\n");

	return str;
}

//等待 dTime 秒的函数，用于等待用户查看信息
void wait(double dTime)
{
	clock_t start;
	clock_t end;
	start = clock(); //开始
	while (1)
	{
		end = clock();
		if (double(end - start) / 1000 >= dTime / 2)
			//如果超过dTime秒
			break;
	}
}

//判断字符是否符合十六进制规范
bool is_hex_num(char chNum)
{
	if ((chNum >= '0' && chNum <= '9') || (chNum >= 'a' && chNum <= 'f') || (chNum >= 'A' && chNum <= 'F'))
		return true;
	else
		return false;
}