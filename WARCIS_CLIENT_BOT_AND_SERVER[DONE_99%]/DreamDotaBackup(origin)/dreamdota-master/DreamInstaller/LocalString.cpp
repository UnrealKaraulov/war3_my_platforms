#include "stdafx.h"
#include "LocaleString.h"

namespace LocaleString {
	LocaleIdEnum LocaleId;

	struct StringDataType {
		const wchar_t* string[MAX_LOCALE_ID];
	};

	StringDataType StringData[MAX_STRING_ID] = {
		{L"Initialize installer failed", L"��ʼ����װ����ʧ�ܡ�"},
		{L"Select install directory", L"ѡ��װĿ¼"},
		{L"DreamDota Installer %u.%02u.%u.%04u", L"DreamDota ��װ���� %u.%02u.%u.%04u"},
		{L"Error reading resource.", L"��ȡ��Դ����"},
		{L"Unable to write:\n%s", L"�޷�д���ļ�:\n%s"},
		{L"Installation completed, starting DreamDota...", L"��װ��ɣ���������DreamDota..."},
		{L"Installation interupted.", L"��װ�жϡ�"},
		{L"Copying %s...", L"���ڸ��� %s..."},
		{L"Waiting process for exit...", L"���ڵȴ������˳�..."}
	};

	void Init() {
		LocaleId = GetSystemDefaultLCID() == 2052 ? ZH_CN : EN_US;
	}

	void Cleanup() {

	}
}

const wchar_t* _(int id) {
	return LocaleString::StringData[id].string[LocaleString::LocaleId];
}