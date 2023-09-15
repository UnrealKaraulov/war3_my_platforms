#pragma once
#include <string>
#include <vector>
#include "utils.h"

class SmallBinReader
{
public:
    std::vector<unsigned char> internaldata;
    bool ErrorFound = false;
    int Operations = 0; // for detect error

    SmallBinReader(const unsigned char* data, int size)
    {
        internaldata = std::vector<unsigned char>(data, data + size);
    }

    std::string ReadStr()
    {
        if (internaldata.empty())
        {
            ErrorFound = true;
            return string();
        }
        Operations++;
        std::vector<unsigned char> outstr;

        while (!internaldata.empty() && internaldata[0] != '\0')
        {
            outstr.push_back(internaldata[0]);
            internaldata.erase(internaldata.begin());
        }
        if (!internaldata.empty())
            internaldata.erase(internaldata.begin());

        return std::string(&outstr[0], &outstr[0] + outstr.size());
    }

    unsigned char ReadByte()
    {
        if (internaldata.empty())
        {
            ErrorFound = true;
            return 0;
        }
        Operations++;
        unsigned char result = internaldata[0];
        internaldata.erase(internaldata.begin());
        return result;
    }

    char ReadChar()
    {
        if (internaldata.empty())
        {
            ErrorFound = true;
            return 0;
        }
        Operations++;

        char result = *(char*)&internaldata[0];
        internaldata.erase(internaldata.begin());
        return result;
    }

    short ReadShort()
    {
        if (internaldata.empty() || internaldata.size() < 2)
        {
            ErrorFound = true;
            return 0;
        }
        Operations++;

        short result = *(short*)&internaldata[0];
        internaldata.erase(internaldata.begin());
        return result;
    }

    unsigned short ReadUShort()
    {
        if (internaldata.empty() || internaldata.size() < 2)
        {
            ErrorFound = true;
            return 0;
        }
        Operations++;

        unsigned short result = *(unsigned short*)&internaldata[0];
        internaldata.erase(internaldata.begin());
        return result;
    }

    int ReadInt()
    {
        if (internaldata.empty() || internaldata.size() < 4)
        {
            ErrorFound = true;
            return 0;
        }
        Operations++;

        int result = *(int*)&internaldata[0];
        internaldata.erase(internaldata.begin());
        internaldata.erase(internaldata.begin());
        internaldata.erase(internaldata.begin());
        internaldata.erase(internaldata.begin());
        return result;
    }

    int ReadUInt()
    {
        if (internaldata.empty() || internaldata.size() < 4)
        {
            ErrorFound = true;
            return 0;
        }
        Operations++;

        unsigned int result = *(unsigned int*)&internaldata[0];
        internaldata.erase(internaldata.begin());
        internaldata.erase(internaldata.begin());
        internaldata.erase(internaldata.begin());
        internaldata.erase(internaldata.begin());
        return result;
    }

};

class WTSW3IParser
{
public:

    std::string MAPname;
    std::string MAPauthor;
    std::string MAPdescription;
    std::string MAPplayers;

    void readW3I(const char* data, int len)
    {
        if (!data)
        {
            return;
        }
        SmallBinReader tmpSmallBinReader((const unsigned char*)data, len);
        int version = tmpSmallBinReader.ReadInt();// version 18 - roc , 25+ tft, warning ! format not equal !
        tmpSmallBinReader.ReadInt();
        tmpSmallBinReader.ReadInt();
        MAPname = GetStrByID(tmpSmallBinReader.ReadStr().c_str());
        AllowedStringText(MAPname);
        MAPauthor = GetStrByID(tmpSmallBinReader.ReadStr().c_str());
        AllowedStringText(MAPauthor);
        if (MAPauthor.size() == 0)
            MAPauthor = "Unknown";
        MAPdescription = GetStrByID(tmpSmallBinReader.ReadStr().c_str());
        AllowedStringText(MAPdescription);
        if (MAPdescription.size() == 0)
            MAPdescription = "Unknown";
        MAPplayers = GetStrByID(tmpSmallBinReader.ReadStr().c_str());
        AllowedStringText(MAPplayers);
        if (MAPplayers.size() == 0)
            MAPplayers = "Unknown";
    }


    struct TriggerStr
    {
        std::string id;
        std::string text;
    };

    std::vector<TriggerStr> TriggerStrList = std::vector<TriggerStr>();

    std::string GetStrByID(const char* str2)
    {
        char* str = (char*)str2;
        if (strstr(str, "trigstr_") || strstr(str, "TRIGSTR_"))
        {
            str += strlen("trigstr_");

            for (auto const& s : TriggerStrList)
            {
                if (s.id == str)
                    return s.text;
            }

            while (str[0] == '0')
            {
                str++;
            }

            for (auto const& s : TriggerStrList)
            {
                if (s.id == str)
                    return s.text;
            }

            return "";
        }
        else
        {
            for (auto const& s : TriggerStrList)
            {
                if (s.id == str)
                    return s.text;
            }

            return str;
        }
    }
   
    void readWTS(const char* data2)
    {
        char* data = (char*)data2;
        if (!data || strlen(data) == 0)
        {
            return;
        }
        TriggerStr tmpTriggerStr = TriggerStr();

        while (true)
        {
            tmpTriggerStr = TriggerStr();
            if (strlen(data) <= 10) // STRING {} = 10 chars minimum
            {
                break;
            }
            data = strstr(data, "STRING ");
            if (!data)
            {
                break;
            }
            data += strlen("STRING ");
            if (strlen(data) <= 4)
                break;
            while (!isdigit(data[0]) && data[0] != '\0')
            {
                tmpTriggerStr.id += data[0];
                data++;
            }
            if (strlen(data) <= 4)
                break;
            while (isdigit(data[0]))
            {
                tmpTriggerStr.id += data[0];
                data++;
            }
            if (strlen(data) <= 4 || tmpTriggerStr.id.size() == 0)
                break;

            data = strstr(data, "{");
            if (!data)
            {
                break;
            }

            if (strlen(data) <= 4)
                break;

            data += 1;
          
            while (data[0] == '\n' || data[0] == '\r')
            {
                data++;
            }

            if (strlen(data) <= 3)
                break;

            char* dataend = strstr(data, "}");
            if (!dataend)
            {
                break;
            }
            tmpTriggerStr.text = std::string(data, dataend);

            TriggerStrList.push_back(tmpTriggerStr);

            data = dataend;
        }

    }
};