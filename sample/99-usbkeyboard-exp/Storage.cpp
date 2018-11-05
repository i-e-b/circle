//
// Created by iain on 05/11/18.
//

#include "Storage.h"

Storage::Storage(CInterruptSystem *interrupt, CTimer *timer)
        :  m_Interrupt(interrupt)
         , m_Timer(timer)
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

    return bOK;
}
