#define _CRT_SECURE_NO_WARNINGS

#include "simdisk.h"

//����������з���i��㣬����i����
int alloc_inode()
{
	int nIndex, nInodeIndex = -1;
	int nTemp, i, j;
	//����Ƿ����㹻�Ŀռ����ڷ���
	for (i = 0; i < BLOCK_GROUPS_NUM; i++)
	{
		for (j = 0; j < INODES_EACH; j++)
		{
			nTemp = (int)dataBlockGroups[i].block_info.inodeBmpIndex + j;
			if (inodeBitmap[nTemp] == NOT_USED)
			{
				nIndex = i;
				nInodeIndex = nTemp;
				break;
			}
		}
		if (nInodeIndex != -1)
			break;
	}
	//����ɹ����޸���Ӧ�ĳ����顢���ݿ黹��λͼ״̬��Ϣ
	if (nInodeIndex != -1)
	{
		for (i = 0; i < BLOCK_GROUPS_NUM; i++)
			dataBlockGroups[i].super_block.totalFreeInodeCount -= 1;
		dataBlockGroups[nIndex].block_info.freeInodeCount -= 1;
		inodeBitmap[nInodeIndex] = USED;
	}
	return nInodeIndex;
}

//������������ͷ�i���
void free_inode(unsigned int nInode)
{
	int i;
	//�޸���Ӧ�ĳ����顢���ݿ黹��λͼ״̬��Ϣ
	for (i = 0; i < BLOCK_GROUPS_NUM; i++)
		dataBlockGroups[i].super_block.totalFreeInodeCount += 1;
	dataBlockGroups[nInode / BLOCKS_EACH].block_info.freeInodeCount += 1;
	inodeBitmap[nInode] = NOT_USED;
}

//��������������ݿ�λͼ nIndex ���������ݿ�ռ䣬���ط���õ��Ŀռ���׵�ַ
long alloc_block(unsigned int nLen, unsigned int& nIndex)
{
	//�������ݿ�洢
	long lAddr = -1;
	//�жϴ��̿ռ䲻��
	if (dataBlockGroups[0].super_block.totalFreeBlockCount < nLen) return lAddr;

	int i, j;
	int nCount = 0;	//���������̿���
	int nAvailIndex = 0;		//�������ݿ�λ������
	int nBlockGroupIndex = 0;		//�׸����ݿ����λ������
	bool bBlockGroup[BLOCK_GROUPS_NUM];		//�����������Ϣ��Ҫ�޸�
	int nBlockGroupNum[BLOCK_GROUPS_NUM];	//���˶��ٿ�

	for (i = 0; i < BLOCK_GROUPS_NUM; i++)
	{
		bBlockGroup[i] = false;
		nBlockGroupNum[i] = 0;
	}
	for (i = 0; i < BLOCK_GROUPS_NUM; i++)
	{
		if (nCount == 0)
			if ((int)dataBlockGroups[i].block_info.freeBlockCount < nLen)
				continue;
		//�������ݿ�洢
		for (j = 0; j < BLOCKS_EACH; j++)
		{
			if (dataBlockBitmap[(dataBlockGroups[i].block_info.blockBmpIndex + j)] == NOT_USED)
			{
				nCount++;
				bBlockGroup[i] = true;
				nBlockGroupNum[i]++;
				if (nCount == 1)
				{
					lAddr = dataBlockGroups[i].block_info.blockAddr + j * BLOCK_SIZE;
					nAvailIndex = i * BLOCKS_EACH + j;
					nIndex = nAvailIndex;
					nBlockGroupIndex = i;
				}
			}
			else	//û���㹻���������ݿ�洢
			{
				//��ԭ�����¿�ʼ
				nCount = 0;
				if (j == 0 && (i - 1) >= 0 && bBlockGroup[i - 1])
				{
					bBlockGroup[i - 1] = false;
					nBlockGroupNum[i - 1] = 0;
				}
				bBlockGroup[i] = false;
				nBlockGroupNum[i] = 0;
			}
			if (nCount == nLen) break;
		}
		if (nCount == nLen) break;
	}
	if (nCount != nLen)
	{
		lAddr = -1;
		return lAddr;
	}
	//����ɹ������޸���Ӧ����Ϣ
	for (i = 0; i < BLOCK_GROUPS_NUM; i++)
		dataBlockGroups[i].super_block.totalFreeBlockCount -= nLen;
	j = nAvailIndex + nLen;
	for (i = nAvailIndex; i < j; i++)
		dataBlockBitmap[i] = USED;
	for (i = nBlockGroupIndex; i < BLOCK_GROUPS_NUM; i++)
		if (bBlockGroup[i]) dataBlockGroups[i].block_info.freeBlockCount -= nBlockGroupNum[i];
	return lAddr;
}

//��������������ݿ�λͼ nIndex ���ͷ����ݿ�
void free_block(unsigned int nLen, unsigned int nIndex)
{
	unsigned int i;
	unsigned int nBlockEnd = nIndex + nLen;	//�����β��ַ
	unsigned int nBlockGroup[BLOCK_GROUPS_NUM];	//���ݿ���
	//�����Ӧ����Ϣ
	for (i = 0; i < BLOCK_GROUPS_NUM; i++)
	{
		dataBlockGroups[i].super_block.totalFreeBlockCount += nLen;
		nBlockGroup[i] = 0;
	}
	for (i = nIndex; i < nBlockEnd; i++)
	{
		dataBlockBitmap[i] = NOT_USED;
		nBlockGroup[i / BLOCKS_EACH]++;
	}
	for (i = nIndex / BLOCKS_EACH; i < BLOCKS_EACH; i++)
	{
		if (nBlockGroup[i] != 0)
			dataBlockGroups[i].block_info.freeBlockCount += nBlockGroup[i];
		else
			break;
	}
}