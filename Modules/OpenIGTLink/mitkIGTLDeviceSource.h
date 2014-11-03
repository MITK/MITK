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

#ifndef IGTLDEVICESOURCE_H_HEADER_INCLUDED_
#define IGTLDEVICESOURCE_H_HEADER_INCLUDED_

#include "mitkIGTLDevice.h"
#include "mitkIGTLMessageSource.h"

namespace mitk {
  /**Documentation
  * \brief Connects a mitk::IGTLDevice to a
  *        MITK-OpenIGTLink Message-Filterpipeline
  *
  * This class is the source of most OpenIGTLink pipelines. It encapsulates a
  * mitk::IGTLDevice and provides the information/messages of the connected
  * OpenIGTLink devices as igtl::MessageBase objects. Note, that there is just
  * one single output.
  *
  */
  class MITK_OPENIGTLINK_EXPORT IGTLDeviceSource : public IGTLMessageSource
  {
  public:
    mitkClassMacro(IGTLDeviceSource, IGTLMessageSource);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /**
    * \brief sets the OpenIGTLink device that will be used as a data source
    */
    virtual void SetIGTLDevice(mitk::IGTLDevice* td);

    /**
    * \brief returns the OpenIGTLink device that is used by this filter
    */
    itkGetConstObjectMacro(IGTLDevice, mitk::IGTLDevice);

    /**
    * \brief Establishes a connection to the OpenIGTLink device. If there is
    * already a connection the method does nothing.
    * \warning. Will throw a std::invalid_argument exception if no OpenIGTLink
    * device was set with SetIGTLDevice(). Will throw a std::runtime_error if
    * the OpenIGTLink device returns an error.
    */
    void Connect();

    /**
    * \brief Closes the connection to the OpenIGTLink device
    * \warning. Will throw a std::invalid_argument exception if no OpenIGTLink
    * device was set with SetIGTLDevice(). Will throw a std::runtime_error if
    * the OpenIGTLink device returns an error.
    */
    void Disconnect();

    /**
    * \brief starts the communication of the device.
    * This needs to be called before Update() or GetOutput()->Update(). If the
    * device is already communicating the method does nothing.
    * \warning. Will throw a std::invalid_argument exception if no OpenIGTLink
    * device was set with SetIGTLDevice(). Will throw a std::runtime_error if
    * the OpenIGTLink device returns an error.
    */
    void StartCommunication();

    /**
    * \brief stops the communication of the device.
    * \warning. Will throw a std::invalid_argument exception if no OpenIGTLink
    * device was set with SetIGTLDevice(). Will throw a std::runtime_error if
    * the OpenIGTLink device returns an error.
    */
    void StopCommunication();

    /**
    * \brief returns true if a connection to the OpenIGTLink device is established
    *
    */
    virtual bool IsConnected();

    /**
    * \brief returns true if communication is in progress
    *
    */
    virtual bool IsCommunicating();

    /**
    * \brief Used for pipeline update
    */
    virtual void UpdateOutputInformation();

  protected:
    IGTLDeviceSource();
    virtual ~IGTLDeviceSource();

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

    /** the OpenIGTLink device that is used as a source for this filter object*/
    mitk::IGTLDevice::Pointer m_IGTLDevice;
  };
} // namespace mitk
#endif /* MITKIGTLDeviceSource_H_HEADER_INCLUDED_ */
