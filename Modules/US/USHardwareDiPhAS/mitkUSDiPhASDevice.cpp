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
#include "mitkUSDiPhASCustomControls.h"

mitk::USDiPhASDevice::USDiPhASDevice(std::string manufacturer, std::string model)
	: mitk::USDevice(manufacturer, model), m_ControlsProbes(mitk::USDiPhASProbesControls::New(this)),
	m_ImageSource(mitk::USDiPhASImageSource::New(this)),
  m_ControlInterfaceCustom(mitk::USDiPhASCustomControls::New(this)),
	m_IsRunning(false)
{
  SetNumberOfOutputs(1);
  SetNthOutput(0, this->MakeOutput(0));
}

mitk::USDiPhASDevice::~USDiPhASDevice()
{
}

//Gets

std::string mitk::USDiPhASDevice::GetDeviceClass()
{
  return "org.mitk.modules.us.USDiPhASDevice";
}

mitk::USControlInterfaceProbes::Pointer mitk::USDiPhASDevice::GetControlInterfaceProbes()
{
  return m_ControlsProbes.GetPointer();
};

mitk::USAbstractControlInterface::Pointer mitk::USDiPhASDevice::GetControlInterfaceCustom()
{
  return m_ControlInterfaceCustom.GetPointer();
}

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
  return true;
}

//----------------------------------------------------------------------------------------------------------------------------
/* ugly wrapper stuff - find better solution so it isn't necessary to create a global pointer to USDiPhASDevice...
 * passing a lambda function would be nicer - sadly something goes wrong when passing the adress of a lambda function:
 * the API produces access violations. Passing the Lambda function itself would be preferable, but that's not possible
*/

mitk::USDiPhASDevice* w_device;
mitk::USDiPhASImageSource* w_ISource;

void WrapperMessageCallback(const char* message) 
{
  w_device->MessageCallback(message);
}

void WrapperImageDataCallback(
	short* rfDataChannelData, int channelDatalinesPerDataset, int channelDataSamplesPerChannel, int channelDataTotalDatasets,
	short* rfDataArrayBeamformed, int beamformedLines, int beamformedSamples, int beamformedTotalDatasets,
	unsigned char* imageData, int imageWidth, int imageHeight, int imagePixelFormat, int imageSetsTotal,

	double timeStamp)
{
	 w_ISource->ImageDataCallback(
		rfDataChannelData, channelDatalinesPerDataset, channelDatalinesPerDataset, channelDataTotalDatasets,
		rfDataArrayBeamformed, beamformedLines, beamformedSamples, beamformedTotalDatasets,
		imageData, imageWidth, imageHeight, imagePixelFormat, imageSetsTotal, timeStamp);
}

//----------------------------------------------------------------------------------------------------------------------------

bool mitk::USDiPhASDevice::OnConnection()
{
  w_device = this;
  w_ISource = m_ImageSource; 
  // Need those pointers for the forwarders to call member functions; createBeamformer expects non-member function pointers. 
  createBeamformer((StringMessageCallback)&WrapperMessageCallback, (NewDataCallback)&WrapperImageDataCallback);

  initBeamformer();
  this->InitializeScanMode();
  
  // pass the new scanmode to the device:
  setupScan(this->m_ScanMode);

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
  // probe controls are available now
  m_ControlsProbes->SetIsActive(true);

  if (m_ControlsProbes->GetProbesCount() < 1)
  {
	  MITK_WARN("USDevice")("USDiPhASDevice") << "No probe found.";
	  return false;
  }

  m_ControlsProbes->SelectProbe(0);

  // toggle the beamformer of the API
  if(!m_IsRunning)
    m_IsRunning=toggleFreeze();
  return true;
}

bool mitk::USDiPhASDevice::OnDeactivation()
{
  if(m_IsRunning)
    m_IsRunning=toggleFreeze();
  return true;
}

void mitk::USDiPhASDevice::OnFreeze(bool freeze)
{
  if(m_IsRunning==freeze)
    m_IsRunning=toggleFreeze(); // toggleFreeze() returns true if it starts running the beamformer, otherwise false
}

void mitk::USDiPhASDevice::UpdateScanmode()
{
  if (m_IsRunning)
    m_IsRunning = toggleFreeze();

  m_ScanMode.imageWidth = m_ScanMode.reconstructionLines;
  m_ScanMode.imageHeight = m_ScanMode.reconstructionSamplesPerLine;
  // a higher resolution is useless, this also ensures correct spacing using any data types

  setupScan(this->m_ScanMode);
  m_ImageSource->UpdateImageGeometry();
  if (!m_IsRunning)
    m_IsRunning = toggleFreeze();
}

void mitk::USDiPhASDevice::InitializeScanMode()
{
	m_ScanMode.scanModeName = "UltrasoundMode";

	// configure a linear transducer
	m_ScanMode.transducerName = "L2-7 ";
	m_ScanMode.transducerCurvedRadiusMeter = 0;
	m_ScanMode.transducerElementCount = 128;
	m_ScanMode.transducerFrequencyHz = 5000000;
	m_ScanMode.transducerPitchMeter = 0.0003f;
	m_ScanMode.transducerType = 1;

	// configure the transmit sequence(s):
	int numChannels = 128;
	m_ScanMode.transmitEventsCount = 1;
	m_ScanMode.BurstHalfwaveClockCountAllChannels = 11; // 120 MHz / (2 * (predefinedBurstHalfwaveClockCount + 1)) --> 5 MHz 
	m_ScanMode.transmitPhaseLengthSeconds = 1e-6f;
	m_ScanMode.voltageV = 70;

	// configure the receive paramters:
	m_ScanMode.receivePhaseLengthSeconds = 65e-6f; // 5 cm imaging depth
	m_ScanMode.tgcdB = new unsigned char[8];
	for (int tgc = 0; tgc < 8; ++tgc)
		m_ScanMode.tgcdB[tgc] = tgc * 2 + 10;
	m_ScanMode.accumulation = 1;
	m_ScanMode.bandpassApply = false;
	m_ScanMode.averagingCount = 1;

	// configure general processing:
	m_ScanMode.transferChannelData = false;

	// configure reconstruction processing:
	m_ScanMode.averageSpeedOfSound = 1540;
  m_ScanMode.computeBeamforming = true;
	m_ScanMode.beamformingAlgorithm = (int)Beamforming::PlaneWaveCompound;

	// setup beamforming parameters:
	BeamformingParametersPlaneWaveCompound parametersPW;
	parametersPW.SpeedOfSoundMeterPerSecond = 1540;
	parametersPW.angleSkipFactor = 1;
	m_ScanMode.beamformingAlgorithmParameters = (void*)&parametersPW;

	m_ScanMode.reconstructedLinePitchMmOrAngleDegree = 0.3f;
	m_ScanMode.reconstructionLines = 128;
	m_ScanMode.reconstructionSamplesPerLine = 2048;
	m_ScanMode.transferBeamformedData = false;

	// configure bandpass:
	m_ScanMode.bandpassApply = false;
  m_ScanMode.bandpassFrequencyLowHz = 0;
  m_ScanMode.bandpassFrequencyHighHz = 5e6f;

	// configure image generation:
  m_ScanMode.imageWidth = m_ScanMode.reconstructionLines;
  m_ScanMode.imageHeight = m_ScanMode.reconstructionSamplesPerLine;
	m_ScanMode.imageMultiplier = 1;
	m_ScanMode.imageLeveling = 0;
	m_ScanMode.transferImageData = true;
}

// callback for the DiPhAS API 

void mitk::USDiPhASDevice::MessageCallback(const char* message)
{
	MITK_INFO << "DiPhAS API: " << message << '\n';
}

