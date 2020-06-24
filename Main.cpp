#include <iostream>
#include <chrono>
#include <thread>

#include <opencv2/opencv.hpp>
#include <ViZDoom.h>

using namespace vizdoom;

DoomGame* game = new DoomGame();

const std::vector<double> actions[4] = {
	{ 1, 0, 0, 0 }, // left
	{ 0, 1, 0, 0 }, // right
	{ 0, 0, 1, 0 }, // delta
	{ 0, 0, 0, 1 }, // ahead
};

void game_init() {
	game->setViZDoomPath("../vizdoom/vizdoom");
	game->setDoomGamePath("../vizdoom/freedoom2.wad");
	game->loadConfig("../vizdoom/scenarios/task3.cfg"); // add configurations for game
	game->setScreenResolution(RES_640X480); // разрешение
	game->setLabelsBufferEnabled(1); // add this
	game->setWindowVisible(0); // exception with Linux without X Series
	game->setRenderWeapon(1); // is the gun will be in the game
	game->setRenderHud(1);
	game->init();
}

int find(cv::Mat screen) {
	const int center = 320;
	for (int y = 399; y > 150; --y)
		for (int distance = 0; distance < 300; ++distance) {
			if (screen.at<unsigned char>(y, center + distance) == 255)
				return center + distance;
			if (screen.at<unsigned char>(y, center - distance) == 255)
				return center - distance;
		}
	return -1;
}

void _move(cv::Point i) {
	if (i.x == -1)
		game->makeAction(actions[2], 90);
	if (i.x >360)
		game->makeAction(actions[1]);
	else if (i.x < 280)
		game->makeAction(actions[0]);
	else
		game->makeAction(actions[3]);
}

int distance(const cv::Point2f& a, const cv::Point2f& b) {
	return int(sqrt(pow(abs(a.x - b.x), 2) + pow(abs(a.y - b.y), 2)));
}

void mksquare(cv::Mat& scr, const cv::Point2f& p, const int& channel, const int& minr, const int& maxr) {
	for (int i = -(maxr - 1); i < maxr; ++i) {
		for (int j = -(maxr - 1); j < maxr; ++j) {
			if ((i + p.x > -1) && (i + p.x < 640) && (j + p.y > -1) && (j + p.y < 480)) {
				int dis = distance(p, cvPoint(i + p.x, j + p.y));
				if (dis < maxr && dis > minr)
					scr.at<cv::Vec3b>(j + p.y, i + p.x)[channel] = 255; //red
			}
		}
	}
}

int main() {
	game_init();

	int height = 640;
	int width = 480;
	auto scr = cv::Mat(480, 640, CV_8UC3);
	auto greyscale = cv::Mat(480, 640, CV_8UC1);
	cvNamedWindow("Origin");
	cv::moveWindow("Origin", 80, 30);
	cvNamedWindow("Changed");
	cv::moveWindow("Changed", 720, 30);
	int res = 0;
	int episodes = 10;
	size_t sleepTime = 1000 / DEFAULT_TICRATE;

	for (int i = 0; i < episodes; ++i) {
		game->newEpisode();
		std::cout << "Episode #" << i + 1 << std::endl;
		while (!game->isEpisodeFinished()) {
			auto state = game->getState();
			auto screenBuf = state->screenBuffer;
			
			std::memcpy(scr.data, screenBuf->data(), screenBuf->size());
			cv::extractChannel(scr, greyscale, 2);
			cv::threshold(greyscale, greyscale, 200, 255, cv::THRESH_BINARY);
			greyscale = greyscale(cv::Rect(0, 0, 640, 400));
			std::vector<cv::Point2f> needs_point;
			for (int x = 0; x < (&greyscale)->cols; ++x) {
				for (int y = 0; y < (&greyscale)->rows; ++y)
					if (greyscale.at<unsigned char>(y, x) == 255)
						needs_point.push_back(cv::Point2f(x, y));
			}
			std::vector<cv::Point> centers;
			cv::Mat mat;
			if (needs_point.size() <= 9) {
				_move(cvPoint(-1, -1));
			}
			else {
				cv::kmeans(needs_point, 8, mat, cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 1000, 1.0), 5, cv::KMEANS_RANDOM_CENTERS, centers);
				int l = 999999999, index = -1;
				for (int j = 0; j < centers.size(); ++j) {
					mksquare(scr, centers[j], 2, 4, 8);
					int dis = distance(cvPoint(320, 1000), centers[j]);
					if (dis < l) {
						l = dis;
						index = j;
					}
				}
				//std::cout << centers[index].x << ' ' << centers[index].y << std::endl;
				//cvWaitKey(500);
				mksquare(scr, centers[index], 1, 6, 8);
				_move(centers[index]);
			}

			cv::imshow("Origin", scr);
			cv::imshow("Changed", greyscale);
			greyscale.convertTo(greyscale, CV_32F);
			greyscale.convertTo(greyscale, CV_8UC3);
			greyscale.create(480, 640, CV_8UC1);

			cvWaitKey(sleepTime);
		}

		cvWaitKey(sleepTime * 10);
		std::cout << "Total reward is: " << game->getTotalReward() << std::endl;
		res += game->getTotalReward();
	}
	cvDestroyAllWindows();
	system("pause >nul");
	game->close();
	delete game;
}