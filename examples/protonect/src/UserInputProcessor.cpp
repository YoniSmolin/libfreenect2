#include <networking/UserInputProcessor.hpp>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <iostream>

namespace Networking
{
	bool UserInputProcessor::TryParse(int argc, const char** argv)
	{
		if (argc < 2) return false;

		if (0 == std::strcmp(argv[1], "Color")) 
		{
			return TryParseColor(argc - 2, argv + 2);
		} 	
		else if (0 == std::strcmp(argv[1], "IR")) 
		{
			return TryParseIr(argc - 2, argv + 2);
		}
		else if (0 == std::strcmp(argv[1], "Depth")) 
		{
			return TryParseDepth(argc - 2, argv + 2);
		}	
		else
		{
			return false;
		}

		return true;
	};

	UserInputProcessor::~UserInputProcessor()
	{
		if (_properties) delete _properties;
	}

	ChannelProperties* UserInputProcessor::GetProperties()
	{
		return _properties;
	}
	
	void UserInputProcessor::PrintUsage(const char* programName)
	{
		ColorProperties color;
		IrProperties ir;
		DepthProperties depth;
		std::cout << programName << " usage:" << std::endl 
			  << programName << " Color TimeToRun [" << color.TimeToRun << "] ShrinkCoefficient [" << color.ImageShrinkFactor << "] JpegQuality [" << color.CompressionQuality << "]" << std::endl
			  << "or" << std::endl
			  << programName << " Depth TimeToRun [" << depth.TimeToRun << "] NearThreshold [" << depth.NearThreshold << "] FarThreshold [" << depth.FarThreshold << "] Resolution [" << depth.Resolution << "]" << std::endl
			  << "or" << std::endl
			  << programName << " IR TimeToRun [" << ir.TimeToRun << "] InputSaturationValue [" << ir.InputSaturationLevel << "]" << std::endl;
	}
	
	bool UserInputProcessor::TryParseColor(int argc, const char** argv)
	{
		if (argc > 3) return false;

		if (_properties) delete _properties;
		
		ColorProperties* colorProps = new ColorProperties();

		switch(argc)
		{
			case 3: colorProps->CompressionQuality = (unsigned int) atoi(argv[2]);
			case 2: colorProps->ImageShrinkFactor = (unsigned int) atoi(argv[1]);
			case 1: colorProps->TimeToRun = (unsigned int) atoi(argv[0]);
			default: break;
		};

		if (colorProps->ImageShrinkFactor == 0 || colorProps->CompressionQuality == 0 || colorProps->CompressionQuality > 100)
		{
			delete colorProps;
			return false;
		}
		
		_properties = colorProps;

		return true;
	}

	bool UserInputProcessor::TryParseIr(int argc, const char** argv)
	{
		if (argc > 2) return false;

		if (_properties) delete _properties;

		IrProperties* irProps = new IrProperties();

		switch(argc)
		{
			case 2: irProps->InputSaturationLevel = atof(argv[1]);
			case 1: irProps->TimeToRun =  (unsigned int) atoi(argv[0]);
			default: break;
		};

		if (irProps->InputSaturationLevel <= 0)
		{
			delete irProps;
			return false;
		}

		_properties = irProps;		

		return true;
	}

	bool UserInputProcessor::TryParseDepth(int argc, const char** argv)
	{
		if (argc > 4) return false;

		if (_properties) delete _properties;
		
		DepthProperties* depthProps = new DepthProperties();
		
		switch(argc)
		{
			case 4: depthProps->Resolution = atof(argv[3]);
			case 3:	depthProps->FarThreshold = atof(argv[2]);
			case 2: depthProps->NearThreshold = atof(argv[1]);
			case 1: depthProps->TimeToRun =  (unsigned int) atoi(argv[0]);
			default: break;
		};

		if (depthProps->Resolution <= 0 || depthProps->NearThreshold <= 0 || depthProps->FarThreshold <= 0)
		{
			delete depthProps;
			return false;
		}

		_properties = depthProps;

		return true;
	}
}
