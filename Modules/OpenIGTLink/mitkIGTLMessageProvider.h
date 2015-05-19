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

//itk
#include "itkCommand.h"

namespace mitk {
  /**
  * \brief Provides information/objects from a MITK-Pipeline to other OpenIGTLink
  * devices
  *
  * This class is intended as the drain of the pipeline. Other OpenIGTLink
  * devices connect with the IGTLDevice hold by this provider. The other device
  * asks for a certain data type. The provider checks if there are other
  * IGTLMessageSources available that provide this data type. If yes the provider
  * connects with this source and sends the message to the requesting device.
  *
  * If a STT message was received the provider looks for fitting messageSources.
  * Once found it connects with it, starts a timing thread (which updates the
  * pipeline) and sends the result to the requesting device.
  *
  * If a GET message was received the provider just calls an update of the
  * found source and sends the result without connecting to the source.
  *
  * If a STP message was received it stops the thread and disconnects from the
  * previous source.
  *
  * So far the provider can just connect with one source.
  *
  * \ingroup OpenIGTLink
  */
  class MITKOPENIGTLINK_EXPORT IGTLMessageProvider : public IGTLDeviceSource
  {
  public:
    mitkClassMacro(IGTLMessageProvider, IGTLDeviceSource);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    typedef itk::SimpleMemberCommand<mitk::IGTLMessageProvider> ProviderCommand;

    /**
    * \brief sends the msg to the requesting client
    *
    * Note: so far it broadcasts the message to all registered clients
    */
    void Send(const IGTLMessage* msg);

    /**
    * \brief Starts the streaming of the given message source with the given fps.
    */
    void StartStreamingOfSource(mitk::IGTLMessageSource* src,
                                unsigned int fps);

    /**
    * \brief Stops the streaming of the given message source.
    */
    void StopStreamingOfSource(mitk::IGTLMessageSource* src);

    /**
    * \brief Returns the streaming state.
    */
    bool IsStreaming();

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
    virtual void GenerateData() override;

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
    virtual void OnIncomingMessage() override;

    /**
    * \brief This method is called when the IGTL device hold by this class
    * receives a new command
    **/
    virtual void OnIncomingCommand() override;

    /**
    *\brief Connects the input of this filter to the outputs of the given
    * IGTLMessageSource
    *
    * This method does not support smartpointer. use FilterX.GetPointer() to
    * retrieve a dumbpointer.
    */
    void ConnectTo( mitk::IGTLMessageSource* UpstreamFilter );

    /**
    *\brief Disconnects this filter from the outputs of the given
    * IGTLMessageSource
    *
    * This method does not support smartpointer. use FilterX.GetPointer() to
    * retrieve a dumbpointer.
    */
    void DisconnectFrom( mitk::IGTLMessageSource* UpstreamFilter );

    /**
    * \brief Looks for microservices that provide messages with the requested
    * type.
    **/
    mitk::IGTLMessageSource::Pointer GetFittingSource(const char* requestedType);

  private:
    /**
     * \brief a command that has to be executed in the main thread
     */
    ProviderCommand::Pointer m_StreamingCommand;

    /**
     * \brief Timer thread for generating a continuous time signal for the stream
     *
     * Everyt time the time is passed a time signal is invoked.
     *
     * \param pInfoStruct pointer to the mitkIGTLMessageProvider object
     * \return
     */
    static ITK_THREAD_RETURN_TYPE TimerThread(void* pInfoStruct);

    int                                       m_ThreadId;

    /** \brief timer thread will terminate after the next wakeup if set to true */
    bool                                      m_StopStreamingThread;

    itk::SmartPointer<itk::MultiThreader>     m_MultiThreader;

    /** \brief the time used for streaming */
    unsigned int                              m_StreamingTime;

    /** \brief mutex for guarding m_Time */
    itk::SmartPointer<itk::FastMutexLock>     m_StreamingTimeMutex;

    /** \brief mutex for guarding m_StopStreamingThread */
    itk::SmartPointer<itk::FastMutexLock>     m_StopStreamingThreadMutex;

    /** \brief flag to indicate if the provider is streaming */
    bool                                      m_IsStreaming;

  };
} // namespace mitk
#endif /* MITKIGTLMESSAGEPROVIDER_H_HEADER_INCLUDED_ */
