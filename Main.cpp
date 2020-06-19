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
	game->setScreenResolution(RES_640X480); // разрешение
	game->setLabelsBufferEnabled(1); // add this
	game->setWindowVisible(1); // exception with Linux without X Series
	game->setRenderWeapon(1); // is the gun will be in the game
	game->setRenderHud(1);
	game->init();
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

int main() {
	game_init();

	int height = 620;
	int width = 440;
	// задаЄм точку дл€ вывода текста
	CvPoint pt = cvPoint(height / 4, width / 2);
	// —оздаЄи 8-битную, 3-канальную картинку
	auto greyscale = cv::Mat(480, 640, CV_8UC3);
	cvNamedWindow("original", CV_WINDOW_AUTOSIZE);

	int episodes = 10;
	size_t sleepTime = 1000 / DEFAULT_TICRATE;
	for (int i = 0; i < episodes; ++i) {
		std::cout << "Episode #" << i + 1 << "\n";
		game->newEpisode();
		while (!game->isEpisodeFinished()) {
			auto state = game->getState();
			auto screenBuf = state->screenBuffer;

			find_demon_and_kill(state);
			std::memcpy(greyscale.data, state->screenBuffer->data(), state->screenBuffer->size());
			cv::extractChannel(greyscale, greyscale, 2);
			cv::threshold(greyscale, greyscale, 190, 255, cv::THRESH_BINARY);
			cv::imshow("original", greyscale);
			greyscale = cv::Mat(480, 640, CV_8UC3);

			//sleep(sleepTime);
			cvWaitKey(sleepTime);
		}
		sleep(sleepTime * 10);
		std::cout << "Total reward is: " << game->getTotalReward() << std::endl;
	}
	game->close();
	delete game;
	cvDestroyWindow("original");
}
//#include <iostream>
//#include <opencv2/opencv.hpp>
//#include <stdlib.h>
//
//#define CV_BLUR_NO_SCALE 0
//#define CV_BLUR  1
//#define CV_GAUSSIAN  2
//#define CV_MEDIAN 3
//#define CV_BILATERAL 4
//
//using namespace cv;
//using namespace std;
//
//const char* catImage = "./assets/cat.jpg";
//const char* eyeImage = "./assets/eye.jpg";
//
//void main() {
//	// задаЄм высоту и ширину картинки
//	int height = 620;
//	int width = 440;
//	// задаЄм точку дл€ вывода текста
//	CvPoint pt = cvPoint(height / 4, width / 2);
//	// —оздаЄи 8-битную, 3-канальную картинку
//	IplImage* hw = cvCreateImage(cvSize(height, width), 8, 3);
//	// заливаем картинку чЄрным цветом
//	cvSet(hw, cvScalar(0, 0, 0));
//	// инициализаци€ шрифта
//	CvFont font;
//	cvInitFont(&font, CV_FONT_HERSHEY_COMPLEX, 1.0, 1.0, 0, 1, CV_AA);
//	// использу€ шрифт выводим на картинку текст
//	cvPutText(hw, "Hello, OpenCV", pt, &font, CV_RGB(255, 255, 255));
//	// создаЄм окошко
//	cvNamedWindow("Hello World", CV_WINDOW_AUTOSIZE);
//	// показываем картинку в созданном окне
//	cvShowImage("Hello World", hw);
//	// ждЄм нажати€ клавиши
//	char c = cvWaitKey(0);
//	if (c == 27) { // если нажали ESC 
//		std::cout << "ESC pressed" << std::endl;
//	}
//	// освобождаем ресурсы
//	cvReleaseImage(&hw);
//	cvDestroyWindow("Hello World");
//}
