// ComputeAssist.hlsl
// for GfxForAll


// API
// 
// {
//   struct ContextGather ui;			// pixel shader or compute shader looping through all pixels
//   ui.init(inpxPos, pxLeftTop);
//   ui.printTxt('A', 'B');
//   Output[ui.pxPos] = ui.dstColor;
// }
//
// {
//   struct ContextScatter ui;			// vertex shader or single thread in compute or pixel shader, onGfxForAllScatter() is used 
//   ui.init(pxLeftTop);
//   ui.printTxt('A', 'B');
// }


struct ContextScatter
{
	// RGBA, alpha 1 is assumed to be opaque
	float4 textColor;
	// in pixels
	int2 pxCursor;
	// window left top
	int2 pxLeftTop;
	// 1/2/3/4
	int scale;
	// -100 if not yet set
	int2 mouseXY;

	void init(int2 inpxLeftTop = int2(0, 0))
	{
		// white, opaque
		textColor = float4(1, 1, 1, 1);
		pxLeftTop = inpxLeftTop;
		pxCursor = pxLeftTop;
		scale = 1;
		mouseXY = int2(-100, -100);
	}

	// e.g. ui.printTxt('I', ' ', 'a', 'm');
	// @param a ascii character or 0
	void printTxt(uint a, uint b = 0, uint c = 0, uint d = 0, uint e = 0, uint f = 0);
	// jump to next line
	void printLF();
	// @param value e.g. 123, 0
	void printInt(int value);
	// print hexadecimal e.g. "0000aa34"
	// @param value 32bit e.g. 0x123, 0xff00
	void printHex(uint value);
	// @param output e.g. g_output from RWTexture2D<float3> g_output : register(u0, space0);
	// @param pos in pixels from left top, left top of the printout
	// @param value
	void printFloat(float value);
	// block in a 8x8 character
	void printBlock(float4 color);
	// circle in a 8x8 character
	void printDisc(float4 color);

	// -------------------------------------------------

	// don't use directly
	void printCharacter(uint ascii);
};

struct ContextGather : ContextScatter
{
	// in pixels
	int2 pxPos;
	// RGBA, alpha 1 is assumed to be opaque
	float4 dstColor;

	void init(int2 inPxPos, int2 inpxLeftTop = int2(0, 0))
	{
		ContextScatter::init(inpxLeftTop);
		pxPos = inPxPos;
		// see through
		dstColor = float4(0, 0, 0, 0);
	}

	// e.g. ui.printTxt('I', ' ', 'a', 'm');
	// @param a ascii character or 0
	void printTxt(uint a, uint b = 0, uint c = 0, uint d = 0, uint e = 0, uint f = 0);
	// jump to next line
	void printLF();
	// @param value e.g. 123, 0
	void printInt(int value);
	// print hexadecimal e.g. "0000aa34"
	// @param value 32bit e.g. 0x123, 0xff00
	void printHex(uint value);
	// @param output e.g. g_output from RWTexture2D<float3> g_output : register(u0, space0);
	// @param pos in pixels from left top, left top of the printout
	// @param value
	void printFloat(float value);
	// block in a 8x8 character
	void printBlock(float4 color);
	//
	void drawCircle(float2 center, float radius, float4 color);
	//
	void drawCrosshair(int2 center, int radius, float4 color);
	// circle in a 8x8 character
	// @return mouseOver
	bool printDisc(float4 color);

	// -------------------------------------------------

	// don't use directly
	void printCharacter(uint ascii);
};

// -------------------------------------------------------------------------------------

// don't use directly
// todo: try different implementations to improve best performance and portability
// @param line 0..7
// @return byte
uint getMiniFontData(uint pos)
{
	// for better performance use texture lookup, this is just easier to integrate

	// ASCII code starting from 32
	// 16x8 letters with 8 bits x 6 lines
	//
	// code to extract:
	// for (int y = 0; y < 48; ++y)
	//	for (int x = 0; x < 16; ++x)
	//		for (int b = 0; b < 8; ++b)
	//		{
	//			int g = (getMiniFontData(x + 16 * y) >> (7 - b)) & 1;
	//			putPixel(x * 8 + b, y, FloatingPointPixel(g, g, g));
	//		}
	// generated with https://www.digole.com/tools/PicturetoC_Hex_converter.php
	static const uint g_miniFont[] = {
		0x00, 0x30, 0x6c, 0x6c, 0x30, 0x00, 0x38, 0x60, 0x18, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 
		0x00, 0x78, 0x6c, 0x6c, 0x7c, 0xc6, 0x6c, 0x60, 0x30, 0x30, 0x66, 0x30, 0x00, 0x00, 0x00, 0x0c, 
		0x00, 0x78, 0x6c, 0xfe, 0xc0, 0xcc, 0x38, 0xc0, 0x60, 0x18, 0x3c, 0x30, 0x00, 0x00, 0x00, 0x18, 
		0x00, 0x30, 0x00, 0x6c, 0x78, 0x18, 0x76, 0x00, 0x60, 0x18, 0xff, 0xfc, 0x00, 0xfc, 0x00, 0x30, 
		0x00, 0x30, 0x00, 0xfe, 0x0c, 0x30, 0xdc, 0x00, 0x60, 0x18, 0x3c, 0x30, 0x00, 0x00, 0x00, 0x60, 
		0x00, 0x00, 0x00, 0x6c, 0xf8, 0x66, 0xcc, 0x00, 0x30, 0x30, 0x66, 0x30, 0x30, 0x00, 0x30, 0xc0, 
		0x00, 0x30, 0x00, 0x6c, 0x30, 0xc6, 0x76, 0x00, 0x18, 0x60, 0x00, 0x00, 0x30, 0x00, 0x30, 0x80, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 
		0x7c, 0x30, 0x78, 0x78, 0x1c, 0xfc, 0x38, 0xfc, 0x78, 0x78, 0x00, 0x00, 0x18, 0x00, 0x60, 0x78, 
		0xc6, 0x70, 0xcc, 0xcc, 0x3c, 0xc0, 0x60, 0xcc, 0xcc, 0xcc, 0x30, 0x30, 0x30, 0x00, 0x30, 0xcc, 
		0xce, 0x30, 0x0c, 0x0c, 0x6c, 0xf8, 0xc0, 0x0c, 0xcc, 0xcc, 0x30, 0x30, 0x60, 0xfc, 0x18, 0x0c, 
		0xde, 0x30, 0x38, 0x38, 0xcc, 0x0c, 0xf8, 0x18, 0x78, 0x7c, 0x00, 0x00, 0xc0, 0x00, 0x0c, 0x18, 
		0xf6, 0x30, 0x60, 0x0c, 0xfe, 0x0c, 0xcc, 0x30, 0xcc, 0x0c, 0x00, 0x00, 0x60, 0x00, 0x18, 0x30, 
		0xe6, 0x30, 0xcc, 0xcc, 0x0c, 0xcc, 0xcc, 0x30, 0xcc, 0x18, 0x30, 0x30, 0x30, 0xfc, 0x30, 0x00, 
		0x7c, 0xfc, 0xfc, 0x78, 0x1e, 0x78, 0x78, 0x30, 0x78, 0x70, 0x30, 0x30, 0x18, 0x00, 0x60, 0x30, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 
		0x7c, 0x30, 0xfc, 0x3c, 0xf8, 0xfe, 0xfe, 0x3c, 0xcc, 0x78, 0x1e, 0xe6, 0xf0, 0xc6, 0xc6, 0x38, 
		0xc6, 0x78, 0x66, 0x66, 0x6c, 0x62, 0x62, 0x66, 0xcc, 0x30, 0x0c, 0x66, 0x60, 0xee, 0xe6, 0x6c, 
		0xde, 0xcc, 0x66, 0xc0, 0x66, 0x68, 0x68, 0xc0, 0xcc, 0x30, 0x0c, 0x6c, 0x60, 0xfe, 0xf6, 0xc6, 
		0xde, 0xcc, 0x7c, 0xc0, 0x66, 0x78, 0x78, 0xc0, 0xfc, 0x30, 0x0c, 0x78, 0x60, 0xfe, 0xde, 0xc6,
		0xde, 0xfc, 0x66, 0xc0, 0x66, 0x68, 0x68, 0xce, 0xcc, 0x30, 0xcc, 0x6c, 0x62, 0xd6, 0xce, 0xc6,
		0xc0, 0xcc, 0x66, 0x66, 0x6c, 0x62, 0x60, 0x66, 0xcc, 0x30, 0xcc, 0x66, 0x66, 0xc6, 0xc6, 0x6c,
		0x78, 0xcc, 0xfc, 0x3c, 0xf8, 0xfe, 0xf0, 0x3e, 0xcc, 0x78, 0x78, 0xe6, 0xfe, 0xc6, 0xc6, 0x38,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0xfc, 0x78, 0xfc, 0x78, 0xfc, 0xcc, 0xcc, 0xc6, 0xc6, 0xcc, 0xfe, 0x78, 0xc0, 0x78, 0x10, 0x00,
		0x66, 0xcc, 0x66, 0xcc, 0xb4, 0xcc, 0xcc, 0xc6, 0xc6, 0xcc, 0xc6, 0x60, 0x60, 0x18, 0x38, 0x00,
		0x66, 0xcc, 0x66, 0xe0, 0x30, 0xcc, 0xcc, 0xc6, 0x6c, 0xcc, 0x8c, 0x60, 0x30, 0x18, 0x6c, 0x00,
		0x7c, 0xcc, 0x7c, 0x70, 0x30, 0xcc, 0xcc, 0xd6, 0x38, 0x78, 0x18, 0x60, 0x18, 0x18, 0xc6, 0x00,
		0x60, 0xdc, 0x6c, 0x1c, 0x30, 0xcc, 0xcc, 0xfe, 0x38, 0x30, 0x32, 0x60, 0x0c, 0x18, 0x00, 0x00,
		0x60, 0x78, 0x66, 0xcc, 0x30, 0xcc, 0x78, 0xee, 0x6c, 0x30, 0x66, 0x60, 0x06, 0x18, 0x00, 0x00,
		0xf0, 0x1c, 0xe6, 0x78, 0x78, 0xfc, 0x30, 0xc6, 0xc6, 0x78, 0xfe, 0x78, 0x02, 0x78, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
		0x30, 0x00, 0xe0, 0x00, 0x1c, 0x00, 0x38, 0x00, 0xe0, 0x30, 0x0c, 0xe0, 0x70, 0x00, 0x00, 0x00, 
		0x30, 0x00, 0x60, 0x00, 0x0c, 0x00, 0x6c, 0x00, 0x60, 0x00, 0x00, 0x60, 0x30, 0x00, 0x00, 0x00, 
		0x18, 0x78, 0x60, 0x78, 0x0c, 0x78, 0x60, 0x76, 0x6c, 0x70, 0x0c, 0x66, 0x30, 0xcc, 0xf8, 0x78, 
		0x00, 0x0c, 0x7c, 0xcc, 0x7c, 0xcc, 0xf0, 0xcc, 0x76, 0x30, 0x0c, 0x6c, 0x30, 0xfe, 0xcc, 0xcc, 
		0x00, 0x7c, 0x66, 0xc0, 0xcc, 0xfc, 0x60, 0xcc, 0x66, 0x30, 0x0c, 0x78, 0x30, 0xfe, 0xcc, 0xcc, 
		0x00, 0xcc, 0x66, 0xcc, 0xcc, 0xc0, 0x60, 0x7c, 0x66, 0x30, 0xcc, 0x6c, 0x30, 0xd6, 0xcc, 0xcc, 
		0x00, 0x76, 0xdc, 0x78, 0x76, 0x78, 0xf0, 0x0c, 0xe6, 0x78, 0xcc, 0xe6, 0x78, 0xc6, 0xcc, 0x78, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x18, 0xe0, 0x76, 0xff, 
		0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x18, 0x30, 0xdc, 0xff, 
		0xdc, 0x76, 0xdc, 0x7c, 0x7c, 0xcc, 0xcc, 0xc6, 0xc6, 0xcc, 0xfc, 0x30, 0x18, 0x30, 0x00, 0xff,
		0x66, 0xcc, 0x76, 0xc0, 0x30, 0xcc, 0xcc, 0xd6, 0x6c, 0xcc, 0x98, 0xe0, 0x00, 0x1c, 0x00, 0xff, 
		0x66, 0xcc, 0x66, 0x78, 0x30, 0xcc, 0xcc, 0xfe, 0x38, 0xcc, 0x30, 0x30, 0x18, 0x30, 0x00, 0xff, 
		0x7c, 0x7c, 0x60, 0x0c, 0x34, 0xcc, 0x78, 0xfe, 0x6c, 0x7c, 0x64, 0x30, 0x18, 0x30, 0x00, 0xff, 
		0x60, 0x0c, 0xf0, 0xf8, 0x18, 0x76, 0x30, 0x6c, 0xc6, 0x0c, 0xfc, 0x1c, 0x18, 0xe0, 0x00, 0xff, 
		0xf0, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff
	};

	return g_miniFont[pos];
}

// don't use directly
// can be used for scatter and gather
// @param ascii 32..127 are valid characters
// @param pxPos int2(0..7, 0..7)
// @return true if there should be a pixel, false if not or outside the valid range
bool fontLookup(uint ascii, int2 pxPos)
{
	if((uint)pxPos.x >= 8 || (uint)pxPos.y >= 8)
		return false;

	if (ascii <= 32 || ascii > 127)
		return false;

	// 0..16*6-1
	uint chr = ascii - 32;
	// uint2(0..127, 0..47) 
	uint2 chrPos = int2(chr % 16, chr / 16);
	uint2 pixel = uint2(chrPos.x * 8 + pxPos.x, chrPos.y * 8 + pxPos.y);
	uint byteId = pixel.x / 8 + pixel.y * 16;
	// 0..7
	uint bitId	= pxPos.x;

	// 0..ff
	uint byteValue = getMiniFontData(byteId);
	uint g		   = (byteValue >> (7 - bitId)) & 1;

	return (byteValue >> (7 - bitId)) & 1;
}

void ContextGather::printCharacter(uint ascii)
{
	int2 pxLocal = (uint2)(pxPos - pxCursor) / scale;

	if(fontLookup(ascii, pxLocal))
		dstColor = textColor;

	pxCursor.x += 8 * scale;
}

void ContextGather::printTxt(uint a, uint b, uint c, uint d, uint e, uint f)
{
	if (a) printCharacter(a);
	if (b) printCharacter(b);
	if (c) printCharacter(c);
	if (d) printCharacter(d);
	if (e) printCharacter(e);
	if (f) printCharacter(f);
}

void ContextGather::printLF()
{
	pxCursor.x = pxLeftTop.x;
	pxCursor.y += 8 * scale;
}

void ContextGather::printInt(int value)
{
	// leading '-'
	if (value < 0)
	{
		printCharacter('-');
		value = -value;
	}
	if (value == 0)
	{
		printCharacter('0');
		return;
	}
	// move to right depending on number length
	{
		uint tmp = (uint)value;
		while (tmp)
		{
			pxCursor.x += 8 * scale;
			tmp /= 10;
		}
	}
	// digits
	{
		int backup = pxCursor.x;
		uint tmp = (uint)value;
		while (tmp)
		{
			// 0..9
			uint digit = tmp % 10;
			tmp /= 10;
			// go backwards
			pxCursor.x -= 8 * scale;
			printCharacter('0' + digit);
			// counter +=8 from printCharacter ()
			pxCursor.x -= 8 * scale;
		}
		pxCursor.x = backup;
	}
}

void ContextGather::printHex(uint value)
{
	// 8 nibbles
	for(int i = 7; i >= 0; --i)
	{
		// 0..15
		uint nibble = (value >> (i * 4)) & 0xf;
		uint start = (nibble < 10) ? '0' : ('A' - 10);
		printCharacter(start + nibble);
	}
}

void ContextGather::printFloat(float value)
{
	printInt((int)value);
	float fractional = frac(abs(value));

	printCharacter('.');

	uint digitCount = 3;

	// todo: unit tests, this is likely wrong at lower precision

	// fractional digits
	for(uint i = 0; i < digitCount; ++i)
	{
		fractional *= 10;
		// 0..9
		uint digit = (uint)(fractional);
		fractional = frac(fractional);
		printCharacter('0' + digit);
	}
}

void ContextGather::printBlock(float4 color)
{
	float2 pxLocal = (float2)(pxPos - pxCursor) / scale - float2(3.5f, 3.5f);

	float mask = saturate(4 - max(abs(pxLocal.x), abs(pxLocal.y)));

//	dstColor = lerp(dstColor, float4(color.rgb, 1), color.a * mask);
	if(mask)
		dstColor = color;

	pxCursor.x += 8 * scale;
}

void ContextGather::drawCircle(float2 center, float radius, float4 color)
{
	float2 pxLocal = (float2)(pxPos - center);

	float d = radius - length(pxLocal);
	float mask = saturate(d + 1) - saturate(d);

//	dstColor = lerp(dstColor, float4(color.rgb, 1), color.a * mask);
	if(mask > 0.5f)
		dstColor = color;
}

void ContextGather::drawCrosshair(int2 center, int radius, float4 color)
{
	int2 pxLocal = abs(pxPos - center);
	int dist = max(pxLocal.x, pxLocal.y);

	if((pxLocal.x == 0 || pxLocal.y == 0) && dist <= radius)
		dstColor = color;
}

bool ContextGather::printDisc(float4 color)
{
	float2 pxLocal = (float2)(pxPos - pxCursor) / scale - float2(3.5f, 3.5f);

	float mask = saturate(4 - length(pxLocal));

//	dstColor = lerp(stColor, float4(color.rgb, 1), color.a * mask);
	if(mask)
		dstColor = color;

	bool mouseOver = length((float2)(mouseXY - pxCursor) / scale - float2(3.5f, 3.5f)) < 4;

	pxCursor.x += 8 * scale;

	return mouseOver;
}

// -------------------------------------------------------------------------------------

// implement this in your code
void onGfxForAllScatter(int2 pxPos, float4 color);

void ContextScatter::printCharacter(uint ascii)
{
	[loop] for(int y = 0; y < 8 * scale; ++y)
	[loop] for(int x = 0; x < 8 * scale; ++x)
		if(fontLookup(ascii, int2(x, y) / scale))
			onGfxForAllScatter(pxCursor + int2(x, y), textColor);

	pxCursor.x += 8 * scale;
}

void ContextScatter::printTxt(uint a, uint b, uint c, uint d, uint e, uint f)
{
	if (a) printCharacter(a);
	if (b) printCharacter(b);
	if (c) printCharacter(c);
	if (d) printCharacter(d);
	if (e) printCharacter(e);
	if (f) printCharacter(f);
}

void ContextScatter::printLF()
{
	pxCursor.x = pxLeftTop.x;
	pxCursor.y += 8 * scale;
}

void ContextScatter::printInt(int value)
{
	// leading '-'
	if (value < 0)
	{
		printCharacter('-');
		value = -value;
	}
	if (value == 0)
	{
		printCharacter('0');
		return;
	}
	// move to right depending on number length
	{
		uint tmp = (uint)value;
		while (tmp)
		{
			pxCursor.x += 8 * scale;
			tmp /= 10;
		}
	}
	// digits
	{
		uint tmp = (uint)value;
		while (tmp)
		{
			// 0..9
			uint digit = tmp % 10;
			tmp /= 10;
			// go backwards
			pxCursor.x -= 8 * scale;
			printCharacter('0' + digit);
			// counter +=8 from printCharacter ()
			pxCursor.x -= 8 * scale;
		}
	}
}

void ContextScatter::printHex(uint value)
{
	// 8 nibbles
	for(int i = 7; i >= 0; --i)
	{
		// 0..15
		uint nibble = (value >> (i * 4)) & 0xf;
		uint start = (nibble < 10) ? '0' : ('A' - 10);
		printCharacter(start + nibble);
	}
}

void ContextScatter::printFloat(float value)
{
	printInt((int)value);
	float fractional = frac(abs(value));

	printCharacter('.');

	uint digitCount = 3;

	// todo: unit tests, this is likely wrong at lower precision

	// fractional digits
	for(uint i = 0; i < digitCount; ++i)
	{
		fractional *= 10;
		// 0..9
		uint digit = (uint)(fractional);
		fractional = frac(fractional);
		printCharacter('0' + digit);
	}
}

void ContextScatter::printBlock(float4 color)
{
	[loop] for(int y = 0; y < 8 * scale; ++y)
	[loop] for(int x = 0; x < 8 * scale; ++x)
	{
		float2 pxLocal = (float2(x, y) ) / scale - float2(3.5f, 3.5f);

		float mask = saturate(4 - max(abs(pxLocal.x), abs(pxLocal.y)));

		if(mask)
			onGfxForAllScatter(pxCursor + int2(x,y), color);
	}

	pxCursor.x += 8 * scale;
}

void ContextScatter::printDisc(float4 color)
{
	[loop] for(int y = 0; y < 8 * scale; ++y)
	[loop] for(int x = 0; x < 8 * scale; ++x)
	{
		float2 pxLocal = (float2(x, y) ) / scale - float2(3.5f, 3.5f);

		float mask = saturate(4 - length(pxLocal));

		if(mask)
			onGfxForAllScatter(pxCursor + int2(x,y), color);
	}

	pxCursor.x += 8 * scale;
}
