#include <iostream>
#include <string>

#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/highgui.hpp>

int main(int argc, char *argv[]) {
  std::string filename(argv[1]);
  cv::VideoCapture capture(filename);
  cv::Mat frame;

  if (!capture.isOpened()) {
    std::cerr << "Could not open" << filename << std::endl;
  }

  int frameIdx = 0;
  int frameCount = capture.get(cv::CAP_PROP_FRAME_COUNT);

  for (;;) {
    if (++frameIdx % 100 == 0) {
      std::cerr << frameIdx << "/" << frameCount << std::endl;
    }

    capture >> frame;
    // end of file
    if (frame.empty()) {
      break;
    }
  }
  return EXIT_SUCCESS;
}