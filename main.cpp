#include "stdio.h"
#include <stdlib.h>
#include <opencv2/opencv.hpp>
#include "ecam.h"


int  main(int argc, char** argv)
{

    eCamera ec;
    if (!ec.isConnected()) {
        return 0;
    }

	int c;
	while ((c = getopt(argc,argv,"g:e:n")) != -1) {
		switch (c) {
			case 'g':
				ec.setGain(optarg);
				break;
			case 'e':
				ec.setExposure(optarg);
				break;
			case 'n':
				printf("numexposure\n");
				break;
			default:
				return 0;
		}
	}

    printf("Gain = %ld\n",ec.getGain());
    printf("Temperature = %f\n",ec.getTemperature());
    printf("Width = %u\n",ec.getWidth());
    printf("Exposure = %ld\n",ec.getExposure());

    //ec.setROI(1000,1000,1000,1000);

    if (ec.isHWBin()) { printf("Hardware binning on\n");}

    ec.showPreviewWindow();
    do {
        //char c;
        ec.startExposure();
        while (ec.exposureStatus() != 0) {
        }
        ec.loadData();
        ec.showRGB();
        waitKey(1);
        //if (c == 'x') break;
    } while(0);
    //waitKey(0);
    ec.saveTIFF("eric.tiff");

    return 0;

}
