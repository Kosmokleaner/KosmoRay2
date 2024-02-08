#include "LineReader.h"
#include <assert.h>
#include <share.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>

CLineReader::CLineReader() 
	:FileHandle(0), FileSize(0), FilePos(0), BufferPos(0), BufferSize(0), bEndOfFile(true)
{
}

bool CLineReader::Open(const wchar_t*FileName, const size_t MaxBufferSize)
{
	assert(FileName);

	if(FileHandle = _wfsopen(FileName, L"rb", _SH_DENYWR))
	{
		// FileSize
		int LocalHandle;

		_wsopen_s(&LocalHandle, FileName, O_RDONLY, _SH_DENYWR, _S_IREAD | _S_IWRITE);

		if(LocalHandle == -1)
		{
			// file found but cannot get FileSize
			fclose(FileHandle);
			return false;
		}

		FileSize = _filelength(LocalHandle);

		Buffer.resize(glm::min(FileSize, MaxBufferSize));

		_close(LocalHandle);

		bEndOfFile = false;

		return true;
	}


	// file not found
	return false;
}

CLineReader::~CLineReader()
{
	FreeData();
}

void CLineReader::FreeData()
{
	if(FileHandle)
	{
		fclose(FileHandle);
		FileHandle = 0;
	}

	// free the memory
	Buffer.clear();
}

bool CLineReader::EndOfFile() const
{
	return bEndOfFile;
}

void CLineReader::GetStatistics(size_t &outCurrentPos, size_t &outSize) const
{
	outCurrentPos = FilePos;
	outSize = FileSize;
}


char CLineReader::TopCharacter()
{
	if(BufferPos >= BufferSize)
	{
		ReadBuffer();
	}

	return Buffer[BufferPos];
}

void CLineReader::PopCharacter()
{
	if(BufferPos >= BufferSize)
	{
		ReadBuffer();
	}

	++FilePos;
	++BufferPos;
}

void CLineReader::GetLine(char outLine[], const UINT MaxLineSize)
{
	char *Dst = outLine;

	uint32 i = 0;
	for(;;)
	{
		if(EndOfFile())
		{
			break;
		}

		char c = TopCharacter();

		PopCharacter();

		if(c == 10 || c == 13)
		{
			char cNext = TopCharacter();

			if(c != cNext && (cNext == 10 || cNext == 13))
			{
				PopCharacter();
			}
			break;
		}

		if(i < MaxLineSize - 1)
		{
			*Dst++ = c;
		}

		++i;
	}

	// 0 terminaton
	*Dst = 0;
}

void CLineReader::ReadBuffer()
{
	BufferSize = fread(&Buffer[0], 1, Buffer.size(), FileHandle);
	BufferPos = 0;

	if(!BufferSize)
	{
		bEndOfFile = true;
	}
}
