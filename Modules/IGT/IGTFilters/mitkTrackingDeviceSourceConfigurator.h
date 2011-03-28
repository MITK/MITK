/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2011-01-18 13:22:38 +0100 (Di, 18 Jan 2011) $
Version:   $Revision: 28959 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKTRACKINGDEVICESOURCECONFIGURATOR_H_HEADER_INCLUDED_
#define MITKTRACKINGDEVICESOURCECONFIGURATOR_H_HEADER_INCLUDED_

#include <MitkIGTExports.h>

//itk includes
#include <itkObject.h>

//mitk IGT includes
#include "mitkTrackingDeviceSource.h"
#include "mitkNavigationToolStorage.h"
#include "mitkNavigationDataObjectVisualizationFilter.h"

namespace mitk {
  /**Documentation
  * \brief This class offers a factory method for objects of the class TrackingDeviceSource. It initializes this TrackingDeviceSource with
  *        the given navigation tools and the given tracking device. The factory method also checks if all tools are valid and of the same
  *        type like the TrackingDevice. You can do this check before trying to create the TrackingDeviceSource by calling the method 
  *        IsCreateTrackingDeviceSourcePossible(), if it returns false you might want to get the error message by calling the method
  *        GetErrorMessage().
  * \ingroup IGT
  */
  class MitkIGT_EXPORT TrackingDeviceSourceConfigurator : public itk::Object
  {
  public:
    mitkClassMacro(TrackingDeviceSourceConfigurator,  itk::Object);
    mitkNewMacro2Param(Self,mitk::NavigationToolStorage::Pointer,mitk::TrackingDevice::Pointer);

    /** @return Returns if it's possible to create a tracking device source, which means the tools are checked
     *          if they are of the same type like the tracking device, etc. If it returns false you can get
     *          the reason for this by getting the error message.
     */
    bool IsCreateTrackingDeviceSourcePossible();

    /** @return Returns a new TrackingDeviceSource. Returns NULL if there was an error on creating the 
     *          TrackingDeviceSource. If there was an error it's availiable as error message.
     */
    mitk::TrackingDeviceSource::Pointer CreateTrackingDeviceSource();

    /** @return Returns a new TrackingDeviceSource. Returns NULL if there was an error on creating the 
     *          TrackingDeviceSource. If there was an error it's availiable as error message.
     *  @param visualizationFilter (return value) returns a visualization filter which is already connected to the tracking device source.
     *                             This filter visualises the surfaces which are availiable by the navigation tool storage.
     */
    mitk::TrackingDeviceSource::Pointer CreateTrackingDeviceSource(mitk::NavigationDataObjectVisualizationFilter::Pointer &visualizationFilter);
    
    /** @return Returns the current error message. Returns an empty string if there was no error.
     */
    std::string GetErrorMessage();

  protected:
    TrackingDeviceSourceConfigurator(mitk::NavigationToolStorage::Pointer NavigationTools, mitk::TrackingDevice::Pointer TrackingDevice);
    virtual ~TrackingDeviceSourceConfigurator();

    mitk::NavigationToolStorage::Pointer m_NavigationTools;
    mitk::TrackingDevice::Pointer m_TrackingDevice;
    std::string m_ErrorMessage;

    mitk::TrackingDeviceSource::Pointer CreateNDIPolarisTrackingDeviceSource(mitk::TrackingDevice::Pointer trackingDevice, mitk::NavigationToolStorage::Pointer navigationTools);
    mitk::TrackingDeviceSource::Pointer CreateNDIAuroraTrackingDeviceSource(mitk::TrackingDevice::Pointer trackingDevice, mitk::NavigationToolStorage::Pointer navigationTools);
    mitk::TrackingDeviceSource::Pointer CreateMicronTrackerTrackingDeviceSource(mitk::TrackingDevice::Pointer trackingDevice, mitk::NavigationToolStorage::Pointer navigationTools);
    mitk::NavigationDataObjectVisualizationFilter::Pointer CreateNavigationDataObjectVisualizationFilter(mitk::TrackingDeviceSource::Pointer trackingDeviceSource, mitk::NavigationToolStorage::Pointer navigationTools);

  };
} // namespace mitk
#endif /* MITKTrackingDeviceSource_H_HEADER_INCLUDED_ */