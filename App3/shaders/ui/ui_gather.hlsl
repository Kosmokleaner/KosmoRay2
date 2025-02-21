// pixel shader or compute shader looping through all pixels

// API
// 
// {
//   struct ContextGather ui;
//   ui.init(inpxPos, pxLeftTop);
//   ui.printTxt('A', 'B');
//   Output[ui.pxPos] = ui.dstColor;
// }

#include "ui_font.hlsl"

struct ContextGather
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

	// in pixels
	int2 pxPos;
	// RGBA, alpha 1 is assumed to be opaque
	float4 dstColor;
};
void init(out ContextGather context, int2 inPxPos, int2 inpxLeftTop = int2(0, 0));
// e.g. ui.printTxt('I', ' ', 'a', 'm');
// @param a ascii character or 0
void printTxt(inout ContextGather context, uint a, uint b = 0, uint c = 0, uint d = 0, uint e = 0, uint f = 0);
// jump to next line
void printLF(inout ContextGather context);
// @param value e.g. 123, 0
void printInt(inout ContextGather context, int value);
// print hexadecimal e.g. "0000aa34"
// @param value 32bit e.g. 0x123, 0xff00
void printHex(inout ContextGather context, uint value);
// @param output e.g. g_output from RWTexture2D<float3> g_output : register(u0, space0);
// @param pos in pixels from left top, left top of the printout
// @param value
void printFloat(inout ContextGather context, float value);
// block in a 8x8 character
void printBlock(inout ContextGather context, float4 color);
// circle in a 8x8 character
// @return mouseOver
bool printDisc(inout ContextGather context, float4 color);
//
void drawCircle(inout ContextGather context, float2 pxCenter, float pxRadius, float4 color, float pxThickness = 1.0f);
//
void drawCrosshair(inout ContextGather context, float2 pxCenter, float pxRadius, float4 color, float pxThickness = 1.0f);
//
void drawLine(inout ContextGather context, float2 pxBegin, float2 pxEnd, float4 color, float pxThickness = 1.0f);
// don't use directly
void printCharacter(inout ContextGather context, uint ascii);

// ----------------------------------------------------------------------

void init(out ContextGather context, int2 inPxPos, int2 inpxLeftTop)
{
	// white, opaque 
	context.textColor = float4(1, 1, 1, 1); 
	context.pxLeftTop = inpxLeftTop; 
	context.pxCursor = context.pxLeftTop; 
	context.scale = 1;
	context.mouseXY = int2(-100, -100); 

	context.pxPos = inPxPos;
	// see through
	context.dstColor = float4(0, 0, 0, 0);
}

void printCharacter(inout ContextGather context, uint ascii)
{
	int2 pxLocal = (uint2)(context.pxPos - context.pxCursor) / context.scale;

	if(fontLookup(ascii, pxLocal))
		context.dstColor = context.textColor;

	context.pxCursor.x += 8 * context.scale;
}

void printTxt(inout ContextGather context, uint a, uint b, uint c, uint d, uint e, uint f)
{
	if (a) printCharacter(context, a);
	if (b) printCharacter(context, b);
	if (c) printCharacter(context, c);
	if (d) printCharacter(context, d);
	if (e) printCharacter(context, e);
	if (f) printCharacter(context, f);
}

void printLF(inout ContextGather context)
{
	context.pxCursor.x = context.pxLeftTop.x;
	context.pxCursor.y += 8 * context.scale;
}

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

void printBlock(inout ContextGather context, float4 color)
{
	float2 pxLocal = (float2)(context.pxPos - context.pxCursor) / context.scale - float2(3.5f, 3.5f);

	float mask = saturate(4 - max(abs(pxLocal.x), abs(pxLocal.y)));

//	dstColor = lerp(dstColor, float4(color.rgb, 1), color.a * mask);
	if(mask)
		context.dstColor = color;

	context.pxCursor.x += 8 * context.scale;
}

void drawCircle(inout ContextGather context, float2 pxCenter, float pxRadius, float4 color, float pxThickness)
{
	float r = pxThickness * 0.5f;
	float2 pxLocal = (float2)(context.pxPos - pxCenter);

	float len = length(pxLocal);

	if(len > pxRadius - r && len < pxRadius + r)
		context.dstColor = color;
}

void drawCrosshair(inout ContextGather context, float2 pxCenter, float pxRadius, float4 color, float pxThickness)
{
	float2 h = float2(pxRadius, 0);
	float2 v = float2(0, pxRadius);

	drawLine(context, pxCenter - h , pxCenter + h, color, pxThickness);
	drawLine(context, pxCenter - v, pxCenter + v, color, pxThickness);

//	int2 pxLocal = abs(pxPos - pxCenter);
//	int dist = max(pxLocal.x, pxLocal.y);
//	if((pxLocal.x == 0 || pxLocal.y == 0) && dist <= pxRadius)
//		dstColor = color;
}

void drawLine(inout ContextGather context, float2 pxBegin, float2 pxEnd, float4 color, float pxThickness)
{
	float r = pxThickness * 0.5f;
	float2 delta = pxEnd - pxBegin;
	float len = length(delta);
	if(len > 0.01f)
	{
		float2 tangent = delta / len;
		float2 normal = float2(tangent.y, -tangent.x);
		float2 local = context.pxPos - pxBegin;
		float2 uv = float2(dot(local, tangent), dot(local, normal));

		if(abs(uv.y) < r && uv.x > -r && uv.x < len + r)
			context.dstColor = color;
	}
}

bool printDisc(inout ContextGather context, float4 color)
{
	float2 pxLocal = (float2)(context.pxPos - context.pxCursor) / context.scale - float2(3.5f, 3.5f);

	float mask = saturate(4 - length(pxLocal));

//	dstColor = lerp(stColor, float4(color.rgb, 1), color.a * mask);
	if(mask)
		context.dstColor = color;

	bool mouseOver = length((float2)(context.mouseXY - context.pxCursor) / context.scale - float2(3.5f, 3.5f)) < 4;

	context.pxCursor.x += 8 * context.scale;

	return mouseOver;
}
