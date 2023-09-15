#define CPPHTTPLIB_THREAD_POOL_COUNT 100

#include "httplib.h"
#include <string>
#include <vector>
#include <iostream>
#include <StormLib.h>
#include <fstream>
#include <streambuf>
#include <filesystem>
#include "BlpReadWrite.h"
#include "crc32.h"
#include "sqlite3.h"
#include "sqlite_orm.h"
#include "WTSW3Iparser.h"
#include "utils.h"
#include <algorithm>
#include <thread>
std::string UploadFormString;
std::string UploadFormString_rus;

using namespace sqlite_orm;

struct MapStruct
{
	int id;
	bool IsMemHack;
	bool ContainPatches;
	int Downloads;
	int Rating;
	unsigned int crc32;
	std::string Path;
	std::string Name;
	std::string Category;
	std::string Author;
	std::string Description;
	std::string Players;
	std::string UpdateCode;
};

auto storage = make_storage("db.sqlite",
	make_table("maps",
		make_column("id", &MapStruct::id, autoincrement(), primary_key()),
		make_column("ismemhack", &MapStruct::IsMemHack),
		make_column("withpatches", &MapStruct::ContainPatches),
		make_column("downloads", &MapStruct::Downloads),
		make_column("rating", &MapStruct::Rating),
		make_column("hashsum", &MapStruct::crc32),
		make_column("path", &MapStruct::Path),
		make_column("name", &MapStruct::Name),
		make_column("category", &MapStruct::Category),
		make_column("author", &MapStruct::Author),
		make_column("description", &MapStruct::Description),
		make_column("players", &MapStruct::Players),
		make_column("updatecode", &MapStruct::UpdateCode)));


void LoadUploadForm()
{
	std::ifstream t("uploadform.txt");
	if (t.is_open())
	{
		t.seekg(0, std::ios::end);
		UploadFormString.reserve(t.tellg());
		t.seekg(0, std::ios::beg);

		UploadFormString.assign((std::istreambuf_iterator<char>(t)), // ( ( . ) )
			std::istreambuf_iterator<char>());
	}
	else UploadFormString = "ANAL ERROR 3";

	t = std::ifstream("uploadform_rus.txt");
	if (t.is_open())
	{
		t.seekg(0, std::ios::end);
		UploadFormString_rus.reserve(t.tellg());
		t.seekg(0, std::ios::beg);

		UploadFormString_rus.assign((std::istreambuf_iterator<char>(t)), // ( ( . ) )
			std::istreambuf_iterator<char>());
	}
	else UploadFormString_rus = "ANAL ERROR RUS 3";
}

std::string MapStoragePath;

void LoadMapStoragePath()
{
	std::ifstream t("mapstoragepath.txt");
	if (t.is_open())
	{
		t.seekg(0, std::ios::end);
		MapStoragePath.reserve(t.tellg());
		t.seekg(0, std::ios::beg);

		MapStoragePath.assign((std::istreambuf_iterator<char>(t)), // ( ( . ) )
			std::istreambuf_iterator<char>());
	}
	else MapStoragePath = "./";
}


std::string StaticStoragePath;

void LoadStaticStoragePath()
{
	std::ifstream t("staticstoragepath.txt");
	if (t.is_open())
	{
		t.seekg(0, std::ios::end);
		StaticStoragePath.reserve(t.tellg());
		t.seekg(0, std::ios::beg);

		StaticStoragePath.assign((std::istreambuf_iterator<char>(t)), // ( ( . ) )
			std::istreambuf_iterator<char>());
	}
	else StaticStoragePath = "./";
}


std::string MemHackPassword;

void LoadMemHackPassword()
{
	std::ifstream t("memhackpassword.txt");
	if (t.is_open())
	{
		t.seekg(0, std::ios::end);
		MemHackPassword.reserve(t.tellg());
		t.seekg(0, std::ios::beg);

		MemHackPassword.assign((std::istreambuf_iterator<char>(t)), // ( ( . ) )
			std::istreambuf_iterator<char>());
	}
	else MemHackPassword = "MemHackPassword";
}



std::string TestHtmlPage;

void LoadTestHtmlPage()
{
	std::ifstream t("testhtmlpage.txt");
	if (t.is_open())
	{
		t.seekg(0, std::ios::end);
		TestHtmlPage.reserve(t.tellg());
		t.seekg(0, std::ios::beg);

		TestHtmlPage.assign((std::istreambuf_iterator<char>(t)), // ( ( . ) )
			std::istreambuf_iterator<char>());
	}
	else TestHtmlPage = "testhtmlpage";
}



std::string SingleMapPage;

void LoadSingleMapPage()
{
	std::ifstream t("viewsinglemappage.txt");
	if (t.is_open())
	{
		t.seekg(0, std::ios::end);
		SingleMapPage.reserve(t.tellg());
		t.seekg(0, std::ios::beg);

		SingleMapPage.assign((std::istreambuf_iterator<char>(t)), // ( ( . ) )
			std::istreambuf_iterator<char>());
	}
	else SingleMapPage = "testhtmlpage";
}



std::string MultipleMapPage;

void LoadMultipleMapPage()
{
	std::ifstream t("viewmultimappage.txt");
	if (t.is_open())
	{
		t.seekg(0, std::ios::end);
		MultipleMapPage.reserve(t.tellg());
		t.seekg(0, std::ios::beg);

		MultipleMapPage.assign((std::istreambuf_iterator<char>(t)), // ( ( . ) )
			std::istreambuf_iterator<char>());
	}
	else MultipleMapPage = "testhtmlpage";
}

std::string SingleMapItem;

void LoadSingleMapItem()
{
	std::ifstream t("singlemapitem.txt");
	if (t.is_open())
	{
		t.seekg(0, std::ios::end);
		SingleMapItem.reserve(t.tellg());
		t.seekg(0, std::ios::beg);

		SingleMapItem.assign((std::istreambuf_iterator<char>(t)), // ( ( . ) )
			std::istreambuf_iterator<char>());
	}
	else SingleMapItem = "testhtmlpage";
}



std::string GlobalMapDeletePassword;

void LoadGlobalMapDeletePassword()
{
	std::ifstream t("globaldeletepassword.txt");
	if (t.is_open())
	{
		t.seekg(0, std::ios::end);
		GlobalMapDeletePassword.reserve(t.tellg());
		t.seekg(0, std::ios::beg);

		GlobalMapDeletePassword.assign((std::istreambuf_iterator<char>(t)), // ( ( . ) )
			std::istreambuf_iterator<char>());
	}
	else GlobalMapDeletePassword = "testhtmlpage";
}

std::string ServerPath;

void LoadServerPath()
{
	std::ifstream t("serverpath.txt");
	if (t.is_open())
	{
		t.seekg(0, std::ios::end);
		ServerPath.reserve(t.tellg());
		t.seekg(0, std::ios::beg);

		ServerPath.assign((std::istreambuf_iterator<char>(t)), // ( ( . ) )
			std::istreambuf_iterator<char>());
	}
	else ServerPath = "qervqvq341435v134513451435";
}



using namespace httplib;



bool ProcessImages(const std::string& mapfilename, HANDLE hMpq, Response& res)
{
	//war3mapMap.tga war3mapMap.blp
	//war3mapPreview.tga war3mapPreview.blp

	HANDLE hFile;
	bool bret = true;
	// ƒостать заставки из карты

	if (SFileOpenFileEx(hMpq, "war3mapMap.tga", 0, &hFile))
	{
		StormBuffer tmpStormBuffer1;
		DWORD dwBytesRead = SFileGetFileSize(hFile, NULL);
		if (dwBytesRead && dwBytesRead < 0x40000000)
		{
			char* Buffer = new char[dwBytesRead];
			SFileReadFile(hFile, Buffer, dwBytesRead, &dwBytesRead, NULL);
			StormBuffer tmpStormBufferInput = StormBuffer(Buffer, dwBytesRead);
			int Width = 256;
			int Height = 256;
			int Bpp = 4;
			if (TGA2Raw(tmpStormBufferInput, tmpStormBuffer1, Width, Height, Bpp, "1"))
			{
				if (RAW2Png(tmpStormBuffer1, tmpStormBufferInput, Width, Height, Bpp, "1"))
				{
					FILE* f;
					fopen_s(&f, (StaticStoragePath + mapfilename + ".map.png").c_str(), "wb");
					if (f)
					{
						fwrite(tmpStormBufferInput.buf, 1, tmpStormBufferInput.length, f);
						fclose(f);
					}

					tmpStormBufferInput.Clear();
				}
			}
			tmpStormBuffer1.Clear();
			delete[] Buffer;
			Buffer = NULL;
		}
		else
		{
			res.set_content("FILE FORMAT ANAL ERROR #44", "text/plain");
		}
		SFileCloseFile(hFile);
	}
	else if (SFileOpenFileEx(hMpq, "war3mapMap.blp", 0, &hFile))
	{
		StormBuffer tmpStormBuffer1;
		DWORD dwBytesRead = SFileGetFileSize(hFile, NULL);
		if (dwBytesRead && dwBytesRead < 0x40000000)
		{
			char* Buffer = new char[dwBytesRead];
			SFileReadFile(hFile, Buffer, dwBytesRead, &dwBytesRead, NULL);
			StormBuffer tmpStormBufferInput = StormBuffer(Buffer, dwBytesRead);
			int Width = 256;
			int Height = 256;
			int Bpp = 4;
			int a1, a2, a3, a4;
			if (Blp2Raw(tmpStormBufferInput, tmpStormBuffer1, Width, Height, Bpp, a1, a2, a3, a4, "1"))
			{
				if (RAW2Png(tmpStormBuffer1, tmpStormBufferInput, Width, Height, Bpp, "1"))
				{
					FILE* f;
					fopen_s(&f, (StaticStoragePath + mapfilename + ".map.png").c_str(), "wb");
					if (f)
					{
						fwrite(tmpStormBufferInput.buf, 1, tmpStormBufferInput.length, f);
						fclose(f);
					}
					tmpStormBufferInput.Clear();
				}
			}
			tmpStormBuffer1.Clear();
			delete[] Buffer;
			Buffer = NULL;
		}
		else
		{
			res.set_content("FILE FORMAT ANAL ERROR #444", "text/plain");
		}
		SFileCloseFile(hFile);
	}

	if (SFileOpenFileEx(hMpq, "war3mapPreview.tga", 0, &hFile))
	{
		StormBuffer tmpStormBuffer1;
		DWORD dwBytesRead = SFileGetFileSize(hFile, NULL);
		if (dwBytesRead && dwBytesRead < 0x40000000)
		{
			char* Buffer = new char[dwBytesRead];
			SFileReadFile(hFile, Buffer, dwBytesRead, &dwBytesRead, NULL);
			StormBuffer tmpStormBufferInput = StormBuffer(Buffer, dwBytesRead);
			int Width = 256;
			int Height = 256;
			int Bpp = 4;
			if (TGA2Raw(tmpStormBufferInput, tmpStormBuffer1, Width, Height, Bpp, "1"))
			{
				if (RAW2Png(tmpStormBuffer1, tmpStormBufferInput, Width, Height, Bpp, "1"))
				{
					FILE* f;
					fopen_s(&f, (StaticStoragePath + mapfilename + ".preview.png").c_str(), "wb");
					if (f)
					{
						fwrite(tmpStormBufferInput.buf, 1, tmpStormBufferInput.length, f);
						fclose(f);
					}
					tmpStormBufferInput.Clear();
				}
			}
			tmpStormBuffer1.Clear();
			delete[] Buffer;
			Buffer = NULL;
		}
		else
		{
			res.set_content("FILE FORMAT ANAL ERROR #3444", "text/plain");
		}
		SFileCloseFile(hFile);
	}
	else if (SFileOpenFileEx(hMpq, "war3mapPreview.blp", 0, &hFile))
	{
		StormBuffer tmpStormBuffer1;
		DWORD dwBytesRead = SFileGetFileSize(hFile, NULL);
		if (dwBytesRead && dwBytesRead < 0x40000000)
		{
			char* Buffer = new char[dwBytesRead];
			SFileReadFile(hFile, Buffer, dwBytesRead, &dwBytesRead, NULL);
			StormBuffer tmpStormBufferInput = StormBuffer(Buffer, dwBytesRead);
			int Width = 256;
			int Height = 256;
			int Bpp = 4;
			int a1, a2, a3, a4;
			if (Blp2Raw(tmpStormBufferInput, tmpStormBuffer1, Width, Height, Bpp, a1, a2, a3, a4, "1"))
			{
				if (RAW2Png(tmpStormBuffer1, tmpStormBufferInput, Width, Height, Bpp, "1"))
				{
					FILE* f;
					fopen_s(&f, (StaticStoragePath + mapfilename + ".preview.png").c_str(), "wb");
					if (f)
					{
						fwrite(tmpStormBufferInput.buf, 1, tmpStormBufferInput.length, f);
						fclose(f);
					}
					tmpStormBufferInput.Clear();
				}
			}
			tmpStormBuffer1.Clear();
			delete[] Buffer;
			Buffer = NULL;
		}
		else
		{
			res.set_content("FILE FORMAT ANAL ERROR #4444", "text/plain");
		}
		SFileCloseFile(hFile);
	}


	return true;
}


bool ProcessW3I(const std::string& mapfilename, HANDLE hMpq, Response& res, WTSW3IParser& tmpWTSParser)
{
	HANDLE hFile;
	bool bret = false;
	// ƒостать им€ карты и параметры
	if (SFileOpenFileEx(hMpq, "war3map.w3i", 0, &hFile))
	{
		DWORD dwBytesRead = SFileGetFileSize(hFile, NULL);
		if (dwBytesRead && dwBytesRead < 0x40000000)
		{
			char* Buffer = new char[dwBytesRead];
			SFileReadFile(hFile, Buffer, dwBytesRead, &dwBytesRead, NULL);

			tmpWTSParser.readW3I(Buffer, dwBytesRead);
			bret = ProcessImages(mapfilename, hMpq, res);

			delete[] Buffer;
			Buffer = NULL;
		}
		else
		{
			res.set_content("FILE FORMAT ANAL ERROR #4", "text/plain");
		}
		SFileCloseFile(hFile);
	}
	else
	{
		res.set_content("FILE FORMAT ANAL ERROR #10", "text/plain");
	}
	return bret;
}

bool ProcessScriptAndWTS(const std::string& mapfilename, HANDLE hMpq, Response& res, WTSW3IParser& tmpWTSParser)
{
	bool bret = false;
	HANDLE hFile;
	// ƒостать им€ карты и параметры
	if (SFileOpenFileEx(hMpq, "war3map.wts", 0, &hFile))
	{
		DWORD dwBytesRead = SFileGetFileSize(hFile, NULL);
		if (dwBytesRead && dwBytesRead < 0x40000000)
		{
			char* Buffer = new char[dwBytesRead];
			SFileReadFile(hFile, Buffer, dwBytesRead, &dwBytesRead, NULL);
			if (strnlen_s(Buffer, dwBytesRead) + 1 >= dwBytesRead)
			{
				tmpWTSParser.readWTS(Buffer);
				bret = ProcessW3I(mapfilename, hMpq, res, tmpWTSParser);
			}
			else
			{
				char outdata[128];
				sprintf_s(outdata, "FILE FORMAT ANAL ERROR #11: %u and %u", strnlen_s(Buffer, dwBytesRead) + 1, dwBytesRead);
				res.set_content(outdata, "text/plain");
			}

			delete[] Buffer;
			Buffer = NULL;
		}
		else
		{
			res.set_content("FILE FORMAT ANAL ERROR #4", "text/plain");
		}
		SFileCloseFile(hFile);
	}
	else
	{
		bret = ProcessW3I(mapfilename, hMpq, res, tmpWTSParser);
	}

	return bret;
}

bool SearchMemhack(const char* str2)
{
	char* str = (char*)str2;
	while (str && str[0] != '\0')
	{
		str = strstr(str, "native");
		if (!str)
		{
			break;
		}

		str += strlen("native");
		while (str[0] == '\t' || str[0] == ' ')
		{
			str++;
		}

		if (str[0] == '\0')
			break;

		if (str == strstr(str, "GetUnitCount"))
		{
			return true;
		}

		if (str == strstr(str, "AttackMoveXY"))
		{
			return true;
		}

		if (str == strstr(str, "GetTownUnitCount"))
		{
			return true;
		}

		if (str == strstr(str, "ConvertUnits"))
		{
			return true;
		}

		if (str == strstr(str, "MergeUnits"))
		{
			return true;
		}
		str++;
	}
	return false;
}

bool SearchCrc32(unsigned int crc32, std::string& mapname, std::string& mapfilename)
{
	//auto pMapStruct = storage.get_pointer<MapStruct>(where(c(&MapStruct::crc32) == crc32));
	//if (pMapStruct != nullptr)
	//{
	//	mapname = pMapStruct->Name;
	//	mapfilename = pMapStruct->Path;
	//	return true;
	//}
	auto pMapStruct = storage.get_all<MapStruct>(where(c(&MapStruct::crc32) == crc32));
	for (auto& map : pMapStruct) {
		mapname = map.Name;
		mapfilename = map.Path;
		return true;
	}
	return false;
}


bool MapNeedUpdateOrRemove(const std::string& mappath, const std::string& mapfilename, const std::string& password)
{
	if (!filesystem::exists(mappath))
	{
		return true;
	}
	if (password.size() == 0)
	{
		return false;
	}
	if (mappath.size() == 0)
	{
		return false;
	}
	//auto pMapStruct = storage.get_pointer<MapStruct>(where(c(&MapStruct::crc32) == crc32));
	//if (pMapStruct != nullptr)
	//{
	//	mapname = pMapStruct->Name;
	//	mapfilename = pMapStruct->Path;
	//	return true;
	//}
	auto pMapStruct = storage.get_all<MapStruct>(
		where(c(&MapStruct::Path) == mapfilename and c(&MapStruct::UpdateCode) == password)
		);

	for (auto& map : pMapStruct) {
		storage.remove<MapStruct>(map.id);
		storage.sync_schema();
		return true;
	}
	return false;
}


bool NeedRemoveMap(int id, std::string& mapfilename, const std::string& password)
{
	if (password.size() == 0)
	{
		return false;
	}
	//auto pMapStruct = storage.get_pointer<MapStruct>(where(c(&MapStruct::crc32) == crc32));
	//if (pMapStruct != nullptr)
	//{
	//	mapname = pMapStruct->Name;
	//	mapfilename = pMapStruct->Path;
	//	return true;
	//}

	if (GlobalMapDeletePassword == password)
	{
		auto pMapStruct = storage.get_all<MapStruct>(
			where(c(&MapStruct::id) == id)
			);

		for (auto& map : pMapStruct) {
			mapfilename = map.Path;
			storage.remove<MapStruct>(map.id);
			storage.sync_schema();
			return true;
		}
	}
	else
	{
		auto pMapStruct = storage.get_all<MapStruct>(
			where(c(&MapStruct::id) == id and c(&MapStruct::UpdateCode) == password)
			);

		for (auto& map : pMapStruct) {
			mapfilename = map.Path;
			storage.remove<MapStruct>(map.id);
			storage.sync_schema();
			return true;
		}
	}
	return false;
}


bool UploadMapFileComplete(const std::string& maparchivepath, const std::string& mapfilename, bool allowmemhack, Response& res,
	unsigned int& crc32, WTSW3IParser& tmpWTSParser)
{
	HANDLE hMpq = NULL;
	HANDLE hFile = NULL;
	char* Buffer = NULL;
	bool retval = false;

	if (SFileOpenArchive(maparchivepath.c_str(), 0, MPQ_OPEN_FORCE_MPQ_V1, &hMpq))
	{
		if (SFileOpenFileEx(hMpq, "war3map.j", 0, &hFile) || SFileOpenFileEx(hMpq, "scripts\\war3map.j", 0, &hFile))
		{
			DWORD dwBytesRead = SFileGetFileSize(hFile, NULL);
			if (dwBytesRead && dwBytesRead < 0x40000000)
			{
				Buffer = new char[dwBytesRead];
				SFileReadFile(hFile, Buffer, dwBytesRead, &dwBytesRead, NULL);
				if (strnlen_s(Buffer, dwBytesRead) + 1 >= dwBytesRead)
				{
					crc32 = crc32_16bytes_prefetch(Buffer, dwBytesRead);
					std::string uploadedmapname = "unknown";
					std::string uploadedmapfilename = "unknown";
					if (SearchMemhack(Buffer) && !allowmemhack)
					{
						res.set_content("MEMHACK FOUND! BAD PASSWORD FOR UPLOAD THIS MAP", "text/plain");
					}
					else if (SearchCrc32(crc32, uploadedmapname, uploadedmapfilename))
					{
						res.set_content("MAP ALREADY UPLOADED WITH PATH:" + uploadedmapfilename + " AND NAME:" + uploadedmapname, "text/plain");
					}
					else
					{
						retval = ProcessScriptAndWTS(mapfilename, hMpq, res, tmpWTSParser);
					}
				}
				else
				{
					char outdata[128];
					sprintf_s(outdata, "FILE FORMAT ANAL ERROR #5: %u and %u", strnlen_s(Buffer, dwBytesRead) + 1, dwBytesRead);

					res.set_content(outdata, "text/plain");
				}

				delete[] Buffer;
				Buffer = NULL;
			}
			else
			{
				res.set_content("FILE FORMAT ANAL ERROR #4", "text/plain");
			}
			SFileCloseFile(hFile);
		}
		else
		{
			res.set_content("FILE FORMAT ANAL ERROR #3", "text/plain");
		}
		SFileCloseArchive(hMpq);
	}
	else
	{
		res.set_content("FILE FORMAT ANAL ERROR #2", "text/plain");
	}

	return retval;
}

void TestImageReadWrite()
{
	FILE* f;
	fopen_s(&f, "test.blp", "rb");
	if (f)
	{
		std::cout << "open" << std::endl;
		fseek(f, 0, SEEK_END);                                      // переместить внутренний указатель в конец файла
		long size = ftell(f);                                       // вернуть текущее положение внутреннего указател€
		fseek(f, 0, SEEK_SET);
		std::cout << "open2" << std::endl;
		StormBuffer tmpbuf = StormBuffer(size);
		fread(tmpbuf.buf, 1, size, f);
		StormBuffer resbuf = StormBuffer();
		int width, height, bpp, mip, alpha, comp, pic;
		std::cout << "open3" << std::endl;
		Blp2Raw(tmpbuf, resbuf, width, height, bpp, mip, alpha, comp, pic, "asdf");
		fclose(f);
		f = NULL;
		fopen_s(&f, "test2.raw", "wb");
		if (f)
		{
			fwrite(resbuf.buf, 1, resbuf.length, f);
			fclose(f);
		}
		tmpbuf.Clear();
		RAW2Png(resbuf, tmpbuf, width, height, 4, "asdf");
		f = NULL;
		fopen_s(&f, "test3.png", "wb");
		if (f)
		{
			fwrite(tmpbuf.buf, 1, tmpbuf.length, f);
			fclose(f);
		}
		tmpbuf.Clear();

		RAW2Jpg(resbuf, tmpbuf, width, height, 4, "asdf");
		f = NULL;
		fopen_s(&f, "test4.jpg", "wb");
		if (f)
		{
			fwrite(tmpbuf.buf, 1, tmpbuf.length, f);
			fclose(f);
		}
	}
	system("pause");
	return;
}



bool IsOkayCategory(std::string str)
{
	if (str == "RPG (Roleplay)") return true;
	if (str == "Strategy") return true;
	if (str == "AoS (MOBA)") return true;
	if (str == "Castle Defense") return true;
	if (str == "Hero Defense") return true;
	if (str == "Hero Arena") return true;
	if (str == "Tag") return true;
	if (str == "Tower Defense") return true;
	if (str == "Tower Wars") return true;
	if (str == "Mini-Game/Sport") return true;
	if (str == "Survival") return true;
	if (str == "Anime") return true;
	if (str == "Offense") return true;
	if (str == "Escape") return true;
	if (str == "Melee") return true;
	if (str == "Other/misc") return true;
	return false;
}

bool FindMapFileNameByID(std::string ids, std::string& mapfilename)
{
	for (auto& c : ids)
	{
		if (!isdigit(c) && c != '\0')
		{
			return false;
		}
	}

	int id = std::stoi(ids);

	auto pMapStruct = storage.get_all<MapStruct>(where(c(&MapStruct::id) == id));
	for (auto& map : pMapStruct) {
		mapfilename = map.Path;
		return true;
	}
	return false;
}

void StaticFilesThread()
{
	LoadStaticStoragePath();
	if (StaticStoragePath.size() < 2)
	{
		std::cout << "ANAL ERROR MapStoragePath BAD!";
		std::terminate();
	}
	Server httpStaticImagesServer;
	auto ret = httpStaticImagesServer.set_mount_point("/", StaticStoragePath.c_str());
	if (!ret) {
		std::cout << "Static Server Anal Error. Bad directory path" << std::endl;
		std::terminate();
	}
	httpStaticImagesServer.listen("0.0.0.0", 81);
	std::cout << "Static Server Anal Error." << std::endl;
	std::terminate();
}

std::string BuildMapViewPage(int mapid)
{
	std::string retval = SingleMapPage;

	auto pMapStruct = storage.get_all<MapStruct>(where(c(&MapStruct::id) == mapid));
	for (auto& map : pMapStruct)
	{
		findAndReplaceAll(retval, "[INSERTMAPID]", std::to_string(mapid));
		findAndReplaceAll(retval, "[INSERTMAPNAME]", map.Name);
		findAndReplaceAll(retval, "[INSERTMAPAUTHOR]", map.Author);
		findAndReplaceAll(retval, "[INSERTDESCRIPTION]", map.Description);
		findAndReplaceAll(retval, "[INSERTDOWNLOADS]", std::to_string(map.Downloads));
		findAndReplaceAll(retval, "[INSERTRATING]", std::to_string(map.Rating));
		if (filesystem::exists(StaticStoragePath + map.Path + ".preview.png"))
			findAndReplaceAll(retval, "[INSERTPREVIEW]", "http://" + ServerPath + ":81/" + map.Path + ".preview.png");
		else
			findAndReplaceAll(retval, "[INSERTPREVIEW]", "http://" + ServerPath + ":81/nopreview.png");

		if (filesystem::exists(StaticStoragePath + map.Path + ".map.png"))
			findAndReplaceAll(retval, "[INSERTMINIMAP]", "http://" + ServerPath + ":81/" + map.Path + ".map.png");
		else
			findAndReplaceAll(retval, "[INSERTPREVIEW]", "http://" + ServerPath + ":81/nominimap.png");

		return retval;
	}

	return "<h1>ANAL VIEW MAP ERROR</h1>";
}

std::string BuildMapList(const std::string& startfrom)
{
	std::string retval = MultipleMapPage;
	std::string tmpMapList = std::string();

	auto pMapStruct = storage.get_all<MapStruct>();
	for (auto& map : pMapStruct) {
		std::string tmpMapItem = SingleMapItem;
		findAndReplaceAll(tmpMapItem, "[INSERTMAPID]", std::to_string(map.id));
		findAndReplaceAll(tmpMapItem, "[INSERTMAPNAME]", map.Name);
		findAndReplaceAll(tmpMapItem, "[INSERTMAPAUTHOR]", map.Author);
		findAndReplaceAll(tmpMapItem, "[INSERTDESCRIPTION]", map.Description);
		findAndReplaceAll(tmpMapItem, "[INSERTDOWNLOADS]", std::to_string(map.Downloads));
		findAndReplaceAll(tmpMapItem, "[INSERTRATING]", std::to_string(map.Rating));
		if (filesystem::exists(StaticStoragePath + map.Path + ".preview.png"))
			findAndReplaceAll(tmpMapItem, "[INSERTPREVIEW]", "http://" + ServerPath + ":81/" + map.Path + ".preview.png");
		else
			findAndReplaceAll(tmpMapItem, "[INSERTPREVIEW]", "http://" + ServerPath + ":81/nopreview.png");

		if (filesystem::exists(StaticStoragePath + map.Path + ".map.png"))
			findAndReplaceAll(tmpMapItem, "[INSERTMINIMAP]", "http://" + ServerPath + ":81/" + map.Path + ".map.png");
		else
			findAndReplaceAll(tmpMapItem, "[INSERTPREVIEW]", "http://" + ServerPath + ":81/nominimap.png");
		tmpMapList += tmpMapItem;
	}


	findAndReplaceAll(retval, "[INSERTMAPLISTITEMHERE]", tmpMapList);
	return retval;
}

std::thread StaticThread = std::thread(StaticFilesThread);



//TestImageReadWrite(); return 323;
int main()
{
	storage.sync_schema();
	LoadUploadForm();
	LoadMapStoragePath();
	LoadStaticStoragePath();
	LoadMemHackPassword();
	LoadTestHtmlPage();
	LoadSingleMapPage();
	LoadMultipleMapPage();
	LoadSingleMapItem();
	LoadGlobalMapDeletePassword();
	LoadServerPath();

	if (MapStoragePath.size() < 2)
	{
		std::cout << "ANAL ERROR MapStoragePath BAD!";
		return -1;
	}

	if (StaticStoragePath.size() < 2)
	{
		std::cout << "ANAL ERROR StaticStoragePath BAD!";
		return -1;
	}

	if (UploadFormString.size() < 10)
	{
		std::cout << "ANAL ERROR UploadFormString BAD!";
		return -1;
	}
	if (UploadFormString_rus.size() < 10)
	{
		std::cout << "ANAL ERROR UploadFormString_rus BAD!";
		return -1;
	}

	if (GlobalMapDeletePassword.size() < 5)
	{
		std::cout << "ANAL ERROR GlobalMapDeletePassword very easy!";
		return -1;
	}

	if (MemHackPassword.size() < 5)
	{
		std::cout << "ANAL ERROR MemHackPassword very easy!";
		return -1;
	}

	if (StaticStoragePath[StaticStoragePath.size() - 1] != '/' &&
		StaticStoragePath[StaticStoragePath.size() - 1] != '\\')
	{
		if (strstr(StaticStoragePath.c_str(), "/"))
		{
			StaticStoragePath += "/";
		}
		else
		{
			StaticStoragePath += "\\";
		}
	}

	if (MapStoragePath[MapStoragePath.size() - 1] != '/' &&
		MapStoragePath[MapStoragePath.size() - 1] != '\\')
	{
		if (strstr(MapStoragePath.c_str(), "/"))
		{
			MapStoragePath += "/";
		}
		else
		{
			MapStoragePath += "\\";
		}
	}

	Server httpMapUploaderSever;

	httpMapUploaderSever.Get("/", [ ](const Request& req, Response& res) {
		res.set_content(UploadFormString, "text/html");
	});

	httpMapUploaderSever.Get("/rus", [ ](const Request& req, Response& res) {
		res.set_content(UploadFormString_rus, "text/html");
	});

	httpMapUploaderSever.Get("/maplist", [ ](const Request& req, Response& res) {
		std::string tmp;
		res.set_content(BuildMapList(tmp), "text/html");
	});

	httpMapUploaderSever.Get("/test", [ ](const Request& req, Response& res) {
		LoadTestHtmlPage();
		res.set_content(TestHtmlPage, "text/html");
	});

	httpMapUploaderSever.Get("/viewmap", [ ](const Request& req, Response& res) {
		if (req.has_param("id")) {
			auto val = req.get_param_value("id");
			for (auto& c : val)
			{
				if (!isdigit(c) && c != '\0')
				{
					res.set_content("MAP ID ANAL ERROR 2!" + val, "text/plain");
					return;
				}
			}
			res.set_content(BuildMapViewPage(std::stoi(val)), "text/html");
		}
		else
		{
			res.set_content("MAP ID ANAL ERROR!", "text/plain");
		}
	});

	httpMapUploaderSever.Get("/delete", [ ](const Request& req, Response& res) {
		if (req.has_param("id")) {
			auto val = req.get_param_value("id");
			if (req.has_param("password")) {
				auto val2 = req.get_param_value("password");
				for (auto& c : val)
				{
					if (!isdigit(c) && c != '\0')
					{
						res.set_content("MAP ID ANAL ERROR 2!", "text/plain");
						return;
					}
				}
				int id = std::stoi(val);
				std::string mapfilename;
				if (NeedRemoveMap(id, mapfilename, val2))
				{
					std::filesystem::remove((MapStoragePath + mapfilename));
					std::filesystem::remove((MapStoragePath + mapfilename) + ".ini");
					std::filesystem::remove((StaticStoragePath + mapfilename) + ".map.png");
					std::filesystem::remove((StaticStoragePath + mapfilename) + ".preview.png");
					res.set_content("Map has been deleted!", "text/html");
				}
				else
				{
					res.set_content("MAP PASSWORD ANAL ERROR 3!", "text/plain");
				}
			}
			else
			{

				res.set_content("MAP PASSWORD ANAL ERROR!", "text/plain");
			}
		}
		else
		{
			res.set_content("MAP ID ANAL ERROR!", "text/plain");
		}
	});

	httpMapUploaderSever.Get("/download", [ & ](const Request& req, Response& res) {
		if (req.has_param("id")) {
			auto val = req.get_param_value("id");
			std::string mapfilename = "unnamed";
			if (FindMapFileNameByID(val, mapfilename))
			{
				std::ifstream t((MapStoragePath + mapfilename), std::ifstream::binary);
				if (t.is_open())
				{
					uint64 datalen;
					t.seekg(0, std::ios::end);
					datalen = t.tellg();
					t.seekg(0, std::ios::beg);
					char* databinary = new char[datalen];
					t.read(databinary, datalen);
					t.close();
					std::string* data = new std::string(databinary, databinary + datalen);
					delete[] databinary;
					res.set_header("Content-Description", "File Transfer");
					res.set_header("Content-Type", "application/octet-stream");
					res.set_header("Content-Disposition", "attachment; filename=" + mapfilename);
					res.set_header("Content-Transfer-Encoding", "binary");
					res.set_header("Connection", "Keep-Alive");

					res.set_content_provider(
						data->size(), // Content length
						[ data ](uint64_t offset, uint64_t length, DataSink& sink) {
						const auto& d = *data;
						sink.write(&d[offset], length);
					},
						[ data ] { delete data; });

				}
				else
				{
					res.set_content("MAP OPEN ANAL ERROR!", "text/plain");
				}
			}
			else
			{
				res.set_content("MAP SEARCH ANAL ERROR!", "text/plain");
			}
		}
		else
		{
			res.set_content("MAP ID ANAL ERROR!", "text/plain");
		}
	});

	httpMapUploaderSever.Post("/uploadmap", [ & ](const Request& req, Response& res) {
		if (req.is_multipart_form_data())
		{
			bool withpatches = req.has_file("haspatches");

			std::string mappassword;
			std::string memhackpassword;
			bool allowmemhack = false;
			if (req.has_file("mapuploadpassword")) {
				mappassword = req.get_file_value("mapuploadpassword").content;
				if (mappassword.size() > 20)
				{
					res.set_content("VERY STRONG PASSWORD... Please back and try again!", "text/plain");
					return;
				}
				if (!AllowedStringText(mappassword))
				{
					res.set_content("PASSWORD CHARACTERS ANAL ERROR!", "text/plain");
					return;
				}
			}

			std::string mapcategory = "RPG (Roleplay)";
			if (req.has_file("mapcategory")) {
				mapcategory = req.get_file_value("mapcategory").content;
				if (!IsOkayCategory(mapcategory))
				{
					res.set_content("CATEGORY ANAL ERROR!", "text/plain");
					return;
				}
			}
			else
			{

			}

			if (req.has_file("memhackuploadpassword")) {
				memhackpassword = req.get_file_value("memhackuploadpassword").content;
				if (memhackpassword == MemHackPassword)
				{
					allowmemhack = true;
				}
			}

			auto size = req.files.size();

			auto ret = req.has_file("file");
			if (ret && size > 0)
			{
				const auto& file = req.get_file_value("file");
				if (file.content.size() > 0x400 && file.content.size() < 0x40000000)
				{
					std::string filefilename = file.filename;
					if (filefilename.size() >= 31)
					{
						res.set_content("MAP NAME LEN ANAL ERROR", "text/plain");
					}
					else // TODO : REPLACE TO LOWER
						if (filesystem::path(filefilename).extension() == ".w3x" ||
							filesystem::path(filefilename).extension() == ".w3m" ||
							filesystem::path(filefilename).extension() == ".W3X" ||
							filesystem::path(filefilename).extension() == ".W3M" ||
							filesystem::path(filefilename).extension() == ".w3X" ||
							filesystem::path(filefilename).extension() == ".w3M" ||
							filesystem::path(filefilename).extension() == ".W3x" ||
							filesystem::path(filefilename).extension() == ".W3m")
						{
							Translit1(filefilename);
							AllowedStringPath(filefilename);

							if (file.content[0] == 'H' &&
								file.content[1] == 'M' &&
								file.content[2] == '3' &&
								file.content[3] == 'W')
							{
								if (MapNeedUpdateOrRemove((MapStoragePath + filefilename), filefilename, mappassword))
								{
									std::filesystem::remove((MapStoragePath + filefilename));
									std::filesystem::remove((MapStoragePath + filefilename) + ".ini");
									std::filesystem::remove((StaticStoragePath + filefilename) + ".map.png");
									std::filesystem::remove((StaticStoragePath + filefilename) + ".preview.png");

									auto ofile = ofstream((MapStoragePath + filefilename), std::ofstream::binary);
									if (ofile.is_open())
									{
										ofile.write(file.content.c_str(), file.content.size());
										ofile.close();
										unsigned int crc32 = 0;

										WTSW3IParser tmpWTSParser = WTSW3IParser();
										if (!UploadMapFileComplete((MapStoragePath + filefilename), filefilename, allowmemhack, res, crc32, tmpWTSParser)
											)
										{
											std::filesystem::remove((MapStoragePath + filefilename));
										}
										else
										{
											if (tmpWTSParser.MAPname.size() == 0)
											{
												std::filesystem::remove((MapStoragePath + filefilename));
												res.set_content("MAP NAME ANAL ERROR", "text/plain");
											}
											else
											{
												std::string mapconfig = "[MapInfo]\nName=" + tmpWTSParser.MAPname +
													"\nHostCmd=" + std::to_string(crc32) + std::to_string(filefilename.size()) +
													"\nBotPath=" + filefilename +
													"\nScriptCrc32=" + std::to_string(crc32) +
													"\nMapCategory=" + mapcategory + "\n";
												ofile = ofstream((MapStoragePath + filefilename) + ".ini", std::ofstream::binary);
												if (ofile.is_open())
												{
													ofile.write(mapconfig.c_str(), mapconfig.size());
													ofile.close();
												}
												MapStruct tmpMapStruct = MapStruct();
												tmpMapStruct.Author = tmpWTSParser.MAPauthor;
												tmpMapStruct.Name = tmpWTSParser.MAPname;
												tmpMapStruct.Description = tmpWTSParser.MAPdescription;
												tmpMapStruct.Players = tmpWTSParser.MAPplayers;
												tmpMapStruct.ContainPatches = withpatches;
												tmpMapStruct.IsMemHack = allowmemhack;
												tmpMapStruct.UpdateCode = mappassword;
												tmpMapStruct.Path = filefilename;
												tmpMapStruct.Rating = 10;
												tmpMapStruct.Downloads = 0;
												tmpMapStruct.crc32 = crc32;
												tmpMapStruct.Category = mapcategory;
												tmpMapStruct.id = -1;
												auto mapid = storage.insert(tmpMapStruct);
												storage.sync_schema();
												char redirectpath[256];
												sprintf_s(redirectpath, "<html><head><meta http-equiv=\"refresh\" content=\"2;url=/viewmap?id=%i\"/></head><body></body></html>",
													mapid);
												res.set_content(redirectpath, "text/html");
											}
										}
									}
									else
									{
										res.set_content("FILE PATH ANAL ERROR", "text/plain");
									}
								}
								else
								{
									res.set_content("UPDATE MAP ANAL ERROR", "text/plain");
								}
							}
							else
							{
								res.set_content("FILE FORMAT ANAL ERROR", "text/plain");
							}
						}
						else
						{
							res.set_content("FILE EXTENSION ANAL ERROR", "text/plain");
						}
				}
				else
				{
					res.set_content("FILE SIZE LENGTH ANAL ERROR", "text/plain");
				}
			}
			else
			{
				res.set_content("FORM FILE FIELD ANAL ERROR", "text/plain");
			}
		}
		else
		{
			res.set_content("FORM ANAL ERROR", "text/plain");
		}
		// filefilename;
		// file.content_type;
		// file.content;
	});

	httpMapUploaderSever.listen("0.0.0.0", 80);


	return 0;
}