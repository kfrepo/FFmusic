//
// Created by workw on 2019/5/27.
//

#ifndef FFMUSIC_PLAYSTATUS_H
#define FFMUSIC_PLAYSTATUS_H


class PlayStatus {

public:
    bool exit = false;
    bool load = true;
    bool seek = false;

public:
    PlayStatus();
    ~PlayStatus();

};


#endif //FFMUSIC_PLAYSTATUS_H
