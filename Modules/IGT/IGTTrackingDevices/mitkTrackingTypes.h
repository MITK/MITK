/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKTRACKINGTYPES_H_HEADER_INCLUDED_
#define MITKTRACKINGTYPES_H_HEADER_INCLUDED_

#include <itkPoint.h>
#include <vector>
//#include <mitkVector.h>

namespace mitk 
{

  /**Documentation
   * \brief Error codes of NDI tracking devices
   */
    enum NDIErrorCode
    {
      NDIOKAY = 0,
      NDIERROR = 1,
      SERIALINTERFACENOTSET,
      SERIALSENDERROR,
      SERIALRECEIVEERROR,
      SROMFILETOOLARGE,
      SROMFILETOOSMALL,
      NDICRCERROR,                          // reply has crc error, local computer detected the error
      NDIINVALIDCOMMAND,
      NDICOMMANDTOOLONG,
      NDICOMMANDTOOSHORT,
      NDICRCDOESNOTMATCH,                   // command had crc error, tracking device detected the error
      NDITIMEOUT,
      NDIUNABLETOSETNEWCOMMPARAMETERS,
      NDIINCORRECTNUMBEROFPARAMETERS,
      NDIINVALIDPORTHANDLE,
      NDIINVALIDTRACKINGPRIORITY,
      NDIINVALIDLED,
      NDIINVALIDLEDSTATE,
      NDICOMMANDINVALIDINCURRENTMODE,
      NDINOTOOLFORPORT,
      NDIPORTNOTINITIALIZED,
      NDISYSTEMNOTINITIALIZED,
      NDIUNABLETOSTOPTRACKING,
      NDIUNABLETOSTARTTRACKING,
      NDIINITIALIZATIONFAILED,
      NDIINVALIDVOLUMEPARAMETERS,
      NDICANTSTARTDIAGNOSTICMODE,
      NDICANTINITIRDIAGNOSTICS,
      NDIFAILURETOWRITESROM,
      NDIENABLEDTOOLSNOTSUPPORTED,
      NDICOMMANDPARAMETEROUTOFRANGE,
      NDINOMEMORYAVAILABLE,
      NDIPORTHANDLENOTALLOCATED,
      NDIPORTHASBECOMEUNOCCUPIED,
      NDIOUTOFHANDLES,
      NDIINCOMPATIBLEFIRMWAREVERSIONS,
      NDIINVALIDPORTDESCRIPTION,
      NDIINVALIDOPERATIONFORDEVICE,
      NDIWARNING,
      NDIUNKNOWNERROR,
      NDIUNEXPECTEDREPLY,
      UNKNOWNHANDLERETURNED,
      TRACKINGDEVICERESET,
      TRACKINGDEVICENOTSET
    };

    /**Documentation
    * \brief identifier for tracking device. The way it is currently used 
	* represents a product line rather than an actal type. Refactoring is a future option.
    */
    enum TrackingDeviceType
    {
      NDIPolaris,                ///< Polaris: optical Tracker from NDI
      NDIAurora,                 ///< Aurora: electromagnetic Tracker from NDI
      ClaronMicron,              ///< Micron Tracker: optical Tracker from Claron
      IntuitiveDaVinci,          ///< Intuitive Surgical: DaVinci Telemanipulator API Interface
      AscensionMicroBird,        ///< Ascension microBird / PCIBird family
      VirtualTracker,            ///< Virtual Tracking device class that produces random tracking coordinates
      TrackingSystemNotSpecified,///< entry for not specified or initialized tracking system
      TrackingSystemInvalid      ///< entry for invalid state (mainly for testing)
    };

	

    /**Documentation
    * \brief Error codes of NDI tracking devices
    */
    enum OperationMode
    {
      ToolTracking6D,
      ToolTracking5D,
      MarkerTracking3D,
      HybridTracking
    };

    /**
    * \brief This enum is deprecated. In future, please use the new TrackingDeviceData to model Specific tracking Volumes
	* Represents the setting of the tracking volume of a NDI tracking device. The tracking volume of
    * a tracking device itself (as 3d-Object) is represented by an instance of the class mitk::TrackingVolume
    * as defined by NDI API SFLIST (Aurora and Polaris API guide)
	* 
    */
    enum NDITrackingVolume
    {
	  Standard,
      Pyramid,
      SpectraPyramid,
      VicraVolume,
      Cube,
      Dome
    };

	/**
	* /brief This structure defines key variables of a device model and type.
	* It is specifically used to find out which models belong to which vendor, and what volume
	* to use for a specific Model. Leaving VolumeModelLocation set to null will instruct the Generator
	* to generate a field to the best of his ability
	*/
	struct TrackingDeviceData {
      TrackingDeviceType Line;
	  std::string Model;
	  std::string VolumeModelLocation;
    };	
		

	/**
	* Here all supported devices are defined. Dont forget to introduce new Devices into the TrackingDeviceList Array at the bottom!
	* If a model does not have a corresponding tracking volume yet, pass an empty string to denote "No Model". pass "cube" to render
	* a default cube of 400x400 px. You can define additional magic strings in the TrackingVolumeGenerator.
	*/
	static TrackingDeviceData DeviceDataAuroraCompact = {NDIAurora, "Aurora Compact", "NDIAuroraCompactFG_Dome.stl"};
	static TrackingDeviceData DeviceDataAuroraPlanarCube = {NDIAurora, "Aurora Planar - Cube Volume", "NDIAurora.stl"};
	static TrackingDeviceData DeviceDataAuroraPlanarDome = {NDIAurora, "Aurora Planar - Dome Volume","NDIAuroraPlanarFG_Dome.stl"};
	static TrackingDeviceData DeviceDataAuroraTabletop = {NDIAurora, "Aurora Tabletop", "NDIAuroraTabletopFG_Dome.stl"};
	// The following entry is for the tabletop prototype, which had an lower barrier of 8cm. The new version has a lower barrier of 12cm.
	//static TrackingDeviceData DeviceDataAuroraTabletopPrototype = {NDIAurora, "Aurora Tabletop Prototype", "NDIAuroraTabletopFG_Prototype_Dome.stl"};
	static TrackingDeviceData DeviceDataMicronTrackerH40 = {ClaronMicron, "Micron Tracker H40", "ClaronMicron.stl"};
	static TrackingDeviceData DeviceDataPolarisSpectra = {NDIPolaris, "Polaris Spectra", "NDIPolaris.stl"};
	static TrackingDeviceData DeviceDataPolarisVicra = {NDIPolaris, "Polaris Vicra", "NDIPolaris.stl"};
	static TrackingDeviceData DeviceDataDaVinci = {IntuitiveDaVinci, "Intuitive DaVinci", "IntuitiveDaVinci.stl"};
	static TrackingDeviceData DeviceDataMicroBird = {AscensionMicroBird, "Ascension MicroBird", ""};
	static TrackingDeviceData DeviceDataVirtualTracker = {VirtualTracker, "Virtual Tracker", "cube"};
	static TrackingDeviceData DeviceDataUnspecified = {TrackingSystemNotSpecified, "Unspecified System", "cube"};
	// Careful when changing the "invalid" device: The mitkTrackingTypeTest is using it's data.
	static TrackingDeviceData DeviceDataInvalid = {TrackingSystemInvalid, "Invalid Tracking System", ""};

	static TrackingDeviceData TrackingDeviceList[] = {DeviceDataAuroraPlanarCube, DeviceDataAuroraPlanarDome, DeviceDataAuroraCompact,
	DeviceDataAuroraTabletop, DeviceDataMicronTrackerH40, DeviceDataPolarisSpectra, DeviceDataPolarisVicra,
	DeviceDataDaVinci, DeviceDataMicroBird, DeviceDataVirtualTracker, DeviceDataUnspecified, DeviceDataInvalid};

	/**
	* /brief Returns all devices compatibel to the given Line of Devices 
	*/
	static std::vector<TrackingDeviceData> GetDeviceDataForLine(TrackingDeviceType Type){
		std::vector<TrackingDeviceData> Result;
		int size = (sizeof (TrackingDeviceList) / sizeof*(TrackingDeviceList));
		for(int i=0; i < size; i++)
		{			
			if(TrackingDeviceList[i].Line == Type ) Result.push_back(TrackingDeviceList[i]);
		} 
		return Result;
	}

	/**
	* /brief Returns the first TracingDeviceData mathing a given line. Useful for backward compatibility
	* with the old way to manage Devices
	*/
	static TrackingDeviceData GetFirstCompatibleDeviceDataForLine(TrackingDeviceType Type){
		
		return GetDeviceDataForLine(Type).front();
	}

	/**
	* /brief Returns the device Data set matching the model name or the invalid device, if none was found
	*/
	static TrackingDeviceData GetDeviceDataByName(std::string modelName){
	
		int size = (sizeof (TrackingDeviceList) / sizeof*(TrackingDeviceList));
		for(int i=0; i < size; i++)
		{			
			if(TrackingDeviceList[i].Model == modelName) return TrackingDeviceList[i];
		} 
		return DeviceDataInvalid;
	}

    /**Documentation
    * \brief activation rate of IR illuminator for NDI Polaris tracking device
    */
    enum IlluminationActivationRate
    {
      Hz20 = 20,
      Hz30 = 30,
      Hz60 = 60
    };

    /**Documentation
    * \brief Data transfer mode for NDI tracking devices
    */
    enum DataTransferMode
    {
      TX = 0,
      BX = 1
    };
    /**Documentation
    * \brief Query mode for NDI tracking devices
    */
    enum PHSRQueryType
    {
      ALL         = 0x00,
      FREED       = 0x01,
      OCCUPIED    = 0x02,
      INITIALIZED = 0x03,
      ENABLED     = 0x04
    };


    typedef itk::Point<double> MarkerPointType;
    typedef std::vector<MarkerPointType> MarkerPointContainerType;

    /**
    * \brief Defines the tools (arms) of the daVinci system:
    * PSM1 - Patient side manipulator 1
    * PSM2 - Patient side manipulator 2
    * ECM - Endoscopic camera manipulator
    * MTML - Left master target manipulator
    * MTMR - Right master target manipulator
    * PSM  - Patient side manipulator 3 (if not existent, its data will always be zero)
    **/
    enum DaVinciToolType
    {
      PSM1 = 0,
      PSM2 = 1,
      ECM  = 2,
      MTML = 3,
      MTMR = 4,
      PSM  = 5,
      //UIEvents = 6,
    };
} // namespace mitk
#endif /* MITKTRACKINGTYPES_H_HEADER_INCLUDED_ */
