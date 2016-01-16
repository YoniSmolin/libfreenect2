#ifndef TIMER_HPP
#define TIMER_HPP

#include <opencv2/opencv.hpp>
#include <stdio.h>

using namespace std;
using namespace cv;

class Timer
{
	public:
		Timer(const char** sectionNames, int sectionCount);
		~Timer();

		void FrameStart();
		void SectionEnd();
		double GetCurrentTime();

	private:
		int _numberOfSections;
		int _numberOfFrames;
		int _currentSection;
		const char** _sectionNames;

		double _startTime;
		double _previousTime;
		double* _sectionDurations;
		double _clockFrequency;
		
		FILE* _logFile;
};

#endif
