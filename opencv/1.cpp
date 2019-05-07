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
	Mat sizeImg = imread("��׼�ߴ�.png");
	if (background_01.empty() || background_02.empty())
	{
		printf("could not load image...\n");
		return -1;
	}
	resize(background_01, background_01, Size(sizeImg.cols, sizeImg.rows), 0, 0, INTER_LINEAR);// ����2��ͼresize�ɱ�׼�ߴ�.png�ĳ���
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
	while (capture.read(frame)) {  //��ȡÿ1֡
		cvtColor(frame, hsv, COLOR_BGR2HSV);
		/*
		�Ӷ�ȡ���ص�ת��hsv�ռ䣬�󾭹�inRang�Ĵ����������������threshold��ֵ��ֻ�ܴ���ͨ����ͼ�񣩣�
		ͨ�������޵ķ�ʽ��ֵ(�ɴ���ͨ�����ͨ��)��������������õ�һ����ֵ����Mat����ΪinRang���ͼ���Ե�������
		ʹ��ͨ����̬ѧ�ղ�������С�׵㣬�õ���ͼ����б�Ե�𻯴���
		�𻯴���ʹ�ø�ʴ�ķ����������mask���ⸯʴһ���߽����أ�
		Ȼ���ø�˹ģ���������ݶȱ�Ե�����õ����յ�Ԥ����Ч��
		*/
		inRange(hsv, Scalar(35, 43, 46), Scalar(155, 255, 255), mask);
		// ��̬ѧ����
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
			if (m == 255) { // ����
				*targetrow++ = *bgrow++;
				*targetrow++ = *bgrow++;
				*targetrow++ = *bgrow++;
				current += 3;

			}
			else if (m == 0) {// ǰ��
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

				// Ȩ��
				wt = m / 255.0;

				// ���
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
