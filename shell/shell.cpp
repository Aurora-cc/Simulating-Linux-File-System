#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include"shell.h"

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

int main()
{
    HANDLE hMapFile;
    SharedData* pSharedData;
    char cmd[MAX_COMMAND_LENGTH];
    char comm[MAX_COMMAND_LENGTH], p1[MAX_PATH_LENGTH], p2[MAX_PATH_LENGTH];
    char userName[MAX_COMMAND_LENGTH];
    char chSelect = '\0';
    user userTable[USER_NUM];

    // ���������ڴ�
    hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE,    // ʹ����Ч�ľ��ֵ����һ�������Ĺ����ļ�ӳ�����
        NULL,                    // Ĭ�ϰ�ȫ��
        PAGE_READWRITE,          // �ɶ�дȨ��
        0,                       // �����ڴ�Ĵ�С��0��ʾ���ļ���С��ͬ
        sizeof(SharedData),      // �������Ĵ�С
        L"MyShareMemory");       // �����ڴ������

    if (hMapFile == NULL)
    {
        printf("Could not create file mapping object (%d).\n", GetLastError());
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

    cout << "�������û�����";
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
        L:cout << "���������룺" << endl;
        unsigned int password;
        cin >> password;
        if (password != userTable[userIndex].password)
        {
            cout << "��¼ʧ�ܣ������³��ԣ�" << endl;
            wait(5);
            system("cls");//�����Ļ
            goto L;
        }
        else
        {
            cout << "��¼�ɹ���" << endl;
            wait(5);
            system("cls");//�����Ļ
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
        cout << "���û�Ϊ�״ε�¼�����������룺" << endl;
        cin >> userTable[userIndex].password;

        cout << endl << "�û� " << userName << " ���ڴ�����" << endl;

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
        cout << endl << "�û������ɹ���" << endl;
        wait(5);
        system("cls");//�����Ļ
    }
    
    fstream virDisk;
    virDisk.open("C:\\Users\\Aurora\\source\\repos\\simdisk\\virtual_disk.bin", ios::in | ios::binary);
    cin.get();
    while (1)
    {
        if (!virDisk.is_open())
        {
            char chSelect = '\0';
            cout << "----------δ��ʼ��----------" << endl;
            cout << "��δ��ʼ��ģ�� Linux �ļ�ϵͳ���Ƿ���У�(Y/N)" << endl;
            cout << "��ѡ��";
            while (chSelect != 27)	//�û���ESCȡ��
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
                //ȫ��ɾ��
                strcpy(pSharedData->if_continue[userIndex], "T\0");
                cout << "���ڴ���ģ�� Linux �ļ�ϵͳ�洢�ռ�" << endl;
                wait(5);
                cout << "��ʼ���ɹ���" << endl;
                system("cls");//�����Ļ
            }
            else
            {
                strcpy(pSharedData->if_continue[userIndex], "F\0");
                cout << "��ʼ��ʧ�ܣ������³��ԣ�" << endl;
                wait(5);
                system("cls");//�����Ļ
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
                cout << "��û�г�ʼ��Ȩ�ޣ���ʼ��ʧ��" << endl;
                continue;
            }
            
            cout << "warning����ʼ��֮��������̵�������Ϣ����ʧ��" << endl;
            cout << "��ѡ���Ƿ��ֳ�ʼ����(Y/N)��ȡ����ESC" << endl;
            while (chSelect != 27)	//�û���ESCȡ��
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
                //ȫ��ɾ��
                strcpy(pSharedData->if_continue[userIndex], "T\0");
                cout << "���ڴ���ģ�� Linux �ļ�ϵͳ�洢�ռ�" << endl;
                wait(5);
                cout << "��ʼ���ɹ���" << endl;
                system("cls");//�����Ļ
            }
            else
            {
                strcpy(pSharedData->if_continue[userIndex], "F\0");
                cout <<"��ʼ��ʧ�ܣ������³��ԣ�" << endl;
                wait(5);
                system("cls");//�����Ļ
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
                cout << "Ŀ¼������Ŀ¼���ļ����Ƿ�ȫ��ɾ����(Y/N)" << endl;
                while (chSelect != 27)	//�û���ESCȡ��
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
                    //ȫ��ɾ��
                    strcpy(pSharedData->if_continue[userIndex], "T\0");
                    cout << "Ŀ¼ɾ���ɹ���" << endl;
                }
                else
                {
                    strcpy(pSharedData->if_continue[userIndex], "F\0");
                    cout <<"Ŀ¼ɾ��ʧ�ܣ������³��ԣ�" << endl;
                }
            }
            pSharedData->if_tip[userIndex][0] = '\0';
            cout << endl;
        }

        //cin.clear();
        if (strcmp(comm, "newfile\0") == 0)
        {
            char input;		//�û�������ַ�
            long nSize = 5;		//Ĭ���ļ��ߴ�
            long charNum = 0;
            char* strBuffer = new char[nSize];		//�ļ�����
            char* strMoreBuffer;					//�����ļ�����
            cout << "�������ļ����ݣ��ԡ�$����β��" << endl;
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
                    //���潻������
                    strcpy(strMoreBuffer, strBuffer);
                    delete[](strBuffer);
                    strBuffer = NULL;
                    //����洢�ռ�
                    nSize = nSize * 2;
                    strBuffer = new char[nSize];
                    for (i = 0; i < nSize; i++) strBuffer[i] = 0;
                    //ȡ�ؽ�������
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


    // �رջ��������
    UnmapViewOfFile(pSharedData);
    CloseHandle(hMapFile);

    return 0;
}