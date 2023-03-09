/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIGTLMessageProvider_h
#define mitkIGTLMessageProvider_h

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
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    typedef itk::SimpleMemberCommand<mitk::IGTLMessageProvider> ProviderCommand;

    /**
    * \brief sends the msg to the requesting client
    *
    * Note: so far it broadcasts the message to all registered clients
    */
    void Send(mitk::IGTLMessage::Pointer msg);

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
    * \brief Stops the streaming of all message source.
    */
    void StopStreamingOfAllSources();

    /**
    * \brief Returns the streaming state.
    */
    bool IsStreaming();

    /**
    * \brief Get method for the streaming time
    */
    itkGetMacro(StreamingTime, unsigned int);

    void Update() override;

  protected:
    IGTLMessageProvider();
    ~IGTLMessageProvider() override;

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
    void OnIncomingMessage() override;

    /**
    * \brief This method is called when the IGTL device hold by this class
    * receives a new command
    **/
    void OnIncomingCommand() override;

    /**
    * \brief This method is called when the IGTL device lost the connection to the other side
    **/
    void OnLostConnection() override;

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

    /** Invokes the start streaming event. This separate method is required, because it
     *  has to be started from the main thread. (It is used as callback function)
     */
    void InvokeStartStreamingEvent();

     /** Invokes the stop streaming event. This separate method is required, because it
     *  has to be started from the main thread. (It is used as callback function)
     */
    void InvokeStopStreamingEvent();

  private:
    /**
     * \brief a command that has to be executed in the main thread
     */
    ProviderCommand::Pointer m_StreamingCommand;

    ProviderCommand::Pointer m_StopStreamingCommand;

    ///**
    // * \brief Timer thread for generating a continuous time signal for the stream
    // *
    // * Everyt time the time is passed a time signal is invoked.
    // *
    // * \param pInfoStruct pointer to the mitkIGTLMessageProvider object
    // * \return
    // */
    //static ITK_THREAD_RETURN_TYPE TimerThread(void* pInfoStruct);

    //int                                       m_ThreadId;

    ///** \brief timer thread will terminate after the next wakeup if set to true */
    //bool                                      m_StopStreamingThread;

    //itk::SmartPointer<itk::MultiThreader>     m_MultiThreader;

    /** \brief the time used for streaming */
    unsigned int                              m_StreamingTime;

    /** \brief mutex for guarding m_Time */
    std::mutex                                m_StreamingTimeMutex;

    ///** \brief mutex for guarding m_StopStreamingThread */
    //itk::SmartPointer<itk::FastMutexLock>     m_StopStreamingThreadMutex;

    /** \brief flag to indicate if the provider is streaming */
    bool                                      m_IsStreaming;

  };

  /**
  * \brief connect to this Event to get notified when a stream is requested
  *
  * \note It is necessary to do the following things to have streaming support: 1. listen to this
  * event. 2. When emitted start a timer with the given interval. 3. In the timeout method of
  * this timer call IGTLMessageProvider::Update. 4. Also listen to the StreamingStopRequiredEvent
  * and stop the timer imdediately.
  * */
  itkEventMacroDeclaration(StreamingStartRequiredEvent, itk::AnyEvent);

  /**
  * \brief connect to this Event to get notified when a stream shall be stopped
  *
  * \note It is necessary to connect to this event and stop the streaming timer when called.
  * */
  itkEventMacroDeclaration(StreamingStopRequiredEvent, itk::AnyEvent);

} // namespace mitk
#endif
