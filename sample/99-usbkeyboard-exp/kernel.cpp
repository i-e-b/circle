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
#include <assert.h>

static const char FromKernel[] = "kernel";

CKernel *CKernel::s_pThis = 0;

CKernel::CKernel (void)
:	m_Screen (m_Options.GetWidth (), m_Options.GetHeight ()),
	m_Timer (&m_Interrupt),
	m_Logger (/*m_Options.GetLogLevel ()*/ TLogSeverity::LogError, &m_Timer),
	m_DWHCI (&m_Interrupt, &m_Timer),
	m_ShutdownMode (ShutdownNone)
{
	s_pThis = this;

	m_ActLED.Blink (2);	// show we are alive
}

CKernel::~CKernel (void)
{
	s_pThis = 0;
}

void CKernel::box (void) {

	// speed test
	auto w = m_Screen.GetWidth();
	auto h = m_Screen.GetHeight();
	auto c_red = COLOR16 (5, 0, 0);
	auto c_blue = COLOR16 (0, 0, 5);

	// draw rectangle on screen
	for (unsigned nPosX = 0; nPosX < w; nPosX++)
	{
		m_Screen.SetPixel (nPosX, 0, c_blue);
		m_Screen.SetPixel (nPosX, h-1, c_blue);
	}
	for (unsigned nPosY = 0; nPosY < h; nPosY++)
	{
		m_Screen.SetPixel (0, nPosY, c_red);
		m_Screen.SetPixel (w-1, nPosY, c_red);
	}
}

boolean CKernel::Initialize (void)
{
	boolean bOK = TRUE;

	if (bOK)
	{
		bOK = m_Screen.Initialize ();
	}

	if (bOK)
	{
		bOK = m_Serial.Initialize (115200);
	}

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
		bOK = m_Timer.Initialize ();
	}

	if (bOK)
	{
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


	box();

	m_Screen.CursorMove(10,10);
	m_Screen.Write("Hello, world",12);
	m_Screen.CursorMove(1,1);

	// Intro graphic?
	m_Screen.Write(" ____ ____ ____ ____ \n",22);
	m_Screen.Write("||M |||E |||C |||S ||\n",22);
	m_Screen.Write("||__|||__|||__|||__||\n",22);
	m_Screen.Write("|/__\\|/__\\|/__\\|/__\\|\n",22);

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

	s_pThis->m_Logger.Write (FromKernel, LogNotice, Message);
}
