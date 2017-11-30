
#include <opencv2/opencv.hpp>
#include <unistd.h>

#include "ASICamera2.h"
using namespace cv;

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

    uint32_t    loadData();
    uint32_t    showData();
    double      adu();
    void        showit();


private:
    long getVal( ASI_CONTROL_TYPE ctl);
    long setVal( ASI_CONTROL_TYPE ctl, long val);
    bool connected_;
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
