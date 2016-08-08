// Framework.IBMT.US.CWrapper.h :
// The DiPhAS C Wrapper that enables C++ programs and libraries to use the .NET DiPhAS components.
// It includes functions to initializes the beamformer, apply parameter sets to the beamformer
// operation and control the beamformer scanning state. 
// It defines delegate functions for internal messages and data to be passed to outside Dll or application
// code
//
// Written by the IBMT Beamformer Team: support-diphas@ibmt.fraunhofer.de
// Copyright Fraunhofer IBMT

#ifdef WIN32
#    define CWRAPPERAPI extern "C" __declspec(dllexport)
#else
#    define CWRAPPERAPI 
#endif

#pragma once

#include <string>

//namespace IBMT_US_DiPhAS
//{

	// This is a function that will be called by the beamformer asynchronously to this program
	// It passes status messages to the user
	typedef void(__cdecl *StringMessageCallback)(const char* message);

	// This is a function that will be called by the beamformer asynchronously to this program
	// It passes the pre-beamformed rf data, beamformed rf data and/or scanconverted image data to the user
	// If multiple pre-beamformed rf datasets are used to generate a beamformed datasets all recorded data is included here
	typedef void(__cdecl *NewDataCallback)(
		short* rfDataChannelData,
		int channelDatalinesPerDataset,
		int channelDataSamplesPerLine,
		int channelDataTotalDatasets,

		short* rfDataArrayBeamformed,
		int beamformedLines,
		int beamformedSamples,
		int beamformedTotalDatasets,  // for sequence mode (or interleaved) we will get multiple rf data sets at once depending on beamforming algorithms

		unsigned char* imageData,
		int imageWidth,
		int imageHeight,
		int imagePixelFormat,
		int imageSetsTotal, // for sequence mode (or interleaved) we will get multiple images at once depending on beamforming algorithms
		double timeStamp);

	typedef enum
	{
		PlaneWaveCompound = 1,
		SphericalReconstruction = 2,
		DelayAndSumLineBased = 3,
		VirtualPointSource = 4,
		Interleaved_OA_US = 5
	} Beamforming;

	typedef struct
	{
		float usePhaseCoherence;
		int SpeedOfSoundMeterPerSecond;
		int angleSkipFactor;
	} BeamformingParametersPlaneWaveCompound;

	typedef struct
	{
		int SpeedOfSoundMeterPerSecond;
		int angleSkipFactor;
	} BeamformingParametersInterleaved_OA_US;

	typedef struct
	{
		int startEvent;
		int endEvent;
	} SequenceNative;

	typedef struct
	{
		bool enabled;
		int triggerWidthMicroseconds;
		int delayTrigger2Microseconds; // -1 for no second trigger
		int constantPulseRepetitionRateHz;
	} TriggerSetupNative;

	// The scanmode structure is used to define a beamformer operation state
	// It includes parameters for ultrasound excitation, echo reception and further processing.
	// This struct can be created and parameterized by the user to define the operation mode.
	typedef struct
	{
		// general info:
		char* scanModeName;

		// used transducer paramters:
		char* transducerName;
		float transducerPitchMeter;
		int transducerType;
		int transducerFrequencyHz;
		int transducerElementCount;
		int transducerCurvedRadiusMeter;

		// transmit parameters:
		float transmitPhaseLengthSeconds;
		int voltageV;
		int transmitEventsCount;

		// predefined transmit pattern:
		float* transmitEventsDelays;
		SequenceNative* transmitSequences;
		int transmitSequenceCount;
		int* transmitEventsMuxPosition; // [0;7]
		int* BurstCountPerChannel;
		int* BurstHalfwaveClockCountPerChannel;
		bool* BurstUseNegativePolarityPerChannel;
		
		int BurstHalfwaveClockCountAllChannels;

		// custom transmit pattern:
		char** transmitBitPattern;


		// trigger setup:
		TriggerSetupNative triggerSetup;

		// receive parameters:
		float receivePhaseLengthSeconds;
		unsigned char* tgcdB;
		int accumulation;
		int averagingCount;

		int averageSpeedOfSound;

		// specify which type of data to be transferred over the API:
		bool transferChannelData;
		bool transferBeamformedData;
		bool transferImageData;

		// reconstruction parameters for rf data:
		bool computeBeamforming;
		int beamformingAlgorithm; // from the enum Beamforming
		void* beamformingAlgorithmParameters; // for example BeamformingParametersPlaneWaveCompound
		int reconstructionLines;
		int reconstructionSamplesPerLine;
		float reconstructedLinePitchMmOrAngleDegree;

		bool bandpassApply;
		bool bandpassFrequencyLowHz;
		bool bandpassFrequencyHighHz;


		// reconstruction parameters for image data:
		int imageWidth;
		int imageHeight;
		int imageBytesPerPixel; // 1: greyscale only, 4: RGBA 32bit color image data with alpha
		float imageMultiplier;
		int imageLeveling;
	} ScanModeNative;

	// This struct contains information about internal datarates or beamformer parameters
	// It can be requested by the API and the data should only be read by the user application.
	typedef struct
	{
		float dataTransferRateMBit;
		float reconstructedDatasetsPerSecond;
		float systemPRF;
		char** customInformation;
		int customInformationCount;
	} BeamformerStateInfoNative;

	// This struct contains data about the configuration and capabilities of the current 
	// beamformer installed. It can be requested by the API and the data should only be read
	// by the user application.
	typedef struct
	{
		int ClockRateTransmitterInHz;
		int ExtraReceiveTimeMicroSeconds;
		const char* HardwareInterface;
		int MaxNumberOfTransmitEvents;
		int MaxSampleRateInHz;
		float MaxTxLengthMicroSeconds;
		float MaxVoltageInV;
		int MinTransmitterPauseInClocksBySwitchingPosNeg;
		bool MultiplexerAvailable;
		int NumberOfChannels;
		unsigned int NumberOfChannelsPerFrontend;
		unsigned int NumberOfFrontends;
		int* PossibleAccumulations;
		int PossibleAccumulationsCount;
		int SamplesTransferIntervalSize;
	} BeamformerCapabilitiesNative;

	// This function created the beamformer class and sets up the Platform Setup.
	// The callback functions specified are used for further information and data transmission
	// from the beamformer implementation to the user code.
	// It is possible to get information strings, pre-beamformed channel data of transducer elements,
	// beamformed data including postprocessing and the final image data after scan conversion of
	// the beamformed data.
	CWRAPPERAPI bool createBeamformer(
		StringMessageCallback stringMessageCallback,
		NewDataCallback newDataCallback);

	// This function established the communication to the beamformer and starts it initially
	CWRAPPERAPI bool initBeamformer();

	// This function configures the current scan mode according to the content of the
	// passed "ScanModeNative" instance. This function blocks until the new scan mode is set.
	CWRAPPERAPI bool setupScan(ScanModeNative scanMode);

	// the function requests the beamformer capabilities of the connected beamformer system.
	// It returns an instance of the "BeamformerCapabilitiesNative" struct.
	CWRAPPERAPI void getBeamformerCapabilities(BeamformerCapabilitiesNative* caps);

	// This function closes the hardware interface connection and frees all resources used
	// during measurements.
	CWRAPPERAPI void closeBeamformer();

	// This function starts or stops the beamformer permanent scanning operation according
	// to its previous state. Data will be send permanently to the user code using the delegate
	// functions for each measurement until this function is called again to stop the operation again.
	// For single measurement the function "makeSingleScan" can be used.
	CWRAPPERAPI bool toggleFreeze();

	// the function requests the current internal information for the set up operation
	// mode and returns it in an instance of the "BeamformerStateInfoNative" struct.
	CWRAPPERAPI void getSystemInfo(BeamformerStateInfoNative* infos);

	// a function to trigger a single sequence defined in the ScanMode that has to be
	// set previously using "bool setupScan(ScanModeNative scanMode)".
	// Data will be send over the delegate functions specified in "initBeamformer" once.
	CWRAPPERAPI void makeSingleScan();

	// Send costum commands directly to beamformer.
	CWRAPPERAPI void CustomCommand(std::string command);

//}