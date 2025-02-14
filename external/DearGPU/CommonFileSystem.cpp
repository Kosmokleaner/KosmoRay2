
#include "CommonFileSystem.h"
//#include <fileapi.h>
#include <windows.h>


// can be optimized
// e.g. "c:\temp/test.txt" -> "c:\temp"
// e.g. "test.txt" -> ""
// @param FilePath must not be 0
std::wstring ExtractPathW(const wchar_t *FilePath)
{
	assert(FilePath);

	std::wstring ret;

	const wchar_t *Ptr = FilePath;
	// one behind the last character
	const wchar_t *End = Ptr;

	while(*Ptr)
	{
		if(*Ptr == L'\\' || *Ptr == L'/')
		{
			End = Ptr;
		}

		++Ptr;
	}

	size_t size = End - FilePath;
	ret.resize(size);

	memcpy(&ret[0], FilePath, size * sizeof(FilePath[0]));

	return ret;
}

std::wstring ConvertToUnicode(const char *In)
{
	assert(In);

	std::wstring ret;

	ret.resize(strlen(In));

	uint8 *Src = (uint8 *)In;
	wchar_t *Dst = &ret[0];
	while(*Src)
	{
		*Dst++ = *Src++;
	}

	// 0 termination
	*Dst = 0;

	return ret;
}


std::string ConvertToASCII(const wchar_t *In)
{
	assert(In);

	std::string ret;

	ret.resize(wcslen(In));

	wchar_t* Src = (wchar_t*)In;
	uint8* Dst = (uint8*)&ret[0];
	while(*Src)
	{
		wchar_t  c = *Src++;

		if(c > 255)
		{
			c = '?';
		}

		*Dst++ = (uint8)c;
	}

	// 0 termination
	*Dst = 0;

	return ret;
}

bool FolderExist(const wchar_t* FolderName)
{
	assert(FolderName);

	if (_waccess(FolderName, 0) == -1)
	{
		//File not found
		return false;
	}

	uint32 attr = GetFileAttributesW(FolderName);

	if (!(attr & FILE_ATTRIBUTE_DIRECTORY))
	{
		// File is not a directory
		return false;
	}

	return true;
}

bool FileExist(const wchar_t* FileName)
{
	// e.g.
	// assert(FileExist(L"c:\\Temp\\a.txt"));			// assuming the files a.txt exist
	// assert(!FileExist(L"c:\\Temp\\b.txt"));			// assuming the files b.txt does not exist

	// http://forums.anandtech.com/archive/index.php/t-122558.html
	uint32 attr = GetFileAttributesW(FileName);

	return attr != -1;
}

std::string Format(const char *fmt, ...)
{
	const uint32 MAX_BUFFER = 1024;

	char textString[MAX_BUFFER*5] = {'\0'};

	va_list args;
	va_start(args, fmt);
	vsnprintf_s(textString, MAX_BUFFER*5, fmt, args);
	va_end(args);
	std::string retStr = textString;
	return retStr;
}


std::wstring Format(const wchar_t *fmt, ...)
{
	const uint32 MAX_BUFFER = 1024;

	wchar_t  textString[MAX_BUFFER * 5] = {'\0'};

	va_list args;
	va_start ( args, fmt );
	vswprintf ( textString, MAX_BUFFER*5, fmt, args );
	va_end ( args );
	std::wstring retStr = textString;
	return retStr;
}

void CreatePathRecursive(const wchar_t* FileName)
{
	std::wstring FolderName = ExtractPathW(FileName);

	std::wstring LocalFolderName;

	// avoid memory allocations
	LocalFolderName.reserve(256);

	const wchar_t*Ptr = &FolderName[0];

	if(*Ptr)
	for(;;)
	{
		wchar_t  c = *Ptr++;

		if(c == L'\\' || c == L'/' || c == 0)
		{
			if(!FolderExist(LocalFolderName.c_str()))
			{
				if(CreateDirectoryW(LocalFolderName.c_str(), NULL) == 0)
				{
					assert(0);
				}
			}

			if(c == 0)
			{
				break;
			}
		}

		LocalFolderName.push_back(c);
	}
}



const char* LoadTextFile(const char* FileName, uint32& OutFileSize)
{
	std::wstring FileName2 = ConvertToUnicode(FileName);

	return LoadTextFile(FileName2.c_str(), OutFileSize);
}

const char* LoadTextFile(const wchar_t* FileName, uint32& OutFileSize)
{
	FILE *in = 0;

	if(_wfopen_s(&in, FileName, L"rb") != 0)
	{
		return 0;
	}

	assert(in);

	// todo: check to fit in 32bit
	OutFileSize = IO_GetFileSize(FileName);

	if(OutFileSize == -1)
	{
		// internal error 
		return 0;
	}

	// + 1 for /0 terminator
	char* buf = new char[OutFileSize + 1];

	if(fread(buf, OutFileSize, 1, in) != 1)
	{
		assert(0);
		delete [] buf;
		return 0;
	}

	// /0 terminate
	buf[OutFileSize] = 0;

	fclose(in);
	return buf;
}

std::string GetErrorString(HRESULT hr)
{
	char* errorText = 0;

	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, 0, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&errorText, 1024, 0);

	char str[2048];
	snprintf(str, sizeof(str), "Error (hr=%x): %s\n", hr, errorText ? errorText : "Unknown Error");

	std::string Ret = str;

	LocalFree(errorText);

	return Ret;
}




uint32 IO_GetFileSize(const wchar_t* Name)
{
	assert(Name);

	WIN32_FILE_ATTRIBUTE_DATA fileInfo;

	if (!GetFileAttributesExW(Name, GetFileExInfoStandard, (void*)&fileInfo))
	{
		return 0;
	}

	// cannot handle files larger than 32bit 4GB
	assert(0 == fileInfo.nFileSizeHigh);

	return (uint32)fileInfo.nFileSizeLow;
}


uint32 IO_GetFileSize(const char* Name)
{
	assert(Name);
	int handle, size;	// test2

	handle = open(Name, O_RDONLY);
	if (handle == -1)
	{
		return 0;
	}

	size = filelength(handle);
	close(handle);

	return size;
}