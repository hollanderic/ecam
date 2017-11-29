#include "stdio.h"
//#include "opencv2/highgui/highgui_c.h"
#include <opencv.hpp>
#include "ASICamera2.h"
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "ecam.h"

using namespace cv;

eCamera::eCamera() {
	this->connected_ = false;
	if (NumConnectedCameras() == 0) {
		printf("No cameras connected!\n");
		return;
	}

	if (ASIOpenCamera(0) != ASI_SUCCESS)
	{
		printf("OpenCamera error\n");
		return;
	}
	if (ASIInitCamera(0) != ASI_SUCCESS) {
		printf("Could not initialize Camera!\n");
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
	temp = ASICameraInfo.SupportedBins;
	for (int i = 0; i < 16 & temp[i] !=0; i++) {
		printf("%d ",temp[i]);
	}
	printf("\n");

	printf("Opened CameraID = %d\n",this->idx_);
	int w;
	int h;
	int bin;
	ASI_IMG_TYPE it;
	this->setMonoBin();

	ASIGetROIFormat(this->idx_, &w, &h,  &bin, &it);
	printf("imgtype =  %d %d %d %d\n",w,h,bin,it);
	sleep(1);
#if 1
	ASI_ERROR_CODE err;
	err = ASISetStartPos(0,0,0);
	if (err != ASI_SUCCESS) {
		printf("Set Start Pos failed\n");
	}
	err = ASISetROIFormat(this->idx_, w/4, h/4, 4, it);
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
	printf("sum = %ld\n",sum);
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
	ec.setGain(500);
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
	printf("\nGot it!\n");
	//printf("adu = %f\n",ec.adu());
	return 0;
 

#if 0
	int numDevices = ASIGetNumOfConnectedCameras();
	if(numDevices <= 0)
	{
		printf("no camera connected, press any key to exit\n");
		getchar();
		return -1;
	}
	printf("Found %d devices\n",numDevices);


	ASI_CAMERA_INFO ASICameraInfo;

	int width=0;
	int height=0;
	for(int i = 0; i < numDevices; i++)
	{
		ASIGetCameraProperty(&ASICameraInfo, i);
		printf("%d %s\n",i, ASICameraInfo.Name);
		printf("\tID: %d\n",ASICameraInfo.CameraID);
		printf("\tWidth:  %ld\n",ASICameraInfo.MaxWidth);
		printf("\tHeight: %ld\n",ASICameraInfo.MaxHeight);
		printf("\tPixSize: %f\n",ASICameraInfo.PixelSize);
		width = ASICameraInfo.MaxWidth;
		height = ASICameraInfo.MaxHeight;
	}

	if(ASIOpenCamera(0) != ASI_SUCCESS)
	{
		printf("OpenCamera error\n");
		return -1;
	}
	ASIInitCamera(0);

	//ASI_CONTROL_CAPS ControlCaps;
	//int iNumOfCtrl = 0;
	//ASIGetNumOfControls(0, &iNumOfCtrl);
	//for (int i=0; i<iNumOfCtrl; i++) {
	//	ASIGetControlCaps(0, i, &ControlCaps);
	//	printf("Control %d: %s\n",i,ControlCaps.Name);
	//	printf("\t%s\n",ControlCaps.Description);
	//}
	if(ASIStartExposure(0,(ASI_BOOL)0) != ASI_SUCCESS) {
		printf("failed exposure start\n");
		return -1;
	}
	sleep(2);
	if(ASIStopExposure(0) != ASI_SUCCESS) {
		printf("failed exposure stop\n");
		return -1;
	}
	printf("%d  %d\n",width,height);
	sleep(1);
	uint16_t *buf = (uint16_t*)malloc(width*height*2);
	uint32_t size = width*height;
	printf("buf size=%d\n",width*height*2);
	sleep(1);
	if (ASIGetDataAfterExp(0,(unsigned char*)(buf),width*height*2) != ASI_SUCCESS) {
		printf("get data failed\n");
		return -1;
	}
	for (int i =0 ; i<100; i++) {
		printf("%d\n",buf[i]);
	}
	uint32_t sum=0;
	for (int i=0; i<size; i++){
		sum+=buf[i];
	}

	printf("total = %u\n",sum);
	printf("success\n");
#endif
}
