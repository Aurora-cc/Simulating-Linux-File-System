#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <ctime>
#include <conio.h>
#include <string>
#include "simdisk.h"
using namespace std;

//dir类的成员函数定义

//打开文件，从虚拟磁盘读取内容到strBuffer中
long dir::open_file(unsigned int nInode, char* strBuffer)
{
	//读取指定地址
	virDisk.open("virtual_disk.bin", ios::in | ios::binary);
	if (!virDisk.is_open()) error(FATAL_READ);
	virDisk.seekg(inodeTable[nInode].diskAddr, ios::beg);
	virDisk.read(reinterpret_cast<char*>(strBuffer), inodeTable[nInode].byte);
	virDisk.close();
	strBuffer[inodeTable[nInode].byte - 1] = '\0';
	return inodeTable[nInode].byte;
}

//保存文件，将strBuffer内容写入虚拟磁盘
void dir::save_file(const char* strFileName, char* strBuffer, unsigned long lFileLen, fileAttribute privilege)
{
	long lAddr = -1;
	int nInode = -1;
	unsigned int nIndex;
	unsigned int nBlockNum;
	//奇偶处理
	if ((lFileLen + 1) % BLOCK_SIZE == 0)
		nBlockNum = (lFileLen + 1) / BLOCK_SIZE;
	else
		nBlockNum = (lFileLen + 1) / BLOCK_SIZE + 1;
	//分配数据块和i-结点
	lAddr = alloc_block(nBlockNum, nIndex);
	if (lAddr < 0)
	{
		error(SPACE_NOT_ENOUGH);
		return;
	}
	nInode = alloc_inode();
	if (nInode < 0)
	{
		error(INODE_ALLOC_FAILED);
		return;
	}
	//开始创建文件，添加设置相应信息
	inodeTable[nInode].type = TYPE_FILE;
	inodeTable[nInode].privilege = privilege;
	inodeTable[nInode].byte = lFileLen + 1;
	inodeTable[nInode].diskAddr = lAddr;
	inodeTable[nInode].blockNum = nBlockNum;
	strcpy(inodeTable[nInode].name, strFileName);
	time_t lTime;
	time(&lTime);
	tm tmCreatedTime = *localtime(&lTime);
	inodeTable[nInode].createdTime.set_date_time(tmCreatedTime);
	//在父目录中添加信息
	this->nSubInode[this->nSize] = (unsigned int)nInode;
	this->nSize++;
	if (this->inodeNum == dirCurPath.inodeNum) dirCurPath = *this;
	//保存
	unsigned int i;
	virDisk.open("virtual_disk.bin", ios::out | ios::binary | ios::_Nocreate);
	if (!virDisk.is_open()) error(FATAL_WRITE);
	for (i = 0; i < BLOCK_GROUPS_NUM; i++)
		virDisk.write(reinterpret_cast<char*>(&dataBlockGroups[i]), sizeof(blockGroup));
	virDisk.seekp((sizeof(blockGroup) * BLOCK_GROUPS_NUM + sizeof(bitmapStatus) * nInode), ios::beg);
	virDisk.write(reinterpret_cast<char*>(&inodeBitmap[nInode]), sizeof(bitmapStatus));
	virDisk.seekp((sizeof(blockGroup) * BLOCK_GROUPS_NUM + sizeof(bitmapStatus) * INODES_NUM
		+ sizeof(inode) * nInode), ios::beg);
	virDisk.write(reinterpret_cast<char*>(&inodeTable[nInode]), sizeof(inode));
	virDisk.seekp((sizeof(blockGroup) * BLOCK_GROUPS_NUM + sizeof(bitmapStatus) * INODES_NUM
		+ sizeof(inode) * INODES_NUM + sizeof(bitmapStatus) * nIndex), ios::beg);
	for (i = 0; i < nBlockNum; i++)
		virDisk.write(reinterpret_cast<char*>(&dataBlockBitmap[nIndex]), sizeof(bitmapStatus));
	virDisk.seekp(lAddr, ios::beg);
	virDisk.write(reinterpret_cast<char*>(strBuffer), lFileLen);
	virDisk.seekp(inodeTable[this->inodeNum].diskAddr, ios::beg);
	virDisk.write(reinterpret_cast<char*>(this), sizeof(dir));
	virDisk.close();
}

//删除文件清除相关信息
void dir::delete_file(const char* strFileName)
{
	unsigned int i;
	unsigned int nInode;
	unsigned int nPos;
	//查找文件
	for (i = 2; i < this->nSize; i++)
	{
		if (strcmp(inodeTable[nSubInode[i]].name, strFileName) == 0 &&
			inodeTable[nSubInode[i]].type == TYPE_FILE)
		{
			nInode = this->nSubInode[i];
			nPos = i;
			break;
		}
	}
	//找不到
	if (i == this->nSize)
	{
		error(FILE_NOT_EXIST, inodeTable[this->inodeNum].name, strFileName);
		return;
	}
	if (inodeTable[nInode].privilege == READ_ONLY)
	{
		error(FILE_READONLY, strFileName);
		return;
	}
	//删除数据块、i-结点和目录信息
	free_block(inodeTable[nInode].blockNum, ((inodeTable[nInode].diskAddr - DATA_AREA_ADDR) / BLOCK_SIZE));
	free_inode(nInode);
	for (i = nPos; i < nSize; i++)
		this->nSubInode[i] = this->nSubInode[i + 1];
	this->nSize--;
	if (this->inodeNum == dirCurPath.inodeNum)
		dirCurPath = *this;
	//保存
	virDisk.open("virtual_disk.bin", ios::out | ios::binary | ios::_Nocreate);
	if (!virDisk.is_open()) error(FATAL_WRITE);
	for (i = 0; i < BLOCK_GROUPS_NUM; i++)
		virDisk.write(reinterpret_cast<char*>(&dataBlockGroups[i]), sizeof(blockGroup));
	virDisk.seekp((sizeof(blockGroup) * BLOCK_GROUPS_NUM + sizeof(bitmapStatus) * nInode), ios::beg);
	virDisk.write(reinterpret_cast<char*>(inodeBitmap), sizeof(bitmapStatus));
	virDisk.seekp((sizeof(blockGroup) * BLOCK_GROUPS_NUM + (sizeof(bitmapStatus) + sizeof(inode)) * INODES_NUM
		+ (inodeTable[nInode].diskAddr - DATA_AREA_ADDR) / BLOCK_SIZE), ios::beg);
	for (i = 0; i < (int)inodeTable[nInode].blockNum; i++)
		virDisk.write(reinterpret_cast<char*>(dataBlockBitmap), sizeof(bitmapStatus));
	virDisk.seekp(inodeTable[this->inodeNum].diskAddr, ios::beg);
	virDisk.write(reinterpret_cast<char*>(this), sizeof(dir));
	virDisk.close();
}

// 删除目录，递归删除其下所有子文件和子目录
void dir::remove_dir(dir dirRemove, unsigned int nIndex)
{
	unsigned int i;		//循环控制变量
	for (i = 2; i < dirRemove.nSize; i++)
	{
		if (inodeTable[dirRemove.nSubInode[i]].type == TYPE_DIR)	//目录，需遍历子文件和子目录
		{
			dir dirSub;
			virDisk.open("virtual_disk.bin", ios::in | ios::binary);
			if (!virDisk.is_open()) error(FATAL_READ);
			virDisk.seekg(inodeTable[dirRemove.nSubInode[i]].diskAddr, ios::beg);
			virDisk.read(reinterpret_cast<char*>(&dirSub), sizeof(dir));
			virDisk.close();
			//删除子文件和子目录
			dirRemove.remove_dir(dirSub, i);
		}
		else	//文件，直接删除
		{
			dirRemove.delete_file(inodeTable[dirRemove.nSubInode[i]].name);
		}
	}
	//删除数据块、i-结点和目录信息
	free_block(inodeTable[dirRemove.inodeNum].blockNum,
		((inodeTable[dirRemove.inodeNum].diskAddr - DATA_AREA_ADDR) / BLOCK_SIZE));
	free_inode(dirRemove.inodeNum);
	for (i = nIndex; i < this->nSize; i++)
		this->nSubInode[i] = this->nSubInode[i + 1];
	this->nSize--;
	if (this->inodeNum == dirCurPath.inodeNum)
		dirCurPath = *this;
	//保存
	virDisk.open("virtual_disk.bin", ios::out | ios::binary | ios::_Nocreate);
	if (!virDisk.is_open()) error(FATAL_WRITE);
	for (i = 0; i < BLOCK_GROUPS_NUM; i++)
		virDisk.write(reinterpret_cast<char*>(&dataBlockGroups[i]), sizeof(blockGroup));
	virDisk.seekp((sizeof(blockGroup) * BLOCK_GROUPS_NUM + dirRemove.inodeNum), ios::beg);
	virDisk.write(reinterpret_cast<char*>(inodeBitmap), sizeof(bitmapStatus));
	virDisk.seekp((sizeof(blockGroup) * BLOCK_GROUPS_NUM + (sizeof(bitmapStatus) + sizeof(inode)) * INODES_NUM
		+ (inodeTable[dirRemove.inodeNum].diskAddr - DATA_AREA_ADDR) / BLOCK_SIZE), ios::beg);
	for (i = 0; i < (int)inodeTable[dirRemove.inodeNum].blockNum; i++)
		virDisk.write(reinterpret_cast<char*>(dataBlockBitmap), sizeof(bitmapStatus));
	virDisk.seekp(inodeTable[this->inodeNum].diskAddr, ios::beg);
	virDisk.write(reinterpret_cast<char*>(this), sizeof(dir));
	virDisk.close();
}

//判断文件夹下是否存在同名文件目录
bool dir::have_child(const char* strDirName)
{
	for (unsigned int i = 2; i < nSize; i++)
	{
		if (strcmp(inodeTable[this->nSubInode[i]].name, strDirName) == 0)
			return true;
	}
	return false;
}