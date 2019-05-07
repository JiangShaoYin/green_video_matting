#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

Mat replace_and_blend(Mat &frame, Mat &mask);
Mat background_01;
Mat background_02;
int main(int argc, char** argv) {
	// start here...	
	background_01 = imread("bg_01.jpg");
	background_02 = imread("bg_02.jpg");
	Mat sizeImg = imread("标准尺寸.png");
	if (background_01.empty() || background_02.empty())
	{
		printf("could not load image...\n");
		return -1;
	}
	resize(background_01, background_01, Size(sizeImg.cols, sizeImg.rows), 0, 0, INTER_LINEAR);// 将这2张图resize成标准尺寸.png的长宽
	resize(background_02, background_02, Size(sizeImg.cols, sizeImg.rows), 0, 0, INTER_LINEAR);

	VideoCapture capture;
	capture.open("01.mp4");
	if (!capture.isOpened()) {
		printf("could not find the video file...\n");
		return -1;
	}
	const char* title = "input video";
	const char* resultWin = "result video";
	namedWindow(title, 0);
	namedWindow(resultWin, 0);

	Mat frame, hsv, mask;
	Mat mask_erode, mask_blur, mask_close;
	int count = 0;
	while (capture.read(frame)) {  //读取每1帧
		cvtColor(frame, hsv, COLOR_BGR2HSV);
		/*
		从读取像素到转到hsv空间，后经过inRang的处理，这个处理类似于threshold二值（只能处理单通道的图像），
		通过上下限的方式二值(可处理单通道或多通道)，经过这个处理后得到一个二值化的Mat，因为inRang后的图像边缘存在噪点
		使用通过形态学闭操作消除小白点，得到的图像进行边缘羽化处理
		羽化处理：使用腐蚀的方法将人物的mask向外腐蚀一个边界像素，
		然后用高斯模糊，进行梯度边缘处理，得到最终的预处理效果
		*/
		inRange(hsv, Scalar(35, 43, 46), Scalar(155, 255, 255), mask);
		// 形态学操作
		Mat k = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));
		morphologyEx(mask, mask_close, MORPH_CLOSE, k);
		erode(mask_close, mask_erode, k);
		GaussianBlur(mask_erode, mask_blur, Size(3, 3), 0, 0);

		Mat result = replace_and_blend(frame, mask_blur);
		char c = waitKey(10);
		if (c == 27) {
			break;
		}
		imshow(resultWin, result);
		imshow(title, frame);
	}

	waitKey(0);
	return 0;
}

Mat replace_and_blend(Mat &frame, Mat &mask) {
	Mat result = Mat::zeros(frame.size(), frame.type());
	int h = frame.rows;
	int w = frame.cols;
	int dims = frame.channels();

	// replace and blend
	int m = 0;
	double wt = 0;

	int r = 0, g = 0, b = 0;
	int r1 = 0, g1 = 0, b1 = 0;
	int r2 = 0, g2 = 0, b2 = 0;

	for (int row = 0; row < h; row++) {
		uchar* current = frame.ptr<uchar>(row);
		uchar* bgrow = background_02.ptr<uchar>(row);
		uchar* maskrow = mask.ptr<uchar>(row);
		uchar* targetrow = result.ptr<uchar>(row);
		for (int col = 0; col < w; col++) {
			m = *maskrow++;
			if (m == 255) { // 背景
				*targetrow++ = *bgrow++;
				*targetrow++ = *bgrow++;
				*targetrow++ = *bgrow++;
				current += 3;

			}
			else if (m == 0) {// 前景
				*targetrow++ = *current++;
				*targetrow++ = *current++;
				*targetrow++ = *current++;
				bgrow += 3;
			}
			else {
				b1 = *bgrow++;
				g1 = *bgrow++;
				r1 = *bgrow++;

				b2 = *current++;
				g2 = *current++;
				r2 = *current++;

				// 权重
				wt = m / 255.0;

				// 混合
				b = b1 * wt + b2 * (1.0 - wt);
				g = g1 * wt + g2 * (1.0 - wt);
				r = r1 * wt + r2 * (1.0 - wt);

				*targetrow++ = b;
				*targetrow++ = g;
				*targetrow++ = r;
			}
		}
	}

	return result;
}
