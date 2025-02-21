// vertex shader, single thread in compute or pixel shader

// API
//
// {
//   struct ContextScatter ui;
//   ui.init(pxLeftTop);
//   ui.printTxt('A', 'B');
// }
// void onGfxForAllScatter(int2 pxPos, float4 color) {}


#include "ui_font.hlsl"

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
};
//
void init(out ContextScatter context, int2 inpxLeftTop = int2(0, 0));
// e.g. ui.printTxt('I', ' ', 'a', 'm');
// @param a ascii character or 0
void printTxt(inout ContextScatter context, uint a, uint b = 0, uint c = 0, uint d = 0, uint e = 0, uint f = 0);
// jump to next line
void printLF(inout ContextScatter context);
// @param value e.g. 123, 0
void printInt(inout ContextScatter context, int value);
// print hexadecimal e.g. "0000aa34"
// @param value 32bit e.g. 0x123, 0xff00
void printHex(inout ContextScatter context, uint value);
// @param output e.g. g_output from RWTexture2D<float3> g_output : register(u0, space0);
// @param pos in pixels from left top, left top of the printout
// @param value
void printFloat(inout ContextScatter context, float value);
// block in a 8x8 character
void printBlock(inout ContextScatter context, float4 color);
// circle in a 8x8 character
void printDisc(inout ContextScatter context, float4 color);
// don't use directly
void printCharacter(inout ContextScatter, uint ascii);

// ---------------------------------------------------------

void init(out ContextScatter context, int2 inpxLeftTop) 
{ 
	// white, opaque 
	context.textColor = float4(1, 1, 1, 1); 
	context.pxLeftTop = inpxLeftTop; 
	context.pxCursor = context.pxLeftTop; 
	context.scale = 1; 
	context.mouseXY = int2(-100, -100); 
} 

// implement this in your code
void onGfxForAllScatter(int2 pxPos, float4 color);

void printCharacter(inout ContextScatter context, uint ascii)
{
	[loop] for(int y = 0; y < 8 * context.scale; ++y)
	[loop] for(int x = 0; x < 8 * context.scale; ++x)
		if(fontLookup(ascii, int2(x, y) / context.scale))
			onGfxForAllScatter(context.pxCursor + int2(x, y), context.textColor);

	context.pxCursor.x += 8 * context.scale;
}

void printTxt(inout ContextScatter context, uint a, uint b, uint c, uint d, uint e, uint f)
{
	if (a) printCharacter(context, a);
	if (b) printCharacter(context, b);
	if (c) printCharacter(context, c);
	if (d) printCharacter(context, d);
	if (e) printCharacter(context, e);
	if (f) printCharacter(context, f);
}

void printLF(inout ContextScatter context)
{
	context.pxCursor.x = context.pxLeftTop.x;
	context.pxCursor.y += 8 * context.scale;
}

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

void printBlock(inout ContextScatter context, float4 color)
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

void printDisc(inout ContextScatter context, float4 color)
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
