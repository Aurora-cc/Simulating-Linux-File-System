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
		str = "出现未知error";
		break;
	case INIT_FAIL:
		str = "您没有初始化权限，初始化失败";
		break;
	case FATAL_READ:
		str = "读取模拟 Linux 文件系统失败";
		wait(2);
		exit(0);
		break;
	case FATAL_WRITE:
		str = "写入模拟 Linux 文件系统失败";
		wait(2);
		exit(0);
		break;
	case DIR_READONLY:
		str.append("error：目录 ");
		str.append(strArg1);
		str.append(" 只读，无法修改");
		break;
	case FILE_READONLY:
		str.append("error：文件 ");
		str.append(strArg1);
		str.append(" 只读，无法删除");
		break;
	case DIR_WRITEONLY:
		str.append("error：目录 ");
		str.append(strArg1);
		str.append(" 只写，无法读取");
		break;
	case FILE_WRITEONLY:
		str.append("error：文件 ");
		str.append(strArg1);
		str.append(" 只写，无法读取");
		break;
	case CD_FAILED:
		str = "切换目录失败";
		break;
	case DIR_FAILED:
		str = "目录显示失败";
		break;
	case MD_FAILED:
		str = "目录创建失败";
		break;
	case RD_FAILED:
		str = "目录删除失败";
		break;
	case NEW_FILE_FAILED:
		str = "文件创建失败";
		break;
	case CAT_FAILED:
		str = "文件打开失败";
		break;
	case DEL_FAILED:
		str = "文件删除失败";
		break;
	case COPY_FAILED:
		str = "文件复制失败";
		break;
	case FILE_EXIST:
		str.append("error：目录 ");
		str.append(strArg1);
		str.append(" 下已存在名为 ");
		str.append(strArg2);
		str.append(" 的文件或目录");
		break;
	case NOT_BOTH_HOST:
		str = "error：复制来源和目标不能均为在宿主机";
		break;
	case HOST_FILE_NOT_EXIST:
		str.append("error：宿主机不存在文件 ");
		str.append(strArg1);
		str.append(" ，文件复制失败");
		break;
	case HOST_FILE_WRITE_FAILED:
		str.append("error：宿主机写入文件");
		str.append(strArg1);
		str.append(" 出错，文件复制失败");
		break;
	case FILE_NOT_EXIST:
		str.append("error：目录 ");
		str.append(strArg1);
		str.append(" 下不存在名为 ");
		str.append(strArg2);
		str.append(" 的文件或目录");
		break;
	case DIR_NOT_EXIST:
		str.append("error：目录 ");
		str.append(strArg1);
		str.append("不存在");
		break;
	case PATH_NOT_FOUND:
		str = "error：找不到指定路径";
		break;
	case NO_DEL_CUR:
		str = "error：不能删除当前目录";
		break;
	case ILLEGAL_FILENAME:
		str.append("error：非法文件名！\n文件名中不能含有字符“/”，长度不能超过 ");
		str.append(to_string(MAX_NAME_LENGTH));
		str.append(" 个字符\n");
		break;
	case SPACE_NOT_ENOUGH:
		str = "error：磁盘空间不足！";
		break;
	case INODE_ALLOC_FAILED:
		str = "error：i节点分配失败！";
		break;
	case INVILID_CMD:
		str.append("error：无效的命令 ");
		str.append(strArg1);
		break;
	case TOO_MANY_ARGS:
		str = "error：参数个数过多";
		break;
	case WRONG_ARGS:
		str = "error：参数错误";
		break;
	case WRONG_COMMANDLINE:
		str = "命令行参数错误";
		break;
	case READ_DIR_ACCESS_DENIED:
		str = "该用户无权限读取该目录";
		break;
	case WRITE_DIR_ACCESS_DENIED:
		str = "该用户无权限创建或修改目录";
		break;
	case EXEC_DIR_ACCESS_DENIED:
		str = "该用户无权限进入该目录";
		break;
	case READ_FILE_ACCESS_DENIED:
		str = "该用户无权限读该文件";
		break;
	case WRITE_FILE_ACCESS_DENIED:
		str = "该用户无权限创建或修改文件";
		break;
	}
	return str;
}