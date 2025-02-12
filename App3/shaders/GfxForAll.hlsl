// ComputeAssist.hlsl
// for GfxForAll


// API
// 
// {
//   struct ContextGather context;			// pixel shader or compute shader looping through all pixels
//   context.init(inpxPos, pxLeftTop);
//   printTxt(context, 'A', 'B');
//   Output[context.pxPos] = context.dstColor;
// }
//
// {
//   struct ContextScatter context;			// vertex shader or single thread in compute or pixel shader, onGfxForAllScatter() is used 
//   context.init(pxLeftTop);
//   printTxt(context, 'A', 'B');
// }
//
// void printColorBlock(inout Context context, float4 color)
// void printColorDisc(inout Context context, float4 color)
// void printTxt(inout Context context, uint a, uint b = 0, uint c = 0, uint d = 0, uint e = 0, uint f = 0)
// void printLF(inout Context context)
// void printInt(inout Context context, int value)
// void printHex(inout Context context, uint value)
// void printFloat(inout Context context, float value)



struct ContextScatter
{
	// RGBA, alpha 1 is assumed to be opaque
	float4 textColor;
	// in pixels
	int2 pxCursor;
	// window
	int2 pxLeftTop;
	// 1/2/3/4
	int scale;

	void init(int2 inpxLeftTop = int2(0, 0))
	{
		// white, opaque
		textColor = float4(1, 1, 1, 1);
		pxLeftTop = inpxLeftTop;
		pxCursor = pxLeftTop;
		scale = 1;
	}

	// todo: consider member functions
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

	// todo: consider member functions
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

// don't use directly, use printTxt() instead
void printCharacter(inout ContextGather context, uint ascii)
{
	int2 pxLocal = (uint2)(context.pxPos - context.pxCursor) / context.scale;

	if(fontLookup(ascii, pxLocal))
		context.dstColor = context.textColor;

	context.pxCursor.x += 8 * context.scale;
}

void printTxt(inout ContextGather context, uint a, uint b = 0, uint c = 0, uint d = 0, uint e = 0, uint f = 0)
{
	if (a) printCharacter(context, a);
	if (b) printCharacter(context, b);
	if (c) printCharacter(context, c);
	if (d) printCharacter(context, d);
	if (e) printCharacter(context, e);
	if (f) printCharacter(context, f);
}

// block in a 8x8 character
void printColorBlock(inout ContextGather context, float4 color)
{
	float2 pxLocal = (float2)(context.pxPos - context.pxCursor) / context.scale - float2(3.5f, 3.5f);

	float mask = saturate(4 - max(abs(pxLocal.x), abs(pxLocal.y)));

//	context.dstColor = lerp(context.dstColor, float4(color.rgb, 1), color.a * mask);
	if(mask)
		context.dstColor = color;


	context.pxCursor.x += 8 * context.scale;
}

void drawColorCircle(inout ContextGather context, float2 center, float radius, float4 color)
{
	float2 pxLocal = (float2)(context.pxPos - center);

	float d = radius - length(pxLocal);
	float mask = saturate(d + 1) - saturate(d);

//	context.dstColor = lerp(context.dstColor, float4(color.rgb, 1), color.a * mask);
	if(mask > 0.5f)
		context.dstColor = color;
}

void drawColorCrosshair(inout ContextGather context, int2 center, int radius, float4 color)
{
	int2 pxLocal = abs(context.pxPos - center);
	int dist = max(pxLocal.x, pxLocal.y);

	if((pxLocal.x == 0 || pxLocal.y == 0) && dist <= radius)
		context.dstColor = color;
}

// circle in a 8x8 character
void printColorDisc(inout ContextGather context, float4 color)
{
	float2 pxLocal = (float2)(context.pxPos - context.pxCursor) / context.scale - float2(3.5f, 3.5f);

	float mask = saturate(4 - length(pxLocal));

//	context.dstColor = lerp(context.dstColor, float4(color.rgb, 1), color.a * mask);
	if(mask)
		context.dstColor = color;

	context.pxCursor.x += 8 * context.scale;
}

void printLF(inout ContextGather context)
{
	context.pxCursor.x = context.pxLeftTop.x;
	context.pxCursor.y += 8 * context.scale;
}

// @param value e.g. 123, 0
void printInt(inout ContextGather context, int value)
{
	// leading '-'
	if (value < 0)
	{
		printCharacter(context, '-');
		value = -value;
	}
	if (value == 0)
	{
		printCharacter(context, '0');
		return;
	}
	// move to right depending on number length
	{
		uint tmp = (uint)value;
		while (tmp)
		{
			context.pxCursor.x += 8 * context.scale;
			tmp /= 10;
		}
	}
	// digits
	{
		int backup = context.pxCursor.x;
		uint tmp = (uint)value;
		while (tmp)
		{
			// 0..9
			uint digit = tmp % 10;
			tmp /= 10;
			// go backwards
			context.pxCursor.x -= 8 * context.scale;
			printCharacter(context, '0' + digit);
			// counter +=8 from printCharacter ()
			context.pxCursor.x -= 8 * context.scale;
		}
		context.pxCursor.x = backup;
	}
}

// print hexadecimal e.g. "0000aa34"
// @param value 32bit e.g. 0x123, 0xff00
void printHex(inout ContextGather context, uint value)
{
	// 8 nibbles
	for(int i = 7; i >= 0; --i)
	{
		// 0..15
		uint nibble = (value >> (i * 4)) & 0xf;
		uint start = (nibble < 10) ? '0' : ('A' - 10);
		printCharacter(context, start + nibble);
	}
}

// @param output e.g. g_output from RWTexture2D<float3> g_output : register(u0, space0);
// @param pos in pixels from left top, left top of the printout
// @param value
void printFloat(inout ContextGather context, float value)
{
	printInt(context, (int)value);
	float fractional = frac(abs(value));

	printCharacter(context, '.');

	uint digitCount = 3;

	// todo: unit tests, this is likely wrong at lower precision

	// fractional digits
	for(uint i = 0; i < digitCount; ++i)
	{
		fractional *= 10;
		// 0..9
		uint digit = (uint)(fractional);
		fractional = frac(fractional);
		printCharacter(context, '0' + digit);
	}
}

// -------------------------------------------------------------------------------------

// 
void onGfxForAllScatter(int2 pxPos, float4 color);

void printCharacter(inout ContextScatter context, uint ascii)
{
	[loop] for(int y = 0; y < 8 * context.scale; ++y)
	[loop] for(int x = 0; x < 8 * context.scale; ++x)
		if(fontLookup(ascii, int2(x, y) / context.scale))
			onGfxForAllScatter(context.pxCursor + int2(x, y), context.textColor);

	context.pxCursor.x += 8 * context.scale;
}

void printTxt(inout ContextScatter context, uint a, uint b = 0, uint c = 0, uint d = 0, uint e = 0, uint f = 0)
{
	if (a) printCharacter(context, a);
	if (b) printCharacter(context, b);
	if (c) printCharacter(context, c);
	if (d) printCharacter(context, d);
	if (e) printCharacter(context, e);
	if (f) printCharacter(context, f);
}

// block in a 8x8 character
void printColorBlock(inout ContextScatter context, float4 color)
{
	[loop] for(int y = 0; y < 8 * context.scale; ++y)
	[loop] for(int x = 0; x < 8 * context.scale; ++x)
	{
		float2 pxLocal = (float2(x, y) ) / context.scale - float2(3.5f, 3.5f);

		float mask = saturate(4 - max(abs(pxLocal.x), abs(pxLocal.y)));

		if(mask)
			onGfxForAllScatter(context.pxCursor + int2(x,y), color);
	}

	context.pxCursor.x += 8 * context.scale;
}

// circle in a 8x8 character
void printColorDisc(inout ContextScatter context, float4 color)
{
	[loop] for(int y = 0; y < 8 * context.scale; ++y)
	[loop] for(int x = 0; x < 8 * context.scale; ++x)
	{
		float2 pxLocal = (float2(x, y) ) / context.scale - float2(3.5f, 3.5f);

		float mask = saturate(4 - length(pxLocal));

		if(mask)
			onGfxForAllScatter(context.pxCursor + int2(x,y), color);
	}

	context.pxCursor.x += 8 * context.scale;
}

void printLF(inout ContextScatter context)
{
	context.pxCursor.x = context.pxLeftTop.x;
	context.pxCursor.y += 8 * context.scale;
}

// @param value e.g. 123, 0
void printInt(inout ContextScatter context, int value)
{
	// leading '-'
	if (value < 0)
	{
		printCharacter(context, '-');
		value = -value;
	}
	if (value == 0)
	{
		printCharacter(context, '0');
		return;
	}
	// move to right depending on number length
	{
		uint tmp = (uint)value;
		while (tmp)
		{
			context.pxCursor.x += 8 * context.scale;
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
			context.pxCursor.x -= 8 * context.scale;
			printCharacter(context, '0' + digit);
			// counter +=8 from printCharacter ()
			context.pxCursor.x -= 8 * context.scale;
		}
	}
}

// print hexadecimal e.g. "0000aa34"
// @param value 32bit e.g. 0x123, 0xff00
void printHex(inout ContextScatter context, uint value)
{
	// 8 nibbles
	for(int i = 7; i >= 0; --i)
	{
		// 0..15
		uint nibble = (value >> (i * 4)) & 0xf;
		uint start = (nibble < 10) ? '0' : ('A' - 10);
		printCharacter(context, start + nibble);
	}
}

// @param output e.g. g_output from RWTexture2D<float3> g_output : register(u0, space0);
// @param pos in pixels from left top, left top of the printout
// @param value
void printFloat(inout ContextScatter context, float value)
{
	printInt(context, (int)value);
	float fractional = frac(abs(value));

	printCharacter(context, '.');

	uint digitCount = 3;

	// todo: unit tests, this is likely wrong at lower precision

	// fractional digits
	for(uint i = 0; i < digitCount; ++i)
	{
		fractional *= 10;
		// 0..9
		uint digit = (uint)(fractional);
		fractional = frac(fractional);
		printCharacter(context, '0' + digit);
	}
}
