//
// TODO:
//  [x] Add a disk image to qemu
//  [ ] Read disk TOC and content of a file
//  [ ] Allocate, use, and free some memory
//  [ ] Write some basic graphics junk (bresenham etc)


#include "kernel.h"
#include "Graphics.h"
#include <circle/usb/usbkeyboard.h>
#include <circle/string.h>
#include <circle/util.h>
#include <circle/alloc.h>
#include <assert.h>

// SD card device name when running under QEMU:
#define PARTITION_QEMU    "umsd1"
#define PARTITION_RASPI   "emmc1-1"

static const char FromKernel[] = "kernel";

CKernel *CKernel::s_pThis = 0;

CKernel::CKernel (void)
:	m_Screen (1280, 720),
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

void CKernel::DrawLogo (void) {
    auto col_white = COLOR16(31,31,31);
    // LOGO
    Graphics::Line(m_Screen, 0,   18,  0,   0, col_white);
    Graphics::Line(m_Screen, 18,  54,  0,   72, col_white);
    Graphics::Line(m_Screen, 54,  91,  72,  0, col_white);
    Graphics::Line(m_Screen, 91,  109, 0,   0, col_white);
    Graphics::Line(m_Screen, 109, 109, 0,   196, col_white);
    Graphics::Line(m_Screen, 109, 87,  196, 196, col_white);
    Graphics::Line(m_Screen, 87,  87,  196, 50, col_white);
    Graphics::Line(m_Screen, 87,  62,  50,  101, col_white);
    Graphics::Line(m_Screen, 62,  47,  101, 101, col_white);
    Graphics::Line(m_Screen, 47,  22,  101, 50, col_white);
    Graphics::Line(m_Screen, 22,  22,  50,  196, col_white);
    Graphics::Line(m_Screen, 22,  0,   196, 196, col_white);
    Graphics::Line(m_Screen, 0,   0,   196, 0, col_white);

    Graphics::FloodFill(m_Screen, 89, 55, COLOR16 (7, 15, 31));

    // E
    Graphics::Line(m_Screen, 146, 255, 0,   0, col_white);
    Graphics::Line(m_Screen, 255, 255, 0,   21, col_white);
    Graphics::Line(m_Screen, 255, 167, 21,  21, col_white);
    Graphics::Line(m_Screen, 167, 167, 21,  87, col_white);
    Graphics::Line(m_Screen, 167, 233, 87,  87, col_white);
    Graphics::Line(m_Screen, 233, 233, 87,  109, col_white);
    Graphics::Line(m_Screen, 167, 233, 109, 109, col_white);
    Graphics::Line(m_Screen, 167, 167, 109, 174, col_white);
    Graphics::Line(m_Screen, 167, 255, 174, 174, col_white);
    Graphics::Line(m_Screen, 255, 255, 174, 196, col_white);
    Graphics::Line(m_Screen, 255, 146, 196, 196, col_white);
    Graphics::Line(m_Screen, 146, 146, 196, 0, col_white);

    Graphics::FloodFill(m_Screen, 148, 2, COLOR16 (31, 15, 7));

    // C
    Graphics::Line(m_Screen, 291, 327, 36,   0, col_white);
    Graphics::Line(m_Screen, 327, 364, 0,    0, col_white);
    Graphics::Line(m_Screen, 364, 400, 0,   36, col_white);
    Graphics::Line(m_Screen, 400, 386, 36,  50, col_white);
    Graphics::Line(m_Screen, 386, 357,  50,  21, col_white);
    Graphics::Line(m_Screen, 357, 335,  21,  21, col_white);
    Graphics::Line(m_Screen, 335, 313,  21,  43, col_white);
    Graphics::Line(m_Screen, 313, 313,  43, 152, col_white);
    Graphics::Line(m_Screen, 313, 335, 152, 174, col_white);
    Graphics::Line(m_Screen, 335, 357, 174, 174, col_white);
    Graphics::Line(m_Screen, 357, 386, 174, 145, col_white);
    Graphics::Line(m_Screen, 386, 400, 145, 159, col_white);
    Graphics::Line(m_Screen, 400, 364, 159, 196, col_white);
    Graphics::Line(m_Screen, 364, 327, 196, 196, col_white);
    Graphics::Line(m_Screen, 327, 291, 196, 159, col_white);
    Graphics::Line(m_Screen, 291, 291, 159,  36, col_white);

    Graphics::FloodFill(m_Screen, 292, 37, COLOR16 (31, 7, 15));

    // S
    Graphics::Line(m_Screen, 437, 473,  36,   0, col_white);
    Graphics::Line(m_Screen, 473, 509,   0,   0, col_white);
    Graphics::Line(m_Screen, 509, 546,   0,  36, col_white);
    Graphics::Line(m_Screen, 546, 531,  36,  50, col_white);
    Graphics::Line(m_Screen, 531, 502,  50,  21, col_white);
    Graphics::Line(m_Screen, 502, 480,  21,  21, col_white);
    Graphics::Line(m_Screen, 480, 459,  21,  43, col_white);
    Graphics::Line(m_Screen, 459, 459,  43,  65, col_white);
    Graphics::Line(m_Screen, 459, 480,  65,  87, col_white);
    Graphics::Line(m_Screen, 480, 509,  87,  87, col_white);
    Graphics::Line(m_Screen, 509, 546,  87, 123, col_white);
    Graphics::Line(m_Screen, 546, 546, 123, 159, col_white);
    Graphics::Line(m_Screen, 546, 509, 159, 196, col_white);
    Graphics::Line(m_Screen, 509, 473, 196, 196, col_white);
    Graphics::Line(m_Screen, 473, 437, 196, 159, col_white);
    Graphics::Line(m_Screen, 437, 451, 159, 145, col_white);
    Graphics::Line(m_Screen, 451, 480, 145, 174, col_white);
    Graphics::Line(m_Screen, 480, 502, 174, 174, col_white);
    Graphics::Line(m_Screen, 502, 524, 174, 152, col_white);
    Graphics::Line(m_Screen, 524, 524, 152, 130, col_white);
    Graphics::Line(m_Screen, 524, 502, 130, 109, col_white);
    Graphics::Line(m_Screen, 502, 473, 109, 109, col_white);
    Graphics::Line(m_Screen, 473, 437, 109,  72, col_white);
    Graphics::Line(m_Screen, 437, 437,  72,  36, col_white);

    Graphics::FloodFill(m_Screen, 440, 36, COLOR16 (7, 31, 15));

    // Draw some sample shapes
#if 0
    Graphics::Line(m_Screen, 50, 50, 50, 0, COLOR16 (15, 15, 0));
    Graphics::Line(m_Screen, 50, 75, 50, 0, COLOR16 (0, 15, 15));
    Graphics::Circle(m_Screen, 50, 50 , 25, COLOR16(15,0,15));
    Graphics::Ellipse(m_Screen, 150, 50, 25, 50, COLOR16(7,7,15));
    Graphics::Ellipse(m_Screen, 150, 50, 50, 25, COLOR16(7,7,15));
    Graphics::Rectangle(m_Screen, 150, 250, 150, 155, COLOR16(15,7,7));
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


void CKernel::ReadFileSystem(void) {

    // Mount file system
    CDevice *pPartition = m_DeviceNameService.GetDevice (PARTITION_QEMU, TRUE);
    if (pPartition == 0)
    {
        m_DeviceNameService.ListDevices (&m_Screen);
        m_Logger.Write (FromKernel, LogPanic, "Partition not found: %s", PARTITION_QEMU);
    }

    if (!m_FileSystem.Mount (pPartition))
    {
        m_Logger.Write (FromKernel, LogPanic, "Cannot mount partition: %s", PARTITION_QEMU);
    }

    // Show contents of root directory
    TDirentry Direntry;
    TFindCurrentEntry CurrentEntry;
    unsigned nEntry = m_FileSystem.RootFindFirst (&Direntry, &CurrentEntry);
    for (unsigned i = 0; nEntry != 0; i++)
    {
        if (!(Direntry.nAttributes & FS_ATTRIB_SYSTEM))
        {
            CString FileName;
            FileName.Format ("%-14s", Direntry.chTitle);

            m_Screen.Write ((const char *) FileName, FileName.GetLength ());

            if (i % 5 == 4)
            {
                m_Screen.Write ("\n", 1);
            }
        }

        nEntry = m_FileSystem.RootFindNext (&Direntry, &CurrentEntry);
    }
    m_Screen.Write ("\n", 1);

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

    ReadFileSystem();


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
