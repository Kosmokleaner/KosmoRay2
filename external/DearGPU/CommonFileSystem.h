#pragma once

#include "deargpu.h"

#include <string>

std::string GetErrorString(HRESULT hr);

// can be optimized
// e.g. "c:\temp/test.txt" -> "c:\temp"
// e.g. "test.txt" -> ""
// @param FilePath must not be 0
std::wstring ExtractPathW(const wchar_t*FilePath);

std::wstring ConvertToUnicode(const char *In);
std::string ConvertToASCII(const wchar_t*In);

bool FolderExist(const wchar_t* FolderName);

bool FileExist(const wchar_t* FileName);

// @param FileName e.g. L"C:\\Temp\\Testa\\TestB/TestC\\TestD//TestFile.txt"
void CreatePathRecursive(const wchar_t* FileName);

std::string Format(const char *fmt, ...);
std::wstring Format(const wchar_t *fmt, ...);

// load a file into a string and 0 terminates it
// @param OutFileSize without 0 terminator
// @return need to call delete [] on it
const char* LoadTextFile(const wchar_t* FileName, uint32& OutFileSize);
const char* LoadTextFile(const char* FileName, uint32& OutFileSize);


uint32 IO_GetFileSize(const wchar_t* Name);

uint32 IO_GetFileSize(const char* Name);