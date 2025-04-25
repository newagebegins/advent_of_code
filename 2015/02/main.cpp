#include <assert.h>
#include <stdio.h>

struct Dimensions
{
	int l, w, h;
};

static bool operator==(const Dimensions& a, const Dimensions& b)
{
	return a.l == b.l &&
		   a.w == b.w &&
		   a.h == b.h;
}

static int eatNumber(const char** p)
{
	int result = 0;
	bool keepGoing = true;
	while (keepGoing)
	{
		char c = **p;
		switch (c)
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			{
				int digit = c - '0';
				result = result*10 + digit;
				++(*p);
				break;
			}
			default:
			{
				keepGoing = false;
				break;
			}
		}
	}
	return result;
}

static void eatX(const char** p)
{
	assert(**p == 'x');
	++(*p);
}

static Dimensions parseDimensions(const char* str)
{
	int l = eatNumber(&str);
	eatX(&str);
	int w = eatNumber(&str);
	eatX(&str);
	int h = eatNumber(&str);
	return { l, w, h };
}

static int min(int a, int b, int c)
{
	int result;
	if (a < b)
	{
		if (a < c)
		{
			result = a;
		}
		else
		{
			// c <= a < b
			result = c;
		}
	}
	else
	{
		// b <= a
		if (b < c)
		{
			result = b;
		}
		else
		{
			// c <= b <= a
			result = c;
		}
	}
	return result;
}

static int max(int a, int b, int c)
{
	int result;
	if (a > b)
	{
		if (a > c)
		{
			result = a;
		}
		else
		{
			// b < a <= c
			result = c;
		}
	}
	else
	{
		// a <= b
		if (b > c)
		{
			result = b;
		}
		else
		{
			// a <= b <= c
			result = c;
		}
	}
	return result;
}

static int getWrappingPaperFeet(int l, int w, int h)
{
	int lw = l*w;
	int wh = w*h;
	int hl = h*l;
	int smallest = min(lw, wh, hl);
	int result = 2*lw + 2*wh + 2*hl + smallest;
	return result;
}

static int getRibbonFeet(int l, int w, int h)
{
	int wrap = 2*l + 2*w + 2*h - 2*max(l, w, h);
	int bow = l * w * h;
	return wrap + bow;
}

int main()
{
	assert(parseDimensions("2x3x4") == Dimensions(2, 3, 4));
	assert(parseDimensions("1x1x10") == Dimensions(1, 1, 10));

	assert(getWrappingPaperFeet(2, 3, 4) == 58);
	assert(getWrappingPaperFeet(1, 1, 10) == 43);

	assert(getRibbonFeet(2, 3, 4) == 34);
	assert(getRibbonFeet(1, 1, 10) == 14);
	
	FILE* file;
	fopen_s(&file, "input.txt", "r");
	assert(file);
	int c;
	constexpr int maxLen = 32;
	char buf[maxLen];
	int len = 0;
	int wrappingPaperFeetTotal = 0;
	int ribbonFeetTotal = 0;
	while ((c = fgetc(file)) != EOF)
	{
		if (c == '\n')
		{
			buf[len] = '\0';
			Dimensions dims = parseDimensions(buf);
			wrappingPaperFeetTotal += getWrappingPaperFeet(dims.l, dims.w, dims.h);
			ribbonFeetTotal += getRibbonFeet(dims.l, dims.w, dims.h);
			len = 0;
		}
		else
		{
			buf[len++] = (char)c;
		}
	}
	assert(wrappingPaperFeetTotal == 1598415);
	printf("%d\n", wrappingPaperFeetTotal);
	printf("%d\n", ribbonFeetTotal);
	return 0;
}
