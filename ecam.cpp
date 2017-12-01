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

    eclogf(INFO,"Opened CameraID = %d\n",this->idx_);
    int w;
    int h;
    int bin;
    ASI_IMG_TYPE it;

    ASIGetROIFormat(this->idx_, &w, &h,  &bin, &it);
    eclogf(INFO,"imgtype =  %d %d %d %d\n",w,h,bin,it);

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


uint32_t eCamera::setROI(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {

    ASI_ERROR_CODE err;
    err = ASISetStartPos(idx_,x,y);
    if (err != ASI_SUCCESS) {
        eclogf(ERROR,"Set Start Pos failed\n");
        return 1;
    }
    width = (width + 8 - 1) & ~0x07;
    height = (height + 8 - 1) & ~0x07;

    printf("setting roi - %d x %d\n",width,height);
    err = ASISetROIFormat(idx_, width, height, 1, ASI_IMG_RAW16);
    if (err != ASI_SUCCESS) {
        eclogf(ERROR,"Set ROI format failed - %d\n",err);
    }
    width_ = width;
    height_ = height;

    int w;
    int h;
    int bin;
    ASI_IMG_TYPE it;
    ASIGetROIFormat(idx_, &w, &h,  &bin, &it);
    width_ = w;
    height_ = h;
    bpp_ = 2;

    return 0;

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

uint32_t eCamera::saveTIFF(const char* fname){
    imwrite(fname,im_rgb_);
    return 0;
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
    image_ = Mat::zeros(height_, width_, CV_16UC1 );

    if (ASIGetDataAfterExp(0,(unsigned char*)image_.ptr(),width_ * height_ * bpp_ ) != ASI_SUCCESS) {
        printf("GetImage failed\n");
        return 1;
    }
    //cv::Mat 8mat(height, width, CV_16UC1, inputBuffer);
    im_rgb_ = cv::Mat(height_, width_, CV_16UC3);
    cv::cvtColor(image_, im_rgb_, cv::COLOR_BayerRG2RGB);
    im_preview_ = cv::Mat(height_/4, width_/4, CV_8UC3);
    resize(im_rgb_, im_preview_, im_preview_.size());
    //im_rgb_ = cv::Mat(width_, height_, CV_8UC3);
    //im_rgb16_.convertTo(im_rgb_, CV_8UC3, 1.0/256);
    return 0;
}


uint32_t eCamera::showData() {
    //namedWindow( "image preview", CV_WINDOW_AUTOSIZE );
    imshow("image preview",image_);
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

