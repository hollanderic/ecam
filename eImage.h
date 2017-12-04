

#include <opencv2/opencv.hpp>

class eImage {

public:

	eImage(Mat in);
	~eImage();

	void putMat();
	Mat  getMat();
	Mat  getRGB16();

private:
	Mat overlay_;
	Mat disp_buf_[2];
	uint8_t active;
	std::mtx lock;
	int	width_;
	int height_;

}