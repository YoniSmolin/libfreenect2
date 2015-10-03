#ifndef TIMER_HPP
#define TIMER_HPP

#include <opencv2/opencv.hpp>

using namespace std;

class Timer
{
	public:
		Timer(char** argv, int argc, int frameWindowSize);

		void FrameStart();
		void SectionEnd();
		float* AverageLatencies();

	private:
		double _startOfFrame;
		double* _sectionEndTimes;
		int _numberOfSections;
		int _frameWindowSize;
}

#endif
