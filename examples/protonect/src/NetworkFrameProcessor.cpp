#include <networking/NetworkFrameProcessor.hpp>
#include <stdexcept>
#include <cassert>
#include <iostream>

#include <CUDA/Filter.h>

#include <opencv2/opencv.hpp>
#include <opencv2/contrib/contrib.hpp>
#include <opencv2/highgui/highgui.hpp> // PNG compression

#define IR_MAX 20000.0f
#define IR_DYNAMIC_RANGE 255.0f

using namespace libfreenect2;

namespace Networking
{
	std::map<int, unsigned int> NetworkFrameProcessor::CreateMap()
	{
		std::map<int, unsigned int> result;
		result[CV_8UC1] = 255;
		result[CV_16UC1] = 65535;
		return result;
	}

	const std::map<int, unsigned int> NetworkFrameProcessor::_pixelTypeToPixelMaxValue = CreateMap();

	NetworkFrameProcessor::NetworkFrameProcessor(ChannelProperties* properties) : _properties(properties), _colorCompressor(NULL) 
 	{
		switch (_properties->Type)
		{
			case Frame::Color: 	{
							ColorProperties* colorProperties = dynamic_cast<ColorProperties*> (_properties);
							_colorCompressor = new JpegCompressor(colorProperties->ImageHeight, // / pColorProperties->ImageShrinkFactor, 
											      colorProperties->ImageWidth, // / pColorProperties->ImageShrinkFactor,
											      colorProperties->CompressionQuality);
						}
						break;
			case Frame::Depth:    	break;	
		};
	}

	NetworkFrameProcessor::~NetworkFrameProcessor()
	{
		if (_properties->Type == Frame::Color) 
			delete _colorCompressor;
	}

	NetworkPacket NetworkFrameProcessor::ProcessFrame(const Frame* frame)
	{
		switch (_properties->Type)
		{
			case Frame::Color: return ProcessColorFrame(frame);
			case Frame::Ir: return ProcessIrFrame(frame); 
			case Frame::Depth: return ProcessDepthFrame(frame);
			default: throw std::runtime_error("Could not identify the type of frame");
		}
	}

	NetworkPacket NetworkFrameProcessor::ProcessColorFrame(const Frame* frame)
	{
		ColorProperties* props = dynamic_cast<ColorProperties*> (_properties);
		// no height/width assertion here because frame->data's runtime type is unsigned char**, thus width = height = 1
		assert(frame->height == 1 && frame->width == 1);

		unsigned char** pprgba = reinterpret_cast<unsigned char **>(frame->data);

		//cv::Mat original(pColorProperties->ImageHeight, pColorProperties->ImageWidth, CV_8UC4, pprgba[0]);
		//cv::Mat shrinked(pColorProperties->ImageHeight / pColorProperties->ImageShrinkFactor, pColorProperties->ImageWidth / pColorProperties->ImageShrinkFactor, CV_8UC4); 	
		//cv::resize(original, shrinked, shrinked.size(), 0, 0, CV_INTER_AREA);
		int compressedSize = _colorCompressor->Compress(pprgba[0]); //shrinked.data);

		NetworkPacket result = {props->Type, compressedSize, (char*)_colorCompressor->GetCompressed()};

		return result;
	}

	NetworkPacket NetworkFrameProcessor::ProcessIrFrame(const Frame* frame)
	{
 		IrProperties* props = dynamic_cast<IrProperties*> (_properties);
		assert(frame->height == props->ImageHeight && frame->width == props->ImageWidth);

		cv::Mat irMat(frame->height, frame->width, CV_32FC1, frame->data);
		
		irMat.convertTo(irMat, props->PixelType, _pixelTypeToPixelMaxValue.find(props->PixelType)->second / props->InputSaturationLevel , 0);

		imencode(".png", irMat, _compressedPNG);	

		NetworkPacket result = {props->Type, _compressedPNG.size(), (char*)&_compressedPNG[0]};

		return result;
	}

	NetworkPacket NetworkFrameProcessor::ProcessDepthFrame(const Frame* frame)
	{
		DepthProperties* props = dynamic_cast<DepthProperties*> (_properties);
		assert(frame->height == props->ImageHeight && frame->width == props->ImageWidth);

		cv::Mat depthMat(frame->height, frame->width, CV_32FC1, frame->data);

		// if desired - CUDA filterting can be performed at this stage
		
		// we need to convert to an unsigned char format for the PNG compression (but a single byte is not enough to represent a depth pixel, so we use 2 bytes)
		depthMat.convertTo(depthMat, props->PixelType, 1 / props->Resolution , 0);

		imencode(".png", depthMat, _compressedPNG);	

		NetworkPacket result = {props->Type, _compressedPNG.size(), (char*)&_compressedPNG[0]};

		return result;
	}
}
