#include <iostream>
#include <string>

#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/imgproc.hpp>

int main(int argc, char *argv[]) {

  std::string filename(argv[1]);
  cv::VideoCapture capture(filename);

  if (!capture.isOpened()) {
    std::cerr << "Could not open" << filename << std::endl;
    return EXIT_FAILURE;
  }

  int frameCount = capture.get(cv::CAP_PROP_FRAME_COUNT);
  int width = capture.get(cv::CAP_PROP_FRAME_WIDTH);
  int height = capture.get(cv::CAP_PROP_FRAME_HEIGHT);

  int stepSize = (frameCount / width) - 1;
  int alongRows = 1;

  cv::Mat frame;
  capture >> frame;
  cv::Mat rainbow(height, width, frame.type());
  cv::Mat meanColumn(height, 1, frame.type());

  for (int col = 0; col < width; ++col) {
    // read current frame
    std::cerr << col << "/" << width << std::endl;
    capture >> frame;

    // end of file
    if (frame.empty()) {
      std::cerr << "premature end of file" << std::endl;
      break;
    }

    // mean-reduce current frame along rows
    cv::reduce(frame, meanColumn, alongRows, cv::REDUCE_AVG);

    // assign output image column to mean column of frame
    meanColumn.col(0).copyTo(rainbow.col(col));

    // skip some frames
    for (int skip = 0; skip < stepSize; ++skip) {
      capture.grab();
    }
  }

  // warp from polar coordinates to cartesian
  int flags = cv::INTER_CUBIC + cv::WARP_POLAR_LINEAR + cv::WARP_INVERSE_MAP;
  int halfWidth = width / 2;
  int halfHeight = height / 2;
  cv::Point2f center(halfWidth, halfHeight);
  int maxRadius = std::min(center.x, center.y);

  cv::Mat diskLinear(rainbow);
  cv::Mat diskLog(rainbow);

  cv::linearPolar(rainbow, diskLinear, center, maxRadius, flags);
  cv::rotate(diskLinear, diskLinear, cv::ROTATE_90_COUNTERCLOCKWISE);
  cv::imwrite("img/linear.jpg", diskLinear);

  flags = cv::INTER_CUBIC + cv::WARP_POLAR_LOG + cv::WARP_INVERSE_MAP;
  cv::linearPolar(rainbow, diskLog, center, maxRadius, flags);
  cv::rotate(diskLog, diskLog, cv::ROTATE_90_COUNTERCLOCKWISE);
  cv::imwrite("img/log.jpg", diskLog);

  return EXIT_SUCCESS;
}