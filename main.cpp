#include "stdio.h"
#include <opencv2/opencv.hpp>
#include "ecam.h"


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

    ec.setROI(1000,1000,500,500);

    if (ec.isHWBin()) { printf("Hardware binning on\n");}
    ec.setExposure(100000);
    printf("Exposure = %ld\n",ec.getExposure());
    do {
        char c;
        ec.startExposure();
        while (ec.exposureStatus() != 0) {
        }
        ec.loadData();
        ec.showData();
        c=waitKey(1);
        if (c == 'x') break;
    } while(0);
    ec.saveTIFF("eric.tiff");

    return 0;

}
