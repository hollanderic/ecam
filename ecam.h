
#pragma once

#include <opencv2/opencv.hpp>
#include <unistd.h>

#include "ASICamera2.h"
using namespace cv;

#define ECAM_LOGS_ALL   (0xffffffff)
#define ECAM_LOGS_ERROR ( 1 << 0 )
#define ECAM_LOGS_DEBUG ( 1 << 1 )

class eCamera {
public:
    static eCamera *Create();
    static uint32_t NumConnectedCameras();

    eCamera();
    eCamera(uint32_t idx);

    ~eCamera();

    long        getGain()               { return getVal(ASI_GAIN);};
    long        setGain(long val)       { return setVal(ASI_GAIN,val);};
    long        getExposure()           { return getVal(ASI_EXPOSURE);};
    long        setExposure(long val)   { return setVal(ASI_EXPOSURE,val);};
    float       getTemperature()        { return (float)getVal(ASI_TEMPERATURE)/10.0;}
    uint32_t    setMonoBin()            { return setVal(ASI_MONO_BIN,1);};
    long        isHWBin()               { return getVal(ASI_HARDWARE_BIN);};

    void        startExposure();
    int         exposureStatus();

    uint32_t    getWidth()              { return width_;};
    uint32_t    getHeight()             { return height_;};
    bool        isConnected()           { return connected_;};

    void        enableLogs(uint32_t flags) { log_flags_ |= flags;};
    void        disableLogs(uint32_t flags) { log_flags_ &= ~flags;};

    uint32_t    loadData();
    uint32_t    showData();
    double      adu();
    void        showit();
    uint32_t    logf(uint32_t flags, const char* fmt, ...);


private:
    long getVal( ASI_CONTROL_TYPE ctl);
    long setVal( ASI_CONTROL_TYPE ctl, long val);

    bool connected_;

    uint32_t log_level_ = 0;

    int idx_ = -1;
    uint32_t width_;
    uint32_t height_;
    uint32_t pixel_size_;
    uint8_t *buf_ = NULL;

    Mat image_;
    Mat im_rgb16_;
    Mat im_rgb_;
    Mat im_preview_;
};
