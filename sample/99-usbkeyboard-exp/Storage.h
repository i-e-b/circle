//
// Created by iain on 05/11/18.
//

#ifndef USBKEYBOARD_STORAGE_H
#define USBKEYBOARD_STORAGE_H

#include <circle/memory.h>
#include <circle/actled.h>
#include <circle/koptions.h>
#include <circle/devicenameservice.h>
#include <circle/screen.h>
#include <circle/interrupt.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <circle/emmc.h>
#include <circle/fs/fat/fatfs.h>
#include <circle/types.h>

class Storage {

public:
    Storage(CInterruptSystem *interrupt, CTimer *timer, CLogger* logger, CDeviceNameService* dns);
    ~Storage (void);

    boolean Initialize (void);

    void ListDirectories(CScreenDevice* screen);

private:
//    CMemorySystem		m_Memory;
    CActLED			    m_ActLED;
    CDeviceNameService	*m_DeviceNameService;
    CInterruptSystem	*m_Interrupt;
    CTimer			    *m_Timer;
    CLogger			    *m_Logger;

    CEMMCDevice		    m_EMMC;
    CFATFileSystem		m_FileSystem;
    CDevice             *m_Partition;
};


#endif //USBKEYBOARD_STORAGE_H
