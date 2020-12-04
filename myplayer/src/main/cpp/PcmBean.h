//
// Created by workw on 2020/12/3.
//

#ifndef FFMUSIC_PCMBEAN_H
#define FFMUSIC_PCMBEAN_H

#include <SoundTouch.h>

using namespace soundtouch;

class PcmBean {

public:
    char *buffer;
    int buffsize;

public:
    PcmBean(SAMPLETYPE *buffer, int size);
    ~PcmBean();
};


#endif //FFMUSIC_PCMBEAN_H
