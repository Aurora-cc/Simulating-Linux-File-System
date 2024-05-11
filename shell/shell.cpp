#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include"shell.h"

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

int main()
{
    HANDLE hMapFile;
    SharedData* pSharedData;
    char cmd[MAX_COMMAND_LENGTH];
    char comm[MAX_COMMAND_LENGTH], p1[MAX_PATH_LENGTH], p2[MAX_PATH_LENGTH];
    char userName[MAX_COMMAND_LENGTH];
    char chSelect = '\0';
    user userTable[USER_NUM];

    // 创建共享内存
    hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE,    // 使用无效的句柄值创建一个匿名的共享文件映射对象
        NULL,                    // 默认安全性
        PAGE_READWRITE,          // 可读写权限
        0,                       // 共享内存的大小，0表示与文件大小相同
        sizeof(SharedData),      // 缓冲区的大小
        L"MyShareMemory");       // 共享内存的名称

    if (hMapFile == NULL)
    {
        printf("Could not create file mapping object (%d).\n", GetLastError());
        return 1;
    }

    // 映射共享内存到进程的地址空间
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

    fstream userInfo;
    userInfo.open("C:\\Users\\Aurora\\source\\repos\\simdisk\\user.bin", ios::in | ios::binary);

    for (int i = 0; i < USER_NUM; i++)
    {
        userTable[i].username[0] = '\0';
        userTable[i].uid = -1;
        userTable[i].password = -1;
        for (int j = 0; j < USER_RIGHT_COUNT; j++)
        {
            for (int k = 0; k < 1024; k++)
            {
                userTable[i].ACL[j][k] = true;
            }
        }
    }

    for (int i = 0; i < USER_NUM; i++)
    {
        pSharedData->userTable[i].username[0] = '\0';
        pSharedData->userTable[i].uid = -1;
        pSharedData->userTable[i].password = -1;
        for (int j = 0; j < USER_RIGHT_COUNT; j++)
        {
            for (int k = 0; k < 1024; k++)
            {
                pSharedData->userTable[i].ACL[j][k] = true;
            }
        }
    }

    if (!userInfo.is_open())
    {
        userInfo.clear();
        userInfo.open("C:\\Users\\Aurora\\source\\repos\\simdisk\\user.bin", ios::out |  ios::binary);

        char buf[sizeof(user)];
        memset(buf, 0, sizeof(user));
        for (int i = 0; i < USER_NUM; i++) {
            userInfo.write(buf, sizeof(user));
        }

        userInfo.seekp(0, userInfo.beg);
        

        for (int i = 0; i < USER_NUM; i++)
        {
            userInfo.write(reinterpret_cast<char*>(&userTable[i]), sizeof(user));
        }
    }
    userInfo.close();

    userInfo.open("C:\\Users\\Aurora\\source\\repos\\simdisk\\user.bin", ios::in | ios::binary);
    for (int i = 0; i < USER_NUM; i++)
    {
        userInfo.read(reinterpret_cast<char*>(&userTable[i]), sizeof(user));
    }
    userInfo.close();


    for (int i = 0; i < USER_NUM; i++)
    {
        pSharedData->updated1[i] = false;
        pSharedData->updated2[i] = false;
        pSharedData->file[i] = false;
        pSharedData->if_tip[i][0] = '\0';
        pSharedData->if_continue[i][0] = '\0';
        strcpy(pSharedData->curDirPath[i], "root\0");
    }

    int i = 0;

    cout << "请输入用户名：";
    cin >> userName;

    i = 0;
    for (; i < USER_NUM; i++)
    {
        //cout << userTable[i].username << endl;
        if (strcmp(userName, userTable[i].username) == 0)
        {
            userIndex = i;
            userTable[userIndex].uid = userIndex;
            break;
        }
    }

    if (userIndex != -1)
    {
        L:cout << "请输入密码：" << endl;
        unsigned int password;
        cin >> password;
        if (password != userTable[userIndex].password)
        {
            cout << "登录失败，请重新尝试！" << endl;
            wait(5);
            system("cls");//清空屏幕
            goto L;
        }
        else
        {
            cout << "登录成功！" << endl;
            wait(5);
            system("cls");//清空屏幕
        }
    }
    else
    {
        for (i = 0; i < USER_NUM; i++)
        {
            if (userTable[i].uid == -1)
            {
                userIndex = i;
                break;
            }
        }

        userTable[userIndex].uid = userIndex;

        strcpy(userTable[userIndex].username, userName);
        cout << "该用户为首次登录，请输入密码：" << endl;
        cin >> userTable[userIndex].password;

        cout << endl << "用户 " << userName << " 正在创建中" << endl;

        userInfo.open("C:\\Users\\Aurora\\source\\repos\\simdisk\\user.bin", ios::out | ios::binary);
        for (int i = 0; i < USER_NUM; i++)
        {
            userInfo.write(reinterpret_cast<char*>(&userTable[i]), sizeof(user));
        }
        userInfo.close();

        for (int i = 0; i < USER_NUM; i++)
        {
            strcpy(pSharedData->userTable[i].username, userTable[i].username);
            pSharedData->userTable[i].uid = userTable[i].uid;
            pSharedData->userTable[i].password = userTable[i].password;
            for (int j = 0; j < USER_RIGHT_COUNT; j++)
            {
                for (int k = 0; k < 1024; k++)
                {
                    pSharedData->userTable[i].ACL[j][k] = userTable[i].ACL[j][k];
                }
            }
        }

        userInfo.open("C:\\Users\\Aurora\\source\\repos\\simdisk\\user.bin", ios::in | ios::binary);
        for (int i = 0; i < USER_NUM; i++)
        {
            userInfo.read(reinterpret_cast<char*>(&userTable[i]), sizeof(user));
        }
        userInfo.close();

        wait(5);
        cout << endl << "用户创建成功！" << endl;
        wait(5);
        system("cls");//清空屏幕
    }
    
    fstream virDisk;
    virDisk.open("C:\\Users\\Aurora\\source\\repos\\simdisk\\virtual_disk.bin", ios::in | ios::binary);
    cin.get();
    while (1)
    {
        if (!virDisk.is_open())
        {
            char chSelect = '\0';
            cout << "----------未初始化----------" << endl;
            cout << "尚未初始化模拟 Linux 文件系统，是否进行？(Y/N)" << endl;
            cout << "请选择：";
            while (chSelect != 27)	//用户按ESC取消
            {
                chSelect = _getch();
                if (chSelect == 'y' || chSelect == 'Y' || chSelect == 'n' || chSelect == 'N')
                {
                    cout << chSelect << endl;
                    break;
                }
                else
                {
                    continue;
                }
            }
            if (chSelect == 'y' || chSelect == 'Y')
            {
                //全部删除
                strcpy(pSharedData->if_continue[userIndex], "T\0");
                cout << "正在创建模拟 Linux 文件系统存储空间" << endl;
                wait(5);
                cout << "初始化成功！" << endl;
                system("cls");//清空屏幕
            }
            else
            {
                strcpy(pSharedData->if_continue[userIndex], "F\0");
                cout << "初始化失败，请重新尝试！" << endl;
                wait(5);
                system("cls");//清空屏幕
                continue;
            }
            cout << endl;
            virDisk.open("C:\\Users\\Aurora\\source\\repos\\simdisk\\virtual_disk.bin", ios::in | ios::binary);
        }
        
        
        stringstream ss;

        cout << endl << userName << "@";
        cout << pSharedData->curDirPath[userIndex] << ">";

        //cin >> cmd;
        cin.getline(cmd, MAX_COMMAND_LENGTH);

        ss << cmd;
        ss >> comm >> p1 >> p2;
        ss.clear();

        for (i = 0; cmd[i] != '\0'; i++)
        {
            pSharedData->buffer1[userIndex][i] = cmd[i];
        }
        pSharedData->buffer1[userIndex][i] = '\0';

        pSharedData->updated1[userIndex] = true;

        if (strcmp(comm, "init\0") == 0)
        {
            if (pSharedData->userTable[userIndex].ACL[0][0] == false)
            {
                cout << "您没有初始化权限，初始化失败" << endl;
                continue;
            }
            
            cout << "warning：初始化之后，虚拟磁盘的所有信息将丢失。" << endl;
            cout << "请选择是否坚持初始化？(Y/N)若取消则按ESC" << endl;
            while (chSelect != 27)	//用户按ESC取消
            {
                chSelect = _getch();
                if (chSelect == 'y' || chSelect == 'Y' || chSelect == 'n' || chSelect == 'N')
                {
                    cout.put(chSelect);
                    cout << endl;
                    break;
                }
                else
                {
                    continue;
                }
            }
            if (chSelect == 'y' || chSelect == 'Y')
            {
                //全部删除
                strcpy(pSharedData->if_continue[userIndex], "T\0");
                cout << "正在创建模拟 Linux 文件系统存储空间" << endl;
                wait(5);
                cout << "初始化成功！" << endl;
                system("cls");//清空屏幕
            }
            else
            {
                strcpy(pSharedData->if_continue[userIndex], "F\0");
                cout <<"初始化失败，请重新尝试！" << endl;
                wait(5);
                system("cls");//清空屏幕
                continue;
            }
            cout << endl;
           
        }

        if (strcmp(comm, "rd\0") == 0)
        {
            while (strlen(pSharedData->if_tip[userIndex]) == 0)
            {
                
            }
            if (strcmp(pSharedData->if_tip[userIndex], "T\0") == 0)
            {
                cout << "目录下有子目录或文件，是否全部删除？(Y/N)" << endl;
                while (chSelect != 27)	//用户按ESC取消
                {
                    chSelect = _getch();
                    if (chSelect == 'y' || chSelect == 'Y' || chSelect == 'n' || chSelect == 'N')
                    {
                        cout.put(chSelect);
                        cout << endl;
                        break;
                    }
                    else
                    {
                        continue;
                    }
                }
                if (chSelect == 'y' || chSelect == 'Y')
                {
                    //全部删除
                    strcpy(pSharedData->if_continue[userIndex], "T\0");
                    cout << "目录删除成功！" << endl;
                }
                else
                {
                    strcpy(pSharedData->if_continue[userIndex], "F\0");
                    cout <<"目录删除失败，请重新尝试！" << endl;
                }
            }
            pSharedData->if_tip[userIndex][0] = '\0';
            cout << endl;
        }

        //cin.clear();
        if (strcmp(comm, "newfile\0") == 0)
        {
            char input;		//用户输入的字符
            long nSize = 5;		//默认文件尺寸
            long charNum = 0;
            char* strBuffer = new char[nSize];		//文件内容
            char* strMoreBuffer;					//交换文件数据
            cout << "请输入文件内容，以“$”结尾：" << endl;
            //cin.getline(file, '$');
            for (i = 0; i < nSize; i++)
            {
                strBuffer[i] = 0;
            }

            while ((input = cin.get()) != '$')
            {
                strBuffer[charNum++] = input;
                if (charNum >= nSize - 1)
                {
                    strMoreBuffer = new char[nSize];
                    //保存交换数据
                    strcpy(strMoreBuffer, strBuffer);
                    delete[](strBuffer);
                    strBuffer = NULL;
                    //扩大存储空间
                    nSize = nSize * 2;
                    strBuffer = new char[nSize];
                    for (i = 0; i < nSize; i++) strBuffer[i] = 0;
                    //取回交换数据
                    strcpy(strBuffer, strMoreBuffer);
                    delete[](strMoreBuffer);
                    strMoreBuffer = NULL;
                }
            }
            strcpy(pSharedData->fileContent[userIndex], strBuffer);
 
            pSharedData->file[userIndex] = true;
            cin.get();
        }

        //cout << strlen(pSharedData->buffer2[userIndex]) << endl;

        while (pSharedData->updated2[userIndex] == false)
        {
            
        }

        cout << pSharedData->buffer2[userIndex] << endl;
        pSharedData->buffer2[userIndex][0] = '\0';
        pSharedData->updated2[userIndex] = false;
        
        cmd[0] = '\0';
    }


    // 关闭互斥量句柄
    UnmapViewOfFile(pSharedData);
    CloseHandle(hMapFile);

    return 0;
}