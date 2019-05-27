//
// Created by workw on 2019/5/23.
//

#include "FFAudio.h"

FFAudio::FFAudio(PlayStatus *playStatus) {
    this->playstatus = playStatus;
    queue = new AVPacketQueue(playstatus);

};

FFAudio::~FFAudio() {

};