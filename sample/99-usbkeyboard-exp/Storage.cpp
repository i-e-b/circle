//
// Created by iain on 05/11/18.
//

#include "Storage.h"

#define PARTITION	"emmc1" //-1"

static const char FromStorage[] = "storage";

Storage::Storage(CInterruptSystem *interrupt, CTimer *timer, CLogger* logger, CDeviceNameService* dns)
        : m_DeviceNameService(dns)
        , m_Interrupt(interrupt)
        , m_Timer(timer)
        , m_Logger(logger)
        , m_EMMC (interrupt, timer, &m_ActLED)
{

    assert (interrupt != 0);
    assert (timer != 0);
}

Storage::~Storage(void) {

}

boolean Storage::Initialize(void) {

    boolean bOK = TRUE;

    if (bOK) {
        bOK = m_EMMC.Initialize ();
    }

    if (bOK) {
        // Try to mount file system
        m_DeviceNameService->ListDevices(m_Logger);

        m_Partition = m_DeviceNameService->GetDevice(PARTITION, TRUE);
        if (m_Partition == 0) {
            m_Logger->Write(FromStorage, LogPanic, "Partition not found: %s", PARTITION);
            return FALSE;
        }

        if (!m_FileSystem.Mount(m_Partition)) {
            m_Logger->Write(FromStorage, LogPanic, "Cannot mount partition: %s", PARTITION);
            return FALSE;
        }
    }


    return bOK;
}

void Storage::ListDirectories(CScreenDevice* screen) {
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

            screen->Write ((const char *) FileName, FileName.GetLength ());

            if (i % 5 == 4)
            {
                screen->Write ("\n", 1);
            }
        }

        nEntry = m_FileSystem.RootFindNext (&Direntry, &CurrentEntry);
    }
    screen->Write ("\n", 1);

}
