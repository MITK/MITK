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

#ifndef IGTLMESSAGEPROVIDER_H_HEADER_INCLUDED_
#define IGTLMESSAGEPROVIDER_H_HEADER_INCLUDED_

#include "mitkIGTLDevice.h"
#include "mitkIGTLDeviceSource.h"

namespace mitk {
  /**Documentation
  * \brief Provides information/objects from a MITK-Pipeline to other OpenIGTLink
  * devices
  *
  * This class is intended as the drain of the pipeline. Other OpenIGTLink
  * devices connect with the device hold by this provider. The other device asks
  * for a special data. The provider checks if there are other IGTLMessageSources
  * available that provide this data type. If yes, they connect with this source
  * and send the message to the requesting device.
  *
  *
  */
  class MITK_OPENIGTLINK_EXPORT IGTLMessageProvider : public IGTLDeviceSource
  {
  public:
    mitkClassMacro(IGTLMessageProvider, IGTLDeviceSource);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

  protected:
    IGTLMessageProvider();
    virtual ~IGTLMessageProvider();

    /**
    * \brief filter execute method
    *
    * queries the OpenIGTLink device for new messages and updates its output
    * igtl::MessageBase objects with it.
    * \warning Will raise a std::out_of_range exception, if tools were added to
    * the OpenIGTLink device after it was set as input for this filter
    */
    virtual void GenerateData();

    /**
    * \brief Create the necessary outputs for the m_IGTLDevice
    *
    * This Method is called internally whenever outputs need to be reset. Old
    * Outputs are deleted when called.
    **/
    void CreateOutputs();

    /**
    * \brief This method is called when the IGTL device hold by this class
    * receives a new message
    **/
    virtual void OnIncomingMessage();

    /**
    * \brief This method is called when the IGTL device hold by this class
    * receives a new command
    **/
    virtual void OnIncomingCommand();

    /**
    * \brief Looks for microservices that provide messages with the requested
    * type.
    **/
    mitk::IGTLMessageSource::Pointer GetFittingSource(const char* requestedType);
  };
} // namespace mitk
#endif /* MITKIGTLMESSAGEPROVIDER_H_HEADER_INCLUDED_ */
