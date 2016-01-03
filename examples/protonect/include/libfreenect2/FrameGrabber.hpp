#ifndef FRAME_GRABBER_HPP_
#define FRAME_GRABBER_HPP_

#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>

namespace libfreenect2
{
	class FrameGrabber
	{
		Frame::Type _type; // Ir, Color or Depth

		Freenect2 _freenect2;
		Freenect2Device* _kinectDevice;
		SyncMultiFrameListener _frameListener;
		FrameMap _frameMap;	

		bool _verbose;

	public:
		FrameGrabber(Frame::Type, bool verbose);
		
		const Frame* GrabFrame();
		void ReleaseFrame();
		
		~FrameGrabber();
	private:
		void PrintIntrinsics();
	};
}
#endif
