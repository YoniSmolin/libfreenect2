#include <libfreenect2/Timer.hpp>

#define MILISECONDS_IN_SECOND 1000

Timer::Timer(const char** sectionNames, int sectionCount, int frameWindowSize) : _sectionNames(sectionNames), _numberOfSections(sectionCount), _frameWindowSize(frameWindowSize), _startTime(0)
{
	_logFile = fopen("ProfileLog.txt", "w");

	_clockFrequency = getTickFrequency();
	_sectionDurations = new double[sectionCount];
	memset(_sectionDurations, 0, sectionCount);
}

Timer::~Timer()
{
	fclose(_logFile);
}

void Timer::FrameStart()
{
	_previousTime = getTickCount();
	if (_startTime == 0) // first time this method is called
		_startTime = _previousTime;
	_currentSection = 0;

	for (int i = 0; i < _numberOfSections; i++)
		_sectionDurations[i] = 0;

	fprintf(_logFile, "******* Frame #%d *********\n", _numberOfFrames+1);
}

void Timer::SectionEnd()
{
	double currentTime = getTickCount();
	_sectionDurations[_currentSection++] += (currentTime - _previousTime) / _clockFrequency;
	_previousTime = currentTime;

	if (_currentSection == _numberOfSections)
	{
		_numberOfFrames++;

		for (int i = 0; i < _numberOfSections; i++ )
			fprintf(_logFile, "%s: %2.2f\n", _sectionNames[i], MILISECONDS_IN_SECOND*_sectionDurations[i]);
	}
}

double Timer::GetCurrentTime()
{
	return (getTickCount() - _startTime) / _clockFrequency;
}
