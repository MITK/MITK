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

    /** @return Returns the internal number of the corresponding tool in the tool storage of a output navigation data. Returns -1 if there was an error. */
    int GetToolNumberInToolStorage(int outputID);

    /** @return Returns the identifier of the corresponding tool in the tool storage of a output navigation data. Returns an empty string if there was an error.*/
    std::string GetToolIdentifierInToolStorage(int outputID);

    /** @return Returns a vector with all internal numbers of the corresponding tools in the tool storage of all outputs.
      *         The order is the same like the order of the outputs. Returns an empty vector if there was an error.
      */
    std::vector<int> GetToolNumbersInToolStorage();

    /** @return Returns a vector with all identifier of the corresponding tools in the tool storage of all outputs.
      *         The order is the same like the order of the outputs. Returns an empty vector if there was an error.
      */
    std::vector<std::string> GetToolIdentifiersInToolStorage();

    /** @return Returns a modified navigation tool storage which holds the tools currently in use in
      *         the same order like the output ids of the pipline.
      */
    mitk::NavigationToolStorage::Pointer GetUpdatedNavigationToolStorage();

    /** @return Returns the current error message. Returns an empty string if there was no error.
     */
    std::string GetErrorMessage();

  protected:
    TrackingDeviceSourceConfigurator(mitk::NavigationToolStorage::Pointer NavigationTools, mitk::TrackingDevice::Pointer TrackingDevice);
    virtual ~TrackingDeviceSourceConfigurator();

    mitk::NavigationToolStorage::Pointer m_NavigationTools;
    mitk::TrackingDevice::Pointer m_TrackingDevice;
    std::string m_ErrorMessage;
    std::vector<int> m_ToolCorrespondencesInToolStorage;

    mitk::TrackingDeviceSource::Pointer CreateNDIPolarisTrackingDeviceSource(mitk::TrackingDevice::Pointer trackingDevice, mitk::NavigationToolStorage::Pointer navigationTools);
    mitk::TrackingDeviceSource::Pointer CreateNDIAuroraTrackingDeviceSource(mitk::TrackingDevice::Pointer trackingDevice, mitk::NavigationToolStorage::Pointer navigationTools);
    mitk::TrackingDeviceSource::Pointer CreateMicronTrackerTrackingDeviceSource(mitk::TrackingDevice::Pointer trackingDevice, mitk::NavigationToolStorage::Pointer navigationTools);
    mitk::NavigationDataObjectVisualizationFilter::Pointer CreateNavigationDataObjectVisualizationFilter(mitk::TrackingDeviceSource::Pointer trackingDeviceSource, mitk::NavigationToolStorage::Pointer navigationTools);

  };
} // namespace mitk
#endif /* MITKTrackingDeviceSource_H_HEADER_INCLUDED_ */
