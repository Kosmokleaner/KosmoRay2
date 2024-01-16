#pragma once
#include "deargpu.h"
#include <vector>

// todo:
// don't use STL
// test with MaxBufferSize==FileSize

// Features:
// * should work with files > 4GB
// * should handle 10, 10+13, 13, 13+10 returns transparently

class CLineReader
{
public:
	// FileName must not be 0
	// BufferSize in Bytes
	CLineReader();
	
	virtual ~CLineReader();

	void FreeData();

	// MaxBufferSize should be at least the size of a line, 4MB works good for big files
	bool Open(const wchar_t*FileName, const size_t MaxBufferSize = 4 * 1024 * 1024);

	// End of File
	// true=end of file reached, false otherwise
	bool EndOfFile() const;

	// returns an empty line if the file has already ended
	// MaxLineSize includes the 0 terminator
	void GetLine(char outLine[], const UINT MaxLineSize);

	void GetStatistics(size_t &outCurrentPos, size_t &outSize) const;
	
private: // --------------------------------------------

	FILE *							FileHandle;
	size_t							FileSize;		// only for statistics
	size_t							FilePos;		// only for statistics

	std::vector<CHAR>				Buffer;
	UINT							BufferPos;
	size_t							BufferSize;		// can be smaller than MaxBufferSize if the EOF was reached
	bool							bEndOfFile;

	void ReadBuffer();

	char TopCharacter();

	// always returns a character, even if EOF was reached
	void PopCharacter();
};
