//
// TODO:
//  [ ] Add a disk image to qemu
//  [ ] Read disk TOC and content of a file
//  [ ] Allocate, use, and free some memory
//  [ ] Write some basic graphics junk (bresenham etc)


#include "kernel.h"
#include <circle/usb/usbkeyboard.h>
#include <circle/string.h>
#include <circle/util.h>
#include <circle/alloc.h>
#include <assert.h>

static const char FromKernel[] = "kernel";

CKernel *CKernel::s_pThis = 0;

CKernel::CKernel (void)
:	m_Screen (1024, 720),
	m_Timer (&m_Interrupt),
	m_Logger (/*m_Options.GetLogLevel ()*/ TLogSeverity::LogError, &m_Timer),
	m_DWHCI (&m_Interrupt, &m_Timer),
	m_ShutdownMode (ShutdownNone)
{
	s_pThis = this;
}

CKernel::~CKernel (void)
{
	s_pThis = 0;
}

static void Line(CScreenDevice &dev, int x0, int x1, int y0, int y1, TScreenColor color) {
    int dx = x1-x0, sx = x0<x1 ? 1 : -1;
    int dy = y1-y0, sy = y0<y1 ? 1 : -1;

    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;

    int err = (dx>dy ? dx : -dy) >> 1, e2;

    for(;;){
        dev.SetPixel (x0, y0, color);
        if (x0==x1 && y0==y1) break;
        e2 = err;
        if (e2 >-dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }
}
void Circle (CScreenDevice &dev, int xc, int yc, int radius, TScreenColor color)
{
    int x = 0;
    int y = radius;
    int delta = 2 - 2 * radius;
    int error = 0;

    while(y >= 0)
    {
        dev.SetPixel(xc + x, yc + y, color);
        dev.SetPixel(xc - x, yc + y, color);
        dev.SetPixel(xc + x, yc - y, color);
        dev.SetPixel(xc - x, yc - y, color);

        error = 2 * (delta + y) - 1;
        if(delta < 0 && error <= 0) {
            ++x;
            delta += 2 * x + 1;
            continue;
        }
        error = 2 * (delta - x) - 1;
        if(delta > 0 && error > 0) {
            --y;
            delta += 1 - 2 * y;
            continue;
        }
        ++x;
        delta += 2 * (x - y);
        --y;
    }
}

void Ellipse (CScreenDevice &dev, int xc, int yc, int width, int height, TScreenColor color)
{
    int a2 = width * width;
    int b2 = height * height;
    int fa2 = 4 * a2, fb2 = 4 * b2;
    int x, y, sigma;

    // Top and bottom
    for (x = 0, y = height, sigma = 2*b2+a2*(1-2*height); b2*x <= a2*y; x++) {
        dev.SetPixel(xc + x, yc + y, color);
        dev.SetPixel(xc - x, yc + y, color);
        dev.SetPixel(xc + x, yc - y, color);
        dev.SetPixel(xc - x, yc - y, color);
        if (sigma >= 0)
        {
            sigma += fa2 * (1 - y);
            y--;
        }
        sigma += b2 * ((4 * x) + 6);
    }

    // Left and right
    for (x = width, y = 0, sigma = 2*a2+b2*(1-2*width); a2*y <= b2*x; y++) {
        dev.SetPixel(xc + x, yc + y, color);
        dev.SetPixel(xc - x, yc + y, color);
        dev.SetPixel(xc + x, yc - y, color);
        dev.SetPixel(xc - x, yc - y, color);
        if (sigma >= 0)
        {
            sigma += fb2 * (1 - x);
            x--;
        }
        sigma += a2 * ((4 * y) + 6);
    }
}

void Rectangle (CScreenDevice &dev, int x0, int x1, int y0, int y1, TScreenColor color) {
    for (int nPosX = x0; nPosX < x1; nPosX++)
    {
        dev.SetPixel (nPosX, y0, color);
        dev.SetPixel (nPosX, y1, color);
    }
    for (int nPosY = y0; nPosY <= y1; nPosY++)
    {
        dev.SetPixel (x0, nPosY, color);
        dev.SetPixel (x1, nPosY, color);
    }
}


void push(int* x_stk, int* y_stk, int &slen, int nx, int ny) {
    if (slen > 100) return;
    x_stk[slen] = nx;
    y_stk[slen] = ny;
    slen++;
}

int pop(int* x_stk, int* y_stk, int &slen, int &ox, int &oy) {
    if (slen < 1) return 0;
    slen--;
    ox = x_stk[slen];
    oy = y_stk[slen];
    return 1;
}

void FloodFill(CScreenDevice &dev, int x, int y, TScreenColor newColor)
{
    TScreenColor oldColor = dev.GetPixel(x,y);
    if (newColor == oldColor) return;

    int x1;
    bool spanAbove, spanBelow;

    int x_stk[101];
    int y_stk[101];
    int slen = 0;

    int w = dev.GetWidth();
    int h = dev.GetHeight();
    if (w < 100) w = 640;
    if (h < 100) h = 360;

    push(x_stk, y_stk, slen, x, y);
    while(pop(x_stk, y_stk, slen, x, y))
    {
        x1 = x;
        while(x1 > 0 && dev.GetPixel(x1,y) == oldColor) x1--;
        x1++;
        spanAbove = spanBelow = 0;
        while(x1 < w && dev.GetPixel(x1,y) == oldColor)
        {
            dev.SetPixel(x1,y,newColor);
            if(!spanAbove && y > 0 && dev.GetPixel(x1,y-1) == oldColor){
                push(x_stk, y_stk, slen, x1, y - 1);
                spanAbove = 1;
            }
            else if(spanAbove && y > 0 && dev.GetPixel(x1,y-1) != oldColor) {
                spanAbove = 0;
            }
            if(!spanBelow && y < h - 1 && dev.GetPixel(x1,y+1) == oldColor){

                push(x_stk, y_stk, slen, x1, y + 1);
                spanBelow = 1;
            }
            else if(spanBelow && y < h - 1 && dev.GetPixel(x1,y+1) != oldColor) {
                spanBelow = 0;
            }
            x1++;
        }
    }
}

void CKernel::DrawLogo (void) {
    auto col_white = COLOR16(31,31,31);
    // LOGO
    Line(m_Screen, 0,   18,  0,   0, col_white);
    Line(m_Screen, 18,  54,  0,   72, col_white);
    Line(m_Screen, 54,  91,  72,  0, col_white);
    Line(m_Screen, 91,  109, 0,   0, col_white);
    Line(m_Screen, 109, 109, 0,   196, col_white);
    Line(m_Screen, 109, 87,  196, 196, col_white);
    Line(m_Screen, 87,  87,  196, 50, col_white);
    Line(m_Screen, 87,  62,  50,  101, col_white);
    Line(m_Screen, 62,  47,  101, 101, col_white);
    Line(m_Screen, 47,  22,  101, 50, col_white);
    Line(m_Screen, 22,  22,  50,  196, col_white);
    Line(m_Screen, 22,  0,   196, 196, col_white);
    Line(m_Screen, 0,   0,   196, 0, col_white);

    FloodFill(m_Screen, 89, 55, COLOR16 (7, 15, 31));

    // E
    Line(m_Screen, 146, 255, 0,   0, col_white);
    Line(m_Screen, 255, 255, 0,   21, col_white);
    Line(m_Screen, 255, 167, 21,  21, col_white);
    Line(m_Screen, 167, 167, 21,  87, col_white);
    Line(m_Screen, 167, 233, 87,  87, col_white);
    Line(m_Screen, 233, 233, 87,  109, col_white);
    Line(m_Screen, 167, 233, 109, 109, col_white);
    Line(m_Screen, 167, 167, 109, 174, col_white);
    Line(m_Screen, 167, 255, 174, 174, col_white);
    Line(m_Screen, 255, 255, 174, 196, col_white);
    Line(m_Screen, 255, 146, 196, 196, col_white);
    Line(m_Screen, 146, 146, 196, 0, col_white);

    FloodFill(m_Screen, 148, 2, COLOR16 (31, 15, 7));

    // C
    Line(m_Screen, 291, 327, 36,   0, col_white);
    Line(m_Screen, 327, 364, 0,    0, col_white);
    Line(m_Screen, 364, 400, 0,   36, col_white);
    Line(m_Screen, 400, 386, 36,  50, col_white);
    Line(m_Screen, 386, 357,  50,  21, col_white);
    Line(m_Screen, 357, 335,  21,  21, col_white);
    Line(m_Screen, 335, 313,  21,  43, col_white);
    Line(m_Screen, 313, 313,  43, 152, col_white);
    Line(m_Screen, 313, 335, 152, 174, col_white);
    Line(m_Screen, 335, 357, 174, 174, col_white);
    Line(m_Screen, 357, 386, 174, 145, col_white);
    Line(m_Screen, 386, 400, 145, 159, col_white);
    Line(m_Screen, 400, 364, 159, 196, col_white);
    Line(m_Screen, 364, 327, 196, 196, col_white);
    Line(m_Screen, 327, 291, 196, 159, col_white);
    Line(m_Screen, 291, 291, 159,  36, col_white);

    FloodFill(m_Screen, 292, 37, COLOR16 (31, 7, 15));

    // S
    Line(m_Screen, 437, 473,  36,   0, col_white);
    Line(m_Screen, 473, 509,   0,   0, col_white);
    Line(m_Screen, 509, 546,   0,  36, col_white);
    Line(m_Screen, 546, 531,  36,  50, col_white);
    Line(m_Screen, 531, 502,  50,  21, col_white);
    Line(m_Screen, 502, 480,  21,  21, col_white);
    Line(m_Screen, 480, 459,  21,  43, col_white);
    Line(m_Screen, 459, 459,  43,  65, col_white);
    Line(m_Screen, 459, 480,  65,  87, col_white);
    Line(m_Screen, 480, 509,  87,  87, col_white);
    Line(m_Screen, 509, 546,  87, 123, col_white);
    Line(m_Screen, 546, 546, 123, 159, col_white);
    Line(m_Screen, 546, 509, 159, 196, col_white);
    Line(m_Screen, 509, 473, 196, 196, col_white);
    Line(m_Screen, 473, 437, 196, 159, col_white);
    Line(m_Screen, 437, 451, 159, 145, col_white);
    Line(m_Screen, 451, 480, 145, 174, col_white);
    Line(m_Screen, 480, 502, 174, 174, col_white);
    Line(m_Screen, 502, 524, 174, 152, col_white);
    Line(m_Screen, 524, 524, 152, 130, col_white);
    Line(m_Screen, 524, 502, 130, 109, col_white);
    Line(m_Screen, 502, 473, 109, 109, col_white);
    Line(m_Screen, 473, 437, 109,  72, col_white);
    Line(m_Screen, 437, 437,  72,  36, col_white);

    FloodFill(m_Screen, 440, 36, COLOR16 (7, 31, 15));

    // Draw some sample shapes
#if 0
    Line(m_Screen, 50, 50, 50, 0, COLOR16 (15, 15, 0));
    Line(m_Screen, 50, 75, 50, 0, COLOR16 (0, 15, 15));
    Circle(m_Screen, 50, 50 , 25, COLOR16(15,0,15));
    Ellipse(m_Screen, 150, 50, 25, 50, COLOR16(7,7,15));
    Ellipse(m_Screen, 150, 50, 50, 25, COLOR16(7,7,15));
    Rectangle(m_Screen, 150, 250, 150, 155, COLOR16(15,7,7));
#endif
}

boolean CKernel::Initialize (void)
{
	boolean bOK = TRUE;

	if (bOK)
	{
		bOK = m_Screen.Initialize ();
	}

    m_Screen.CursorMove(12,1);
    // this might be hurting the timer?
    DrawLogo();

	/*if (bOK)
	{
		bOK = m_Serial.Initialize (115200);
	}*/

	if (bOK)
	{
		CDevice *pTarget = m_DeviceNameService.GetDevice (m_Options.GetLogDevice (), FALSE);
		if (pTarget == 0)
		{
			pTarget = &m_Screen;
		}

		bOK = m_Logger.Initialize (pTarget);
	}

	if (bOK)
	{
		bOK = m_Interrupt.Initialize ();
	}

	if (bOK)
	{
	    // needed to drive keyboard interrupts
		bOK = m_Timer.Initialize ();
	}

	if (bOK)
	{
	    // Wake up USB...
		bOK = m_DWHCI.Initialize ();
	}

	return bOK;
}

TShutdownMode CKernel::Run (void)
{
	m_Logger.Write (FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);

	CUSBKeyboardDevice *pKeyboard = (CUSBKeyboardDevice *) m_DeviceNameService.GetDevice ("ukbd1", FALSE);
	if (pKeyboard == 0)
	{
		m_Logger.Write (FromKernel, LogError, "Keyboard not found. Plug it in and turn the computer back on");

		return ShutdownHalt;
	}


#if 1	// set to 0 to test raw mode
	pKeyboard->RegisterShutdownHandler (ShutdownHandler);
	pKeyboard->RegisterKeyPressedHandler (KeyPressedHandler);
#else
	pKeyboard->RegisterKeyStatusHandlerRaw (KeyStatusHandlerRaw);
#endif

    m_Screen.Write("Ready > ",8);


	//for (unsigned nCount = 0; m_ShutdownMode == ShutdownNone; nCount++)
	while (m_ShutdownMode == ShutdownNone)
	{
		// CUSBKeyboardDevice::UpdateLEDs() must not be called in interrupt context,
		// that's why this must be done here. This does nothing in raw mode.
		pKeyboard->UpdateLEDs ();

		//m_Screen.Rotor (0, nCount);
		//box();
		m_Timer.MsDelay (500);
	}

	return m_ShutdownMode;
}

void CKernel::KeyPressedHandler (const char *pString)
{
	assert (s_pThis != 0);
	if (*pString == 127) { // backspace
		s_pThis->m_Screen.DeleteChars(1);
		return;
	}
	s_pThis->m_Screen.Write (pString, strlen (pString));
}

void CKernel::ShutdownHandler (void)
{
	assert (s_pThis != 0);
	s_pThis->m_ShutdownMode = ShutdownReboot;
}

void CKernel::KeyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char RawKeys[6])
{
	assert (s_pThis != 0);

	CString Message;
	Message.Format ("Key status (modifiers %02X)", (unsigned) ucModifiers);

	for (unsigned i = 0; i < 6; i++)
	{
		if (RawKeys[i] != 0)
		{
			CString KeyCode;
			KeyCode.Format (" %02X", (unsigned) RawKeys[i]);

			Message.Append (KeyCode);
		}
	}

	s_pThis->m_Logger.Write (FromKernel, LogError, Message);
}
