/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkUSDiPhASDevice.h"

//#include "mitkUSDiPhASSDKHeader.h"

mitk::USDiPhASDevice::USDiPhASDevice(std::string manufacturer, std::string model)
	: mitk::USDevice(manufacturer, model), m_ControlsProbes(mitk::USDiPhASProbesControls::New(this)),
	m_ControlsBMode(mitk::USDiPhASBModeControls::New(this)),
	m_ControlsDoppler(mitk::USDiPhASDopplerControls::New(this)),
	m_ImageSource(mitk::USDiPhASImageSource::New()),
	m_IsRunning(false);
{
  SetNumberOfOutputs(1);
  SetNthOutput(0, this->MakeOutput(0));
}

mitk::USDiPhASDevice::~USDiPhASDevice()
{
}

std::string mitk::USDiPhASDevice::GetDeviceClass()
{
  return "org.mitk.modules.us.USDiPhASDevice";
}

mitk::USControlInterfaceBMode::Pointer mitk::USDiPhASDevice::GetControlInterfaceBMode()
{
  return m_ControlsBMode.GetPointer();
}

mitk::USControlInterfaceProbes::Pointer mitk::USDiPhASDevice::GetControlInterfaceProbes()
{
  return m_ControlsProbes.GetPointer();
};

mitk::USControlInterfaceDoppler::Pointer mitk::USDiPhASDevice::GetControlInterfaceDoppler()
{
  return m_ControlsDoppler.GetPointer();
};

mitk::USImageSource::Pointer mitk::USDiPhASDevice::GetUSImageSource()
{
  return m_ImageSource.GetPointer();
}

ScanModeNative& mitk::USDiPhASDevice::GetScanMode()
{
  return m_ScanMode;
}

// Setup and Cleanup

bool mitk::USDiPhASDevice::OnInitialization()
{
  MITK_INFO<<"I was initialized";
  return true;
}

bool mitk::USDiPhASDevice::OnConnection()
{
  static auto WrapMessageCallback = [this](const char* message)->void{ this->StringMessageCallback(message); };
  static auto WrapImageDataCallback = [this](
      short* rfDataChannelData, int channelDatalinesPerDataset, int channelDataSamplesPerLine, int channelDataTotalDatasets,
      short* rfDataArrayBeamformed, int beamformedLines, int beamformedSamples, int beamformedTotalDatasets,
      unsigned char* imageData, int imageWidth, int imageHeight, int imagePixelFormat, int imageSetsTotal, double timeStamp)->void
    { 
      this->m_ImageSource->ImageDataCallback(
        rfDataChannelData, channelDatalinesPerDataset, channelDataSamplesPerLine, channelDataTotalDatasets,
        rfDataArrayBeamformed, beamformedLines, beamformedSamples, beamformedTotalDatasets,
        imageData, imageWidth, imageHeight, imagePixelFormat, imageSetsTotal, timeStamp);
	};
  // Need those forwarders to call member functions; createBeamformer expects non-member function pointers. 
  // "static" is needed to ensure those methods remain in memory

  createBeamformer((StringMessageCallback)&WrapMessageCallback, (NewDataCallback)&WrapImageDataCallback);
  initBeamformer();
  this->InitializeScanMode();
  
  // pass the new scanmode to the device:
  setupScan(m_ScanMode);
  m_IsRunning = toggleFreeze(); //start scanning
  
  return true;
}

bool mitk::USDiPhASDevice::OnDisconnection()
{
  //close the beamformer so hardware is disconnected
  closeBeamformer();
  return true;
}

bool mitk::USDiPhASDevice::OnActivation()
{
  if(!m_IsRunning)
    m_IsRunning=toggleFreeze();
  MITK_INFO<< "I was activated";
  return true;
}

bool mitk::USDiPhASDevice::OnDeactivation()
{
  if(m_IsRunning)
    m_IsRunning=toggleFreeze();
  MITK_INFO<< "I was deactivated";
  return true;
}

void mitk::USDiPhASDevice::OnFreeze(bool freeze)
{
  if(m_IsRunning==freeze)
    m_IsRunning=toggleFreeze(); // toggleFreeze() returns true if it starts running the beamformer, otherwise false
}

void mitk::USDiPhASDevice::InitializeScanMode()
{
	// initialize the ScanMode to be used for measurements:
	m_ScanMode.scanModeName = "TestMode";

	// configure a linear transducer
	m_ScanMode.transducerName = "L2-7 ";
	m_ScanMode.transducerCurvedRadiusMeter = 0;
	m_ScanMode.transducerElementCount = 128;
	m_ScanMode.transducerFrequencyHz = 5000000;
	m_ScanMode.transducerPitchMeter = 0.0003f;
	m_ScanMode.transducerType = 1;

	// configure the transmit sequence(s):
	int selectedChannelOn = 63;
	int numChannels = 128;
	m_ScanMode.transmitEventsCount = 1;
	int totalNumberOfDelays = numChannels * m_ScanMode.transmitEventsCount;
	m_ScanMode.transmitEventsDelays = new float[totalNumberOfDelays];
	m_ScanMode.BurstHalfwaveClockCountPerChannel = new int[numChannels];
	m_ScanMode.BurstCountPerChannel = new int[numChannels];
	m_ScanMode.BurstUseNegativePolarityPerChannel = new bool[numChannels];
	for (int i = 0; i < numChannels; ++i)
	{
		if (i == selectedChannelOn)
		{
			m_ScanMode.BurstHalfwaveClockCountPerChannel[i] = 12; // 5MHz
			m_ScanMode.BurstCountPerChannel[i] = 1;
			m_ScanMode.BurstUseNegativePolarityPerChannel[i] = true;
			m_ScanMode.transmitEventsDelays[i] = 0;
		}
		else
		{
			m_ScanMode.BurstHalfwaveClockCountPerChannel[i] = 0; // 5MHz
			m_ScanMode.BurstCountPerChannel[i] = 1;
			m_ScanMode.BurstUseNegativePolarityPerChannel[i] = true;
			m_ScanMode.transmitEventsDelays[i] = 0;
		}
	}

	m_ScanMode.transmitPhaseLengthSeconds = 1e-6f;
	m_ScanMode.voltageV = 70;

	// configure the receive paramters:
	m_ScanMode.receivePhaseLengthSeconds = 65e-6f;
	m_ScanMode.tgcdB = new unsigned char[8];
	for (int tgc = 0; tgc < 8; ++tgc)
		m_ScanMode.tgcdB[tgc] = tgc * 2 + 10;
	m_ScanMode.accumulation = 1;
	m_ScanMode.bandpassApply = false;
	m_ScanMode.averagingCount = 1;

	// configure general processing:
	m_ScanMode.transferChannelData = true;

	// configure reconstruction processing:
	m_ScanMode.averageSpeedOfSound = 1540;
	m_ScanMode.computeBeamforming = false;
	m_ScanMode.beamformingAlgorithm = 0; //  (int)Beamforming::PlaneWaveCompound;
	/*BeamformingParametersPlaneWaveCompound parameters;
	parameters.usePhaseCoherence = 0;
	parameters.SpeedOfSoundMeterPerSecond = 1540;
	m_ScanMode.beamformingAlgorithmParameters = (void*)&parameters;*/
	m_ScanMode.reconstructedLinePitchMmOrAngleDegree = 0.3f;
	m_ScanMode.reconstructionLines = 128;
	m_ScanMode.reconstructionSamplesPerLine = 2048;
	m_ScanMode.transferBeamformedData = false;

	// configure image generation:
	m_ScanMode.imageWidth = 512;
	m_ScanMode.imageHeight = 512;
	m_ScanMode.imageMultiplier = 1;
	m_ScanMode.imageLeveling = 0;
	m_ScanMode.transferImageData = true;
}

// callback for the DiPhAS API 

void mitk::USDiPhASDevice::StringMessageCallback(const char* message)
{
	MITK_INFO << "DiPhAS API: " << message << '\n';
}

