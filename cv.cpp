/**
 * @file Drawing_1.cpp
 * @brief Simple geometric drawing
 * @author OpenCV team
 */
//#include <core/core.hpp>
//#include <imgproc/imgproc.hpp>
//#include <highgui/highgui.hpp>
#include<opencv2/opencv.hpp>

#define w 400

using namespace cv;
#define LINE_8 8

/**
 * @function main
 * @brief Main function
 */
int main( void ){

  //![create_images]
  /// Windows names
  char atom_window[] = "Drawing 1: Atom";
  char rook_window[] = "Drawing 2: Rook";

  /// Create black empty images
  Mat atom_image = Mat::zeros( w, w, CV_8UC3 );
  Mat rook_image = Mat::zeros( w, w, CV_8UC3 );
  //![create_images]

  /// 3. Display your stuff!
  imshow( atom_window, atom_image );
  moveWindow( atom_window, 0, 200 );
  imshow( rook_window, rook_image );
  moveWindow( rook_window, w, 200 );

  waitKey( 0 );
  return(0);
}
