/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2008-06-26 19:19:58 +0200 (Do, 26 Jun 2008) $
Version:   $Revision: 14656 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKMIRCOBIRDTRACKINGDEVICE_H_HEADER_INCLUDED
#define MITKMIRCOBIRDTRACKINGDEVICE_H_HEADER_INCLUDED

#include "mitkTrackingDevice.h"

#include <mitkCommon.h>
#include <itkMultiThreader.h>

#include <list>

#include "mitkTrackingTypes.h"
#include "mitkMicroBirdTool.h"

#include "PCIBird3.h"

namespace mitk {
  namespace tracking {
    
    //## Documentation
    //## \brief superclass for specific MIRCOBIRD tracking Devices
    //## 
    //## ...
    //## 
    //## @ingroup Tracking
    class MicroBirdTrackingDevice : public TrackingDevice
    {
    public:

	  /*
	   * \brief Construction
	   */
      mitkClassMacro(MicroBirdTrackingDevice, TrackingDevice);
      itkNewMacro(Self);

	  /**
      * \brief Set the type of the microBird Tracking Device because it can not jet handle this itself
      **/
      itkSetMacro(Type,TrackingDeviceType);

      /**
      * \brief Builds up the connection (loads tools, initializes and enables them)
      **/
      virtual bool OpenConnection();

      /**
      * \brief Closes the connection
      **/
      virtual bool CloseConnection();

      /**
      * \brief Start the tracking. 
      * 
      * A new thread is created, which reads the position and orientation information of each tool and stores them inside the tools.
      **/
      virtual bool StartTracking();
      
      /**
      * \brief here we use the superclass method.
      **/
      virtual bool StopTracking();
      
      /**
      * \brief returns a tracking tool that contains positional information about one of the sensors
      **/
      virtual TrackingTool* GetTool(unsigned int toolNumber);

      /**
      * \brief returns a tracking tool that contains positional information about one of the sensors
      **/
      MicroBirdTool* GetMicroBirdTool(unsigned int toolNumber);

	  /**
      * \brief returns a the number of attached sensors
      **/
	  virtual unsigned int GetToolCount() const;

      /**
      * \brief returns description of most recent error.
      **/
      itkGetStringMacro(ErrorMessage);

    protected:

      void errorHandler(int errorCode);
      bool errorCompare(int errorCode, int errorConstant);

	    typedef std::vector<MicroBirdTool::Pointer> ToolContainerType;  // List of 6D tools of the correct type for this tracking device

      MicroBirdTrackingDevice();
      virtual ~MicroBirdTrackingDevice();

      /// invalidates all tools (on stoptracking, closeconnection)
      virtual void InvalidateAll();

      /// Switches the transmitter on resp. off
      bool SwitchTransmitter(bool switchOn);

      /// tracks the 6D position of all tools until StopTracking() is called.
      // This function should only be executed by a new thread (through StartTracking() and ThreadStartTracking())
      virtual void TrackTools();    

      /// Helper function, because the itk::MultiThreader can only start a new thread with a static member function
      static ITK_THREAD_RETURN_TYPE ThreadStartTracking(void* data);

      itkSetStringMacro(ErrorMessage);

      itk::FastMutexLock::Pointer m_ToolsMutex;
      ToolContainerType m_Tools;  

      std::string m_ErrorMessage;

      itk::MultiThreader::Pointer m_MultiThreader;
      int m_ThreadID;

		// One tracking data record (matrix orientation format)
		//DOUBLE_POSITION_MATRIX_TIME_Q_RECORD record, *pRecord;

		// One tracking data record (quaternion orientation format)
		DOUBLE_POSITION_QUATERNION_TIME_Q_RECORD record, *pRecord;

		/*
		 * The system configuration - used to specify its use
		 */
		SYSTEM_CONFIGURATION		m_SystemConfig;

		/*
		 * The sensor configuration - used to get and set the sensor properties
		 */
		SENSOR_CONFIGURATION		*m_SensorConfig;

		/*
		 * The transmitter configuration - used to get and set the transmitter properties\
		 */
		TRANSMITTER_CONFIGURATION	*m_TransmitterConfig;

		/*
		 * Specifies whether metric measurement is used
		 */
		BOOL m_metric;

		/*
		 * Specifies the measurement rate - default set to maximum
		 */
		double m_measurementRate;

		/*
		 * Specifies the power line frequency (Europe 50Hz, USA 60Hz)
		 */
		double m_pl;

		// AGC (automatic gain control) mode flag
		bool m_agcModeBoth;
		AGC_MODE_TYPE m_agc;

	};
} // namespace tracking
} // namespace mitk

#endif /* MITKMIRCOBIRDTRACKINGDEVICE_H_HEADER_INCLUDED*/
