/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#ifndef MITKTRACKINGDEVICESOURCE_H_HEADER_INCLUDED_
#define MITKTRACKINGDEVICESOURCE_H_HEADER_INCLUDED_

#include <mitkNavigationDataSource.h>
#include "mitkTrackingDevice.h"

namespace mitk {
  /**Documentation
  * \brief Connects a mitk::TrackingDevice to a MITK-IGT NavigationData-Filterpipeline
  *
  * This class is the source of most navigation pipelines. It encapsulates a mitk::TrackingDevice
  * and provides the position and orientation of the connected mitk::TrackingTool objects 
  * as NavigationData objects. Note, that the number of outputs of TrackingDeviceSource
  * is equal to the number of tools connected to the TrackingDevice at the time 
  * SetTrackingDevice() is called. If tools are added to the TrackingDevice later,
  * there will not be additional outputs in TrackingDeviceSource. You have to call 
  * SetTrackingDevice() again to add the new tools as additional outputs of the filter.
  * Otherwise TrackingDeviceSource will raise an std::out_of_range exception when the filter pipeline is executed.
  * the tool number corresponds with the output number, e.g. trackingDevice-GetTool(0) is 
  * the tool that will produce trackingDeviceSourceFilter->GetOutput(0).
  * \warning If a tool is removed from the tracking device, there will be a mismatch between
  * the outputs and the tool number!
  *
  * \ingroup IGT
  */
  class MitkIGT_EXPORT TrackingDeviceSource : public NavigationDataSource
  {
  public:
    mitkClassMacro(TrackingDeviceSource, NavigationDataSource);
    itkNewMacro(Self);

    /**
    * \brief sets the tracking device that will be used as a source for tracking data
    */
    virtual void SetTrackingDevice(mitk::TrackingDevice* td);
    
    /**
    * \brief returns the tracking device that is used by this filter
    */
    itkGetConstObjectMacro(TrackingDevice, mitk::TrackingDevice);

    /**
    * \brief Establishes a connection to the tracking device
    * \warning. Will throw a std::invalid_argument exception if no tracking device was 
    * set with SetTrackingDevice(). Will throw a std::runtime_error if the tracking device 
    * returns an error.
    */
    void Connect();

    /**
    * \brief Closes the connection to the tracking device
    * \warning. Will throw a std::invalid_argument exception if no tracking device was 
    * set with SetTrackingDevice(). Will throw a std::runtime_error if the tracking device 
    * returns an error.
    */
    void Disconnect();

    /**
    * \brief starts tracking. 
    * This needs to be called before Update() or GetOutput()->Update().
    * \warning. Will throw a std::invalid_argument exception if no tracking device was 
    * set with SetTrackingDevice(). Will throw a std::runtime_error if the tracking device 
    * returns an error.
    */
    void StartTracking();

    /**
    * \brief stops tracking. 
    * \warning. Will throw a std::invalid_argument exception if no tracking device was 
    * set with SetTrackingDevice(). Will throw a std::runtime_error if the tracking device 
    * returns an error.
    */
    void StopTracking();

    /**
    * \brief returns true if a connection to the tracking device is established
    *
    */
    virtual bool IsConnected();

    /**
    * \brief returns true if tracking is in progress
    *
    */
    virtual bool IsTracking();

    /**
    * \brief Used for pipeline update
    */
    virtual void UpdateOutputInformation();

  protected:
    TrackingDeviceSource();
    virtual ~TrackingDeviceSource();

    /**
    * \brief filter execute method
    *
    * queries the tracking device for new position and orientation data for all tools 
    * and updates its output NavigationData objects with it.
    * \warning Will raise a std::out_of_range exception, if tools were added to the 
    * tracking device after it was set as input for this filter     
    */
    virtual void GenerateData();
    
    /**
    * \brief Create the necessary outputs for the TrackingTool objects in m_TrackingDevice
    **/
    void CreateOutputs();

    mitk::TrackingDevice::Pointer m_TrackingDevice;  ///< the tracking device that is used as a source for this filter object
  };
} // namespace mitk
#endif /* MITKTrackingDeviceSource_H_HEADER_INCLUDED_ */
