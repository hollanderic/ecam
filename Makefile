ver = debug
platform = x64

CC = g++

PLAT = $(shell uname -s)

ZWOPATH = $(HOME)/src/external/zwo


ifeq ($(PLAT),Darwin)
#INCLIB = /usr/local/include
#LDLIB = /usr/local/lib
OPENCV = /usr/local/Cellar/opencv@2/2.4.13.4
OPENCVCFLAGS = -I$(OPENCV)/include
OPENCVLIBS = -L/usr/local/Cellar/opencv@2/2.4.13.4/lib -lopencv_calib3d \
             -lopencv_contrib -lopencv_core -lopencv_features2d -lopencv_flann \
             -lopencv_gpu -lopencv_highgui -lopencv_imgproc -lopencv_legacy -lopencv_ml \
             -lopencv_nonfree -lopencv_objdetect -lopencv_ocl -lopencv_photo \
             -lopencv_stitching -lopencv_superres -lopencv_ts -lopencv_video -lopencv_videostab
platform = mac
ZWOLIBS = DYLD_LIBRARY_PATH=$(ZWOPATH)/lib/$(platform)
endif

ifeq ($(PLAT),Linux)
platform = x64
OPENCVCFLAGS = $(shell pkg-config --cflags opencv) -I/usr/include/opencv2
OPENCVLIBS = $(shell pkg-config --libs opencv)
ZWOLIBS = LD_LIBRARY_PATH=$(ZWOPATH)/lib/$(platform)

endif
#$(shell pkg-config --libs opencv)  -lopencv_core -lopencv_highgui -lopencv_imgproc
#USB =  -I../libusb/include  -L../libusb/$(platform) -lusb-1.0

LIBSPATH = -L$(ZWOPATH)/lib/$(platform) -L/usr/lib64 -I$(ZWOPATH)/include





ifeq ($(ver), debug)
DEFS = -D_LIN -D_DEBUG
CFLAGS = -std=c++11 -g  -I $(INCLIB) -L $(LDLIB) $(DEFS) $(COMMON) $(LIBSPATH)  -lpthread  -DGLIBC_20
else
DEFS = -D_LIN
CFLAGS =  -std=c++11 -O3 -I $(INCLIB) -L $(LDLIB) $(DEFS) $(COMMON) $(LIBSPATH)  -lpthread  -DGLIBC_20
endif

ifeq ($(platform), x64)
CFLAGS += -m64
CFLAGS += -lrt
endif
.PHONY: ecam

all: ecam


ecam:ecam.cpp
	$(CC)  ecam.cpp -o ecam $(CFLAGS) $(OPENCVCFLAGS) $(OPENCVLIBS) -lASICamera2
	@echo "$(ZWOLIBS) ./ecam">tester
	@chmod +x tester

cv: cv.cpp
	$(CC) cv.cpp -o cv $(OPENCVCFLAGS) $(OPENCVLIBS)
	echo "LD_LIBRARY_PATH=$(ZWOPATH)/lib/$(platform) cv">tester
	chmod +x tester

clean:
	rm -f ecam
#pkg-config libusb-1.0 --cflags --libs
#pkg-config opencv --cflags --libs

