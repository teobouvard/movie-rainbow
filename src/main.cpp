#include <iostream>
#include <string>

#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/imgproc.hpp>

/**
 * @brief Modified version of cv::warpPolar to pass outlier fill color to
 * cv::remap
 *
 * @param _src
 * @param _dst
 * @param dsize
 * @param center
 * @param maxRadius
 * @param flags
 * @param background
 */
void warpPolar(cv::InputArray _src, cv::OutputArray _dst, cv::Size dsize,
               cv::Point2f center, double maxRadius, int flags,
               cv::Scalar background) {
  // if dest size is empty given than calculate using proportional setting
  // thus we calculate needed angles to keep same area as bounding circle
  if ((dsize.width <= 0) && (dsize.height <= 0)) {
    dsize.width = cvRound(maxRadius);
    dsize.height = cvRound(maxRadius * CV_PI);
  } else if (dsize.height <= 0) {
    dsize.height = cvRound(dsize.width * CV_PI);
  }

  cv::Mat mapx, mapy;
  mapx.create(dsize, CV_32F);
  mapy.create(dsize, CV_32F);
  bool semiLog = (flags & cv::WARP_POLAR_LOG) != 0;

  if (!(flags & cv::WARP_INVERSE_MAP)) {
    CV_Assert(!dsize.empty());
    double Kangle = CV_2PI / dsize.height;
    int phi, rho;

    // precalculate scaled rho
    cv::Mat rhos = cv::Mat(1, dsize.width, CV_32F);
    float *bufRhos = (float *)(rhos.data);
    if (semiLog) {
      double Kmag = std::log(maxRadius) / dsize.width;
      for (rho = 0; rho < dsize.width; rho++)
        bufRhos[rho] = (float)(std::exp(rho * Kmag) - 1.0);

    } else {
      double Kmag = maxRadius / dsize.width;
      for (rho = 0; rho < dsize.width; rho++)
        bufRhos[rho] = (float)(rho * Kmag);
    }

    for (phi = 0; phi < dsize.height; phi++) {
      double KKy = Kangle * phi;
      double cp = std::cos(KKy);
      double sp = std::sin(KKy);
      float *mx = (float *)(mapx.data + phi * mapx.step);
      float *my = (float *)(mapy.data + phi * mapy.step);

      for (rho = 0; rho < dsize.width; rho++) {
        double x = bufRhos[rho] * cp + center.x;
        double y = bufRhos[rho] * sp + center.y;

        mx[rho] = (float)x;
        my[rho] = (float)y;
      }
    }
    cv::remap(_src, _dst, mapx, mapy, flags & cv::INTER_MAX,
              (flags & cv::WARP_FILL_OUTLIERS) ? cv::BORDER_CONSTANT
                                               : cv::BORDER_TRANSPARENT,
              background);
  } else {
    const int ANGLE_BORDER = 1;
    cv::copyMakeBorder(_src, _dst, ANGLE_BORDER, ANGLE_BORDER, 0, 0,
                       cv::BORDER_WRAP);
    cv::Mat src = _dst.getMat();
    cv::Size ssize = _dst.size();
    ssize.height -= 2 * ANGLE_BORDER;
    CV_Assert(!ssize.empty());
    const double Kangle = CV_2PI / ssize.height;
    double Kmag;
    if (semiLog)
      Kmag = std::log(maxRadius) / ssize.width;
    else
      Kmag = maxRadius / ssize.width;

    int x, y;
    cv::Mat bufx, bufy, bufp, bufa;

    bufx = cv::Mat(1, dsize.width, CV_32F);
    bufy = cv::Mat(1, dsize.width, CV_32F);
    bufp = cv::Mat(1, dsize.width, CV_32F);
    bufa = cv::Mat(1, dsize.width, CV_32F);

    for (x = 0; x < dsize.width; x++)
      bufx.at<float>(0, x) = (float)x - center.x;

    for (y = 0; y < dsize.height; y++) {
      float *mx = (float *)(mapx.data + y * mapx.step);
      float *my = (float *)(mapy.data + y * mapy.step);

      for (x = 0; x < dsize.width; x++)
        bufy.at<float>(0, x) = (float)y - center.y;

      cv::cartToPolar(bufx, bufy, bufp, bufa, 0);

      if (semiLog) {
        bufp += 1.f;
        log(bufp, bufp);
      }

      for (x = 0; x < dsize.width; x++) {
        double rho = bufp.at<float>(0, x) / Kmag;
        double phi = bufa.at<float>(0, x) / Kangle;
        mx[x] = (float)rho;
        my[x] = (float)phi + ANGLE_BORDER;
      }
    }
    cv::remap(src, _dst, mapx, mapy, flags & cv::INTER_MAX,
              (flags & cv::WARP_FILL_OUTLIERS) ? cv::BORDER_CONSTANT
                                               : cv::BORDER_TRANSPARENT,
              background);
  }
}

int generateRainbow(const std::string &filename) {
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

  // add disk interior padding
  int paddingRadius = 400;
  cv::Scalar paddingColor(255, 255, 255);
  cv::Mat paddedRainbow(height + paddingRadius, width, frame.type());
  paddedRainbow.setTo(paddingColor);
  for (int i = 0; i < rainbow.rows; ++i) {
    rainbow.row(i).copyTo(paddedRainbow.row(i));
  }

  // TODO these should be parameters
  int flags;
  flags += cv::INTER_CUBIC;
  flags += cv::WARP_POLAR_LINEAR;
  flags += cv::WARP_INVERSE_MAP;
  flags += cv::WARP_FILL_OUTLIERS;

  cv::rotate(paddedRainbow, paddedRainbow, cv::ROTATE_90_CLOCKWISE);
  int centerX = (height + paddingRadius) / 2;
  int centerY = width / 2;
  int radius = std::min(centerX, centerY) - 100;
  cv::Point2f center(centerX, centerY);

  // warp from polar to cartesian space
  warpPolar(paddedRainbow, paddedRainbow, paddedRainbow.size(), center, radius,
            flags, paddingColor);
  cv::rotate(paddedRainbow, paddedRainbow, cv::ROTATE_90_COUNTERCLOCKWISE);

  cv::imwrite("img/out.png", paddedRainbow);
  return EXIT_SUCCESS;
}
int main(int argc, char *argv[]) {

  std::string movieFilename(argv[1]);
  generateRainbow(movieFilename);

  return EXIT_SUCCESS;
}