#include "stdio.h"
#include <opencv2/opencv.hpp>
#include "ASICamera2.h"
#include <sys/time.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>

#include "ecam.h"

using namespace cv;

eCamera::eCamera() {
    this->log_flags_ = ECAM_LOGS_ERROR;

    this->connected_ = false;
    if (NumConnectedCameras() == 0) {
        eclogf(ERROR,"No cameras connected!\n");
        return;
    }

    if (ASIOpenCamera(0) != ASI_SUCCESS)
    {
        eclogf(ERROR,"OpenCamera error\n");
        return;
    }
    if (ASIInitCamera(0) != ASI_SUCCESS) {
        eclogf(ERROR,"Could not initialize Camera!\n");
        ASICloseCamera(0);
        return;
    }

    ASI_CAMERA_INFO ASICameraInfo;

    ASIGetCameraProperty(&ASICameraInfo, 0);
    this->width_ = ASICameraInfo.MaxWidth;
    this->height_ = ASICameraInfo.MaxHeight;
    this->pixel_size_ = ASICameraInfo.PixelSize;
    this->idx_ = ASICameraInfo.CameraID;
    int *temp;

    printf("Opened CameraID = %d\n",this->idx_);
    int w;
    int h;
    int bin;
    ASI_IMG_TYPE it;
    this->setMonoBin();

    ASIGetROIFormat(this->idx_, &w, &h,  &bin, &it);
    printf("imgtype =  %d %d %d %d\n",w,h,bin,it);
    sleep(1);
#if 0
    ASI_ERROR_CODE err;
    err = ASISetStartPos(0,0,0);
    if (err != ASI_SUCCESS) {
        printf("Set Start Pos failed\n");
    }
    err = ASISetROIFormat(this->idx_, w, h, 1, it);
    if (err != ASI_SUCCESS) {
        printf("Set ROI format failed - %d\n",err);
    }
    this->width_=w/4;
    this->height_ = h/4;

    ASIGetROIFormat(this->idx_, &w, &h,  &bin, &it);

    printf("imgtype =  %d %d %d %d\n",w,h,bin,it);
#endif
    this->image_ = Mat::zeros(this->height_, this->width_, CV_8UC1 );

#if 0
    int numcontrols;
    ASI_CONTROL_CAPS cap;
    ASIGetNumOfControls(this->idx_, &numcontrols);
    for (int i=0 ; i<numcontrols; i++) {
        ASIGetControlCaps(this->idx_,i,&cap);
        printf("Control %s\n",cap.Name);
        printf("\t%s\n",cap.Description);
        printf("\tMax: %ld\n",cap.MaxValue);
        printf("\tMin: %ld\n",cap.MinValue);
        printf("\tdef:%ld\n",cap.DefaultValue);
        printf("\tctl type:%d\n",cap.ControlType);
    }
#endif
    this->connected_ = true;
    return;
}

uint32_t eCamera::NumConnectedCameras() {
    return ASIGetNumOfConnectedCameras();
}

eCamera::~eCamera() {
    if (idx_ >= 0) {
        ASICloseCamera(idx_);
    }
    if (buf_) free(buf_);
    printf("eCamera closed\n");
}

eCamera* eCamera::Create() {
    if (NumConnectedCameras() == 0) {
        printf("No cameras connected!\n");
        return NULL;
    }

    if (ASIOpenCamera(0) != ASI_SUCCESS)
    {
        printf("OpenCamera error\n");
        return NULL;
    }
    if (ASIInitCamera(0) != ASI_SUCCESS) {
        printf("Could not initialize Camera!\n");
        ASICloseCamera(0);
        return NULL;
    }
    eCamera *ec = new eCamera;

    ec->idx_ = 0;
    return ec;
}

void eCamera::startExposure() {
    ASI_BOOL dark = ASI_FALSE;
    ASIStartExposure(idx_, dark);
}

int eCamera::exposureStatus() {
    ASI_EXPOSURE_STATUS stat;
    ASIGetExpStatus(idx_,&stat);
    if (stat == ASI_EXP_WORKING) return 1;
    if (stat == ASI_EXP_SUCCESS) return 0;
    if (stat == ASI_EXP_FAILED) return -1;
    return -1;
}

long eCamera::getVal(ASI_CONTROL_TYPE ctl) {
    long val;
    ASI_BOOL pbool = ASI_FALSE;
    ASI_ERROR_CODE stat;
    stat = ASIGetControlValue(idx_,ctl,&val,&pbool);

    if (stat != ASI_SUCCESS) {
        printf("getVal failed %d\n",stat);
    }
    return val;
}

long eCamera::setVal(ASI_CONTROL_TYPE ctl, long val){
    ASI_BOOL pbool = ASI_FALSE;
    ASI_ERROR_CODE stat;
    stat = ASISetControlValue(idx_,ctl,val,pbool);

    if (stat != ASI_SUCCESS) {
        printf("setVal failed %d\n",stat);
    }
    return val;
}

uint32_t eCamera::loadData() {
    //uint8_t *buf = malloc(width_*height_);

    if (ASIGetDataAfterExp(0,(unsigned char*)image_.ptr(),width_ * height_ ) != ASI_SUCCESS) {
        printf("GetImage failed\n");
        //free(buf);
        return 1;
    }
    //cv::Mat 8mat(height, width, CV_16UC1, inputBuffer);
    im_rgb_ = cv::Mat(height_, width_, CV_8UC3);
    cv::cvtColor(image_, im_rgb_, cv::COLOR_BayerRG2RGB);
    im_preview_ = cv::Mat(height_/4, width_/4, CV_8UC3);
    resize(im_rgb_, im_preview_, im_preview_.size());
    //im_rgb_ = cv::Mat(width_, height_, CV_8UC3);
    //im_rgb16_.convertTo(im_rgb_, CV_8UC3, 1.0/256);
    return 0;
}


uint32_t eCamera::showData() {
    //namedWindow( "image preview", CV_WINDOW_AUTOSIZE );
    imshow("image preview",im_preview_);
    return 0;
}


double eCamera::adu() {
    if (!buf_) {
        printf("no valid buffer!\n");
        return 0;
    }
    uint64_t sum=0;
    uint16_t *temp = (uint16_t*)buf_;
    for (int i=0; i < width_ *height_; i++) {
        sum+=temp[i];
    }
    printf("sum = %lld\n",sum);
    return sum/(double)(width_*height_);
}

void eCamera::showit() {
    uint8_t *ptr = image_.ptr();
    for (int x=0 ; x<20; x++)
        printf("%02x ",ptr[x]);
    printf("\n");
}



int  main()
{
    eCamera ec;// = eCamera::Create();
    if (!ec.isConnected()) {
        return 0;
    }

    ec.setGain(300);
    printf("Gain = %ld\n",ec.getGain());
    printf("Temperature = %f\n",ec.getTemperature());
    printf("Width = %u\n",ec.getWidth());
    printf("Exposure = %ld\n",ec.getExposure());


    if (ec.isHWBin()) { printf("Hardware binning on\n");}
    ec.setExposure(100000);
    printf("Exposure = %ld\n",ec.getExposure());
    while (1) {
        char c;
        ec.startExposure();
        while (ec.exposureStatus() != 0) {
        }
        ec.loadData();
        ec.showData();
        c=waitKey(1);
        if (c == 'x') break;
    }
    return 0;

}
