#include "stdafx.h"
#include "Exception.h"

jmp_buf GLOBAL_ENV;

const wchar_t* ExceptionDescription[MAX_EXCEPTION] = {
	/* 0x00000001 */	L"��ʼ��ʧ�ܣ���������Ƿ�ɱ�������ֹ��",									
	/* 0x00000002 */	L"��ȡ����ԱȨ��ʧ�ܣ��������Ƿ��й���ԱȨ���Լ������Ƿ�ɱ�������ֹ��",								

	NULL
};