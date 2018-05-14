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
  m_IsRunning(false),
  m_BurstHalfwaveClockCount(7),
  m_Interleaved(true)  
{
  m_NumberOfOutputs = 2;
  this->SetNumberOfIndexedOutputs(m_NumberOfOutputs);

  SetNthOutput(0, this->MakeOutput(0));
  SetNthOutput(1, this->MakeOutput(1));
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
  unsigned char* imageData, int imageWidth, int imageHeight, int imagePixelFormat, int imageSetsTotal, double timeStamp)
{
	 w_ISource->ImageDataCallback(
     rfDataChannelData, channelDatalinesPerDataset, channelDataSamplesPerChannel, channelDataTotalDatasets,
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

  InitializeScanMode();
  initBeamformer();                     //start the hardware connection

  m_ImageSource->UpdateImageGeometry(); //make sure the image geometry is initialized!
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
  OnFreeze(true);
  SetInterleaved(m_Interleaved); // update the beamforming parameters...
  UpdateTransmitEvents();

  if (!(dynamic_cast<mitk::USDiPhASCustomControls*>(this->m_ControlInterfaceCustom.GetPointer())->GetSilentUpdate()))
  {
    setupScan(this->m_ScanMode);
    m_ImageSource->UpdateImageGeometry();
  }

  OnFreeze(false);
}

void mitk::USDiPhASDevice::UpdateTransmitEvents()
{
  int numChannels = m_ScanMode.reconstructionLines;

  // transmitEventsCount defines only the number of acoustic measurements (angles); there will be one event added to the start for OA measurement
  m_ScanMode.TransmitEvents = new TransmitEventNative[m_ScanMode.transmitEventsCount];

  for (int ev = 0; ev < m_ScanMode.transmitEventsCount; ++ev)
  {
    m_ScanMode.TransmitEvents[ev].transmitEventDelays = new float[numChannels];
    m_ScanMode.TransmitEvents[ev].BurstHalfwaveClockCountPerChannel = new int[numChannels];
    m_ScanMode.TransmitEvents[ev].BurstCountPerChannel = new int[numChannels];
    m_ScanMode.TransmitEvents[ev].BurstUseNegativePolarityPerChannel = new bool[numChannels];
    m_ScanMode.TransmitEvents[ev].ChannelMultiplexerSetups = nullptr;
    float tiltStrength = ((m_ScanMode.transmitEventsCount - 1) / 2 - ev) * 20e-9f;

    for (int i = 0; i < numChannels; ++i)
    {
      m_ScanMode.TransmitEvents[ev].BurstHalfwaveClockCountPerChannel[i] = m_BurstHalfwaveClockCount; // 120 MHz / (2 * (predefinedBurstHalfwaveClockCount + 1)) --> 7.5 MHz 
      m_ScanMode.TransmitEvents[ev].BurstCountPerChannel[i] = 1; // Burst with 1 cycle
      m_ScanMode.TransmitEvents[ev].BurstUseNegativePolarityPerChannel[i] = true;
      m_ScanMode.TransmitEvents[ev].transmitEventDelays[i] = 2e-6f + (i - numChannels / 2) * tiltStrength;
    }
  }

  m_ScanMode.transmitSequenceCount = 1;
  m_ScanMode.transmitSequences = new SequenceNative[m_ScanMode.transmitSequenceCount];
  m_ScanMode.transmitSequences[0].startEvent = 0;
  m_ScanMode.transmitSequences[0].endEvent = m_ScanMode.transmitEventsCount;
}



void mitk::USDiPhASDevice::InitializeScanMode()
{
  // create a scanmode to be used for measurements:
  m_ScanMode.scanModeName = "InterleavedMode";

  // configure a linear transducer
  m_ScanMode.transducerName = "L5-10";
  m_ScanMode.transducerCurvedRadiusMeter = 0;
  m_ScanMode.transducerElementCount = 128;
  m_ScanMode.transducerFrequencyHz = 7500000;
  m_ScanMode.transducerPitchMeter = 0.0003f;
  m_ScanMode.transducerType = 1;

  // configure the receive paramters:
  m_ScanMode.receivePhaseLengthSeconds = 185e-6f; // about 15 cm imaging depth
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
  m_ScanMode.computeBeamforming = true;

  // setup beamforming parameters:
  SetInterleaved(true);

  m_ScanMode.reconstructedLinePitchMmOrAngleDegree = 0.3f;
  m_ScanMode.reconstructionLines = 128;
  m_ScanMode.reconstructionSamplesPerLine = 2048;
  m_ScanMode.transferBeamformedData = true;

  // configure the transmit sequence(s):
  m_ScanMode.transmitEventsCount = 1;
  m_ScanMode.transmitPhaseLengthSeconds = 1e-6f;
  m_ScanMode.voltageV = 75;
  UpdateTransmitEvents();

  // configure bandpass:
  m_ScanMode.bandpassApply = false;
  m_ScanMode.bandpassFrequencyLowHz = 1e6f;
  m_ScanMode.bandpassFrequencyHighHz = 20e6f;

  // configure image generation:
  m_ScanMode.imageWidth = 512;
  m_ScanMode.imageHeight = 512;
  m_ScanMode.imageMultiplier = 1;
  m_ScanMode.imageLeveling = 0;
  m_ScanMode.transferImageData = false;

  // Trigger setup:
  m_ScanMode.triggerSetup.enabled = true;
  m_ScanMode.triggerSetup.constantPulseRepetitionRateHz = 20;
  m_ScanMode.triggerSetup.triggerWidthMicroseconds = 15;
  m_ScanMode.triggerSetup.delayTrigger2Microseconds = 300;
}

// callback for the DiPhAS API 

void mitk::USDiPhASDevice::MessageCallback(const char* message)
{
	MITK_INFO << "DiPhAS API: " << message << '\n';
}

void mitk::USDiPhASDevice::SetBursts(int bursts)
{
  m_BurstHalfwaveClockCount = bursts;
}

bool mitk::USDiPhASDevice::IsInterleaved()
{
  return m_Interleaved;
}

void mitk::USDiPhASDevice::SetInterleaved(bool interleaved)
{
  m_Interleaved = interleaved;
  if (interleaved) {
    m_ScanMode.scanModeName = "Interleaved Beamforming Mode";
    m_CurrentBeamformingAlgorithm = Beamforming::Interleaved_OA_US;

    paramsInterleaved.SpeedOfSoundMeterPerSecond = m_ScanMode.averageSpeedOfSound;
    paramsInterleaved.angleSkipFactor = 1;
    paramsInterleaved.OptoacousticDelay = 0.0000003; // 300ns
    paramsInterleaved.filter = Filter::None;

    m_ScanMode.beamformingAlgorithmParameters = &paramsInterleaved;
  }
  else {
    m_ScanMode.scanModeName = "Plane Wave Beamforming Mode";
    m_CurrentBeamformingAlgorithm = Beamforming::PlaneWaveCompound;

    paramsPlaneWave.SpeedOfSoundMeterPerSecond = m_ScanMode.averageSpeedOfSound;
    paramsPlaneWave.angleSkipFactor = 0;
    paramsPlaneWave.usePhaseCoherence = 0;

    m_ScanMode.beamformingAlgorithmParameters = &paramsPlaneWave;
  }
  m_ScanMode.beamformingAlgorithm = m_CurrentBeamformingAlgorithm;
}