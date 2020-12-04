//
// Created by workw on 2020/12/3.
//

#include "PcmBean.h"

PcmBean::PcmBean(SAMPLETYPE *buffer, int size) {

    this->buffer = (char *) malloc(size);
    this->buffsize = size;
    memcpy(this->buffer, buffer, size);
}

PcmBean::~PcmBean() {
    free(buffer);
    buffer = NULL;
}
