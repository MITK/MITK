/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIGTLDeviceSource_h
#define mitkIGTLDeviceSource_h

#include "mitkIGTLDevice.h"
#include "mitkIGTLMessageSource.h"

namespace mitk {
  /**
  * \brief Connects a mitk::IGTLDevice to a
  *        MITK-OpenIGTLink-Message-Filter-Pipeline
  *
  * This class is the source of most OpenIGTLink pipelines. It encapsulates a
  * mitk::IGTLDevice and provides the information/messages of the connected
  * OpenIGTLink devices as igtl::MessageBase objects. Note, that there is just
  * one single output.
  *
  */
  class MITKOPENIGTLINK_EXPORT IGTLDeviceSource : public IGTLMessageSource
  {
  public:
    mitkClassMacro(IGTLDeviceSource, IGTLMessageSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
    *\brief These Constants are used in conjunction with Microservices
    */
    static const std::string US_PROPKEY_IGTLDEVICENAME;

    /**
    * \brief sets the OpenIGTLink device that will be used as a data source
    */
    virtual void SetIGTLDevice(mitk::IGTLDevice* td);

    /**
    * \brief returns the OpenIGTLink device that is used by this filter
    */
    itkGetObjectMacro(IGTLDevice, mitk::IGTLDevice);

    /**
    *\brief Registers this object as a Microservice, making it available to every
    * module and/or plugin. To unregister, call UnregisterMicroservice().
    */
    void RegisterAsMicroservice() override;

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
    void UpdateOutputInformation() override;

  protected:
    IGTLDeviceSource();
    ~IGTLDeviceSource() override;

    /**
    * \brief filter execute method
    *
    * queries the OpenIGTLink device for new messages and updates its output
    * igtl::MessageBase objects with it.
    * \warning Will raise a std::out_of_range exception, if tools were added to
    * the OpenIGTLink device after it was set as input for this filter
    */
    void GenerateData() override;

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
    * \brief This method is called when the IGTL device lost the connection to the other side
    **/
    virtual void OnLostConnection();

    /**
    * \brief Removes all observers that listen to the igtl device
    **/
    virtual void RemoveObservers();


    using Superclass::SetInput;

    /**
    * \brief Set input with id idx of this filter
    *
    */
    virtual void SetInput( unsigned int idx, const IGTLMessage* msg );

    /**
    * \brief Get the input of this filter
    */
    const IGTLMessage* GetInput(void) const;

    /**
    * \brief Get the input with id idx of this filter
    */
    const IGTLMessage* GetInput(unsigned int idx) const;

    /**
    * \brief Get the input with name messageName of this filter
    */
    const IGTLMessage* GetInput(std::string msgName) const;

    /**
    *\brief return the index of the input with name msgName, throw
    * std::invalid_argument exception if that name was not found
    *
    * \warning if a subclass has inputs that have different data type than
    * mitk::IGTLMessage, they have to overwrite this method
    */
    DataObjectPointerArraySizeType GetInputIndex(std::string msgName);

    /**
    *\brief return the index of the output with name msgName, -1 if no output
    * with that name was found
    *
    * \warning if a subclass has outputs that have different data type than
    * mitk::IGTLMessage, they have to overwrite this method
    */
    DataObjectPointerArraySizeType GetOutputIndex(std::string msgName);

    /** the OpenIGTLink device that is used as a source for this filter object*/
    mitk::IGTLDevice::Pointer m_IGTLDevice;

    /** observer tags */
    unsigned int m_LostConnectionObserverTag;
    unsigned int m_IncomingCommandObserverTag;
    unsigned int m_IncomingMessageObserverTag;
  };
} // namespace mitk
#endif
