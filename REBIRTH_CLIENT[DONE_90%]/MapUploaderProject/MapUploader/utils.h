#pragma once
#include <string>


void AllowedStringPath(std::string& str)
{
	for (auto& c : str)
	{
		if (isalnum(c) || c == '_'
			|| c == '.' || c == '[' || c == ']'
			|| c == '(' || c == ')' || c == '!')
		{

		}
		else
		{
			c = '_';
		}
	}
}

bool AllowedStringText(std::string& str)
{
	bool retval = true;
	for (auto& c : str)
	{
		if (c == '<')
		{
			retval = false;
			c = '(';
		}
		else if (c == '[')
		{
			retval = false;
			c = '{';
		}
	}

	return retval;
}

void findAndReplaceAll(std::string& data, std::string toSearch, std::string replaceStr)
{
	// Get the first occurrence
	size_t pos = data.find(toSearch);

	// Repeat till end is reached
	while (pos != std::string::npos)
	{
		// Replace this occurrence of Sub String
		data.replace(pos, toSearch.size(), replaceStr);
		// Get the next occurrence from the current position
		pos = data.find(toSearch, pos + replaceStr.size());
	}
}


//********************************************************************************************************//
#define RUS_ABC "а","б","в","г","д","е","ё","ж","з","и","й","к","л","м","н","о","п","р","с","т","у","ф","х","ц","ч","ш","щ","ь","ы","ъ","э","ю" ,"я" ," ","А","Б","В","Г","Д","Е","Ё" ,"Ж","З","И","Й","К","Л","М","Н","О","П","Р","С","Т","У","Ф","Х","Ц","Ч","Ш","Щ","Ь","Ы","Ъ","Э","Ю","Я","№", '\0'
#define ANG_ABC "a","b","v","g","d","e","e","j","z","i","y","k","l","m","n","o","p","r","s","t","u","f","h","c","h","s","s","\'","i","\'","e","u","a","_","A","B","V","G","D","E","E","J","Z","I","Y","K","L","M","N","O","P","R","S","T","U","F","H","C","H","S","S","\'","I","\'","E","U","A","#", '\0'


void Translit1(std::string & str) {
   
    char  szRusABC[][5] = { RUS_ABC };
    char  szEngABC[][2] = { ANG_ABC };
	for (int i = 0; ; i++)
	{
		if (szRusABC[i][0] == '\0')
			break;
		findAndReplaceAll(str, szRusABC[i], szEngABC[i]);
	}
}