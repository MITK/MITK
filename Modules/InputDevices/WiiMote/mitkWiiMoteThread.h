#ifndef MITK_WIIMOTEHTREAD_H
#define MITK_WIIMOTEHTREAD_H

#include "mitkCommon.h"
#include "itkObject.h"

#include "itkMultiThreader.h"
#include "itkFastMutexLock.h"
#include "mitkCallbackFromGUIThread.h"

#include "wiimote.h"

#include "itksys/SystemTools.hxx" // used for GetTime()
#include "mitkVector.h"

namespace mitk
{
  // instead of include, enables this class to know the addon
  class WiiMoteAddOn;

  class WiiMoteThread : public itk::Object
  {
  public:

    // typedefs are used in context with CallbackFromGUIThread
    typedef itk::ReceptorMemberCommand<mitk::WiiMoteAddOn> ReceptorCommand;

    // eventually probs with Linux -> typename
    typedef ReceptorCommand::Pointer ReceptorCommandPointer;

    WiiMoteThread();
    ~WiiMoteThread();

    itkSetMacro(SleepTime, int);
    itkGetConstMacro(SleepTime, int);

    /**
    * Allows to set report types, detects extensions and responds to connect/disconnect <br>
    * events of extension, such as MotionPlus. 
    *
    * NOTE: don't access the public state from the 'remote' object here, as it will
    *	      be out-of-date (it's only updated via RefreshState() calls, and these
    *		    are reserved for the main application so it can be sure the values
    *		    stay consistent between calls).  Instead query 'new_state' only.
    *
    * @param remote 
    *          the old state of the connected Wii remote
    * @param changed
    *          the state change of the Wii remote
    * @param newState
    *          the new state, after the change is applied (i.e. new extension connected)
    *           
    */
    static void OnStateChange(wiimote &remote, state_change_flags  changed, const wiimote_state &newState);

    /**
    * Starts the thread for the Wiimote.
    */
    void Run();

    /**
    * Helper function, because the itk::MultiThreader can only <br>
    * start a new thread with a static member function.
    */
    static ITK_THREAD_RETURN_TYPE StartWiiMoteThread(void* data);

    /**
    * Connects the Wiimote and allows access to its functionality.
    */
    void StartWiiMote();

    /**
    * Stops the running thread.
    */
    void StopWiiMote();

    /**
    * Reconnects the Wiimote in case the connection is lost. 
    */
    void ReconnectWiiMote();

    /**
    * TODO
    * Detects all available Wiimotes. 
    */
    void DetectWiiMotes();

    /**
    * Reads incoming data from the IR camera. After processing the data <br>
    * (e.g. computations, assigning commands...) fires Wiimote events accordingly. 
    */
    void WiiMoteIRInput();

   /**
    * Reads incoming data from buttons. After processing the data <br>
    * (e.g. computations, assigning commands...) fires Wiimote events accordingly. 
    *
    * @param buttonType
    *            the type of button, that was used to trigger this event
    */
    void WiiMoteButtonPressed(int buttonType);

    /**
    * Reads incoming data from the IR camera. Afterwards the raw x and y coordinates <br>
    * are stored in an event and fired as an event. 
    */
    void WiiMoteCalibrationInput();

  protected:

  private: 

    int m_ThreadID;
    itk::MultiThreader::Pointer m_MultiThreader;

    // mutex to control the flow of the method StartWiiMote()
    itk::FastMutexLock::Pointer m_WiiMoteThreadFinished;
    bool m_StopWiiMote;

    // access to the wiimote and parameter for callbackfromguithread
    wiimote m_WiiMote;
    ReceptorCommandPointer m_Command;

    // required for computation of movement
    Point2D m_LastReadData;
    double m_LastRecordTime;
    bool m_ReadDataOnce;

    // calibration
    bool m_InCalibrationMode;

    // compensate delay
    // release: Skip = 1
    // debug: 
    //int m_SkipTimeSteps;
    //int m_CurrentTimeStep;

    // to compensate delay, because normally
    // the data arrives too fast and is queued
    int m_SleepTime;

    //store all connected Wiimotes
    wiimote* m_WiiMotes[6];

  };
}

#endif // MITK_WIIMOTEHTREAD_H