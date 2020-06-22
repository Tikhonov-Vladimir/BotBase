#include <iostream>
#include <chrono>
#include <thread>

#include <opencv2/opencv.hpp>
#include <ViZDoom.h>

using namespace vizdoom;

DoomGame* game = new DoomGame();
const std::vector<double> actions[3] = {
	{ 1, 0, 0 }, // left
	{ 0, 1, 0 }, // right
	{ 0, 0, 1 }, // shoot
};

void sleep(size_t time) {
	std::this_thread::sleep_for(std::chrono::milliseconds(time));
}

void game_init() {
	game->setViZDoomPath("../vizdoom/vizdoom");
	game->setDoomGamePath("../vizdoom/freedoom2.wad");
	game->loadConfig("../vizdoom/scenarios/task1.cfg"); // add configurations for game
	game->setScreenResolution(RES_640X480); // ����������
	game->setLabelsBufferEnabled(1); // add this
	game->setWindowVisible(1); // exception with Linux without X Series
	game->setRenderWeapon(1); // is the gun will be in the game
	game->setRenderHud(1);
	game->init();
}

struct crd {
	long long x = 0, y = 0;
};

crd getCrdMonstr(IplImage* img, const int& width, const int& height, GameStatePtr state) {
	crd sum{ 0, 0 };
	std::cout << sum.x << ' ' << sum.y << std::endl;
	long long counter = 0;
	cv::Mat m = cv::Mat(width, height, CV_8UC1);
	m = cv::cvarrToMat(img);
	cvNamedWindow("original2", CV_WINDOW_AUTOSIZE);
	std::memcpy(m.data, state->screenBuffer->data(), state->screenBuffer->size());
	system("pause >nul");
	for (int x = 200; x < 250/*height*/; ++x) {
		for (int y = 0; y < width; ++y) {
			if (m.at<unsigned char>(x, y) < 100) {
				sum.x += x;
				sum.y += y;
				++counter;
			}
		}
	}
	sum.x = long long(double(sum.x) / counter);
	sum.y = long long(double(sum.y) / counter);
	std::cout << sum.x << ' ' << sum.y << std::endl;
	return sum;
}

void find_demon_and_kill(GameStatePtr state) {
	double eps = 10; // monster's width
	if (state->labels[0].objectPositionY - eps > state->labels[1].objectPositionY) {
		game->makeAction(actions[0]); //left
	}
	else if (state->labels[0].objectPositionY + eps < state->labels[1].objectPositionY) {
		game->makeAction(actions[1]); //right
	}
	else {
		game->makeAction(actions[2]); // shoot
	}
}

CvPoint find(cv::Mat matrix) {
	CvPoint sum = cvPoint(0, 0);
	int counter = 0;
	for (int y = 200; y < 210; y++) {
		for (int x = 30; x < (&matrix)->cols - 30; x++) {
			if (matrix.at<unsigned char>(y, x) == 255) {
				sum.x += x + 10;
				sum.y += y;
				return sum;
			}
		}
		//std::cout << sum.x << ' ' << sum.y << "\n";
	}
	sum.x = sum.x / double(counter);
	sum.y = sum.y / double(counter);
	return sum;
}

void kill(int x) {
	if (x >= 356)
		game->makeAction(actions[1]); //right;
	else if (x <= 284)
		game->makeAction(actions[0]); //left
	else
		game->makeAction(actions[2]); // shoot

}


int main() {
	game_init();

	int height = 640;
	int width = 480;
	auto scr = cv::Mat(480, 640, CV_8UC3);
	auto greyscale = cv::Mat(480, 640, CV_8UC1);
	int res = 0;

	int episodes = 10;
	size_t sleepTime = 2000 / DEFAULT_TICRATE;
	for (int i = 0; i < episodes; ++i) {
		std::cout << "Episode #" << i + 1 << "\n";
		game->newEpisode();
		while (!game->isEpisodeFinished()) {
			auto state = game->getState();
			auto screenBuf = state->screenBuffer;

			//cvSetImageROI(image, CvRect rect);

			//find_demon_and_kill(state);
			std::memcpy(scr.data, state->screenBuffer->data(), state->screenBuffer->size());
			cv::extractChannel(scr, greyscale, 1);
			cv::threshold(greyscale, greyscale, 130, 255, cv::THRESH_BINARY);
			//auto greyscale1 = greyscale(cvRect(0, 200, 640, 5));
			CvPoint w = find(greyscale);
			kill(w.x);
			//cv::imshow("original", greyscale);


			//sleep(sleepTime);
			cvWaitKey(sleepTime);
		}
		sleep(sleepTime * 10);
		std::cout << "Total reward is: " << game->getTotalReward() << std::endl;
		res += game->getTotalReward();
	}
	cvDestroyWindow("original");
	std::cout << double(res) / episodes;
	system("pause >nul");
	game->close();
	delete game;
}

//#include <iostream>
//#include <cv.h>
//#include <highgui.h>
//
//IplImage* img = 0;
//IplImage* img1 = 0;
//
//int main(int argc, char* argv[]) {
//	
//	char filename[] = "D:\\hh.jpg";
//	
//	cvNamedWindow("Mypicture", CV_WINDOW_AUTOSIZE);
//	cvNamedWindow("Mypicture2", CV_WINDOW_AUTOSIZE);
//	img = cvLoadImage(filename);
//	img1 = cvCloneImage(img);
//	img1 = cvCreateImage(cvSize(img->width / 2, img->height / 2), img->depth, img->nChannels);
//	cvResize(img, img1, 2);
//
//	cvShowImage("Mypicture", img);
//	cvShowImage("Mypicture2", img1);
//
//	cvWaitKey(0);
//	cvDestroyAllWindows();
//	
//}

//#include <cv.h>
//#include <highgui.h>
//#include <stdlib.h>
//#include <stdio.h>
//
//IplImage* image = 0;
//IplImage* del = 0;
//IplImage* paste = 0;
//
//struct crd {
//	double x = 0, y = 0;
//};
//
//int main(int argc, char* argv[])
//{
//	// ��� �������� ������� ������ ����������
//	char filename[] = "D:\\hh.jpg";
//	// �������� ��������
//	image = cvLoadImage(filename, 1);
//
//	printf("[i] image: %s\n", filename);
//	assert(image != 0);
//
//	cvNamedWindow("origianl", CV_WINDOW_AUTOSIZE);
//	cvNamedWindow("ROI", CV_WINDOW_AUTOSIZE);
//	//cvNamedWindow("del", CV_WINDOW_AUTOSIZE);
//
//	// ����� ROI
//	crd A{ 20, 20 }, B{ 60, 60 }, C{ 80, 20 }, D{ 120, 60 };
//
//	paste = cvCloneImage(image);
//	cvSetImageROI(image, cv::Rect(A.x, A.y, B.x, B.y));
//	cvAddS(image, CvScalar(100), image);
//	cvResetImageROI(image);
//	cvSetImageROI(paste, cv::Rect(C.x, C.y, D.x, D.y));
//	cvZero(paste);
//	//cvAddS(paste, CvScalar(100), paste);
//	cvResetImageROI(paste);
//	//cvSetImageROI(image, CvRect(A.x, A.y, B.x, B.y));
//	//del = cvCloneImage(image);
//	//cvAddS(image, CvScalar(100), image);
//	//cvResetImageROI(image);
//
//	cvShowImage("origianl", image);
//	cvShowImage("ROI", paste);
//	//cvShowImage("del", del);
//
//	// ��� ������� �������
//	cvWaitKey(0);
//
//	// ����������� �������
//	cvReleaseImage(&image);
//	cvDestroyAllWindows();
//	return 0;
//}
