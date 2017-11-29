ver = debug
platform = x64

CC = g++
#INCLIB = /usr/local/include
#LDLIB = /usr/local/lib
OPENCV = $(shell pkg-config --cflags opencv) $(shell pkg-config --libs opencv) -I/usr/include/opencv2 -lopencv_core -lopencv_highgui -lopencv_imgproc

#USB =  -I../libusb/include  -L../libusb/$(platform) -lusb-1.0  

#-lopencv_core -lopencv_highgui -lopencv_imgproc#

ZWOPATH = $(HOME)/src/external/zwo
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


all: ecam


ecam:ecam.cpp
	$(CC)  ecam.cpp -o ecam $(CFLAGS) $(OPENCV) -lASICamera2

cv: cv.c
	$(CC) cv.c -o cv $(CFLAGS) $(OPENCV)

clean:
	rm -f ecam
#pkg-config libusb-1.0 --cflags --libs
#pkg-config opencv --cflags --libs

