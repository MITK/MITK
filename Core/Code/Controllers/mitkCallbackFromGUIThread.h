/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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

#ifndef MITK_CALLBACK_WITHIN_GUI_TREAD_H_INCLUDGEWQ
#define MITK_CALLBACK_WITHIN_GUI_TREAD_H_INCLUDGEWQ

#include<itkCommand.h>
#include <itkEventObject.h>
#include <MitkExports.h>

namespace mitk
{

  /*!
    \brief Used by CallbackFromGUIThread to pass parameters.
  */
  template <class T>
  class CallbackEventOneParameter : public itk::AnyEvent
  { 
  public: 
    typedef CallbackEventOneParameter Self; 
    typedef itk::AnyEvent Superclass; 

    CallbackEventOneParameter(const T t )
      : m_Data(t) {} 

    virtual ~CallbackEventOneParameter() {} 

    virtual const char * GetEventName() const 
    { 
      return "CallbackEventOneParameter"; 
    } 

    virtual bool CheckEvent(const ::itk::EventObject* e) const 
    { 
      return dynamic_cast<const Self*>(e); 
    } 

    virtual ::itk::EventObject* MakeObject() const 
    { 
      return new Self( m_Data ); 
    } 

    const T GetData() const 
    { 
      return m_Data;
    }
    
    CallbackEventOneParameter(const Self& s) : itk::AnyEvent(s), m_Data(s.m_Data) {}; 

  protected:
    const T m_Data;
  
  private: 
    void operator=(const Self&); 
  };


/*!
  \brief Toolkit specific implementation of mitk::CallbackFromGUIThread

  For any toolkit, this class has to be sub-classed. One instance of that sub-class has to
  be registered with mitk::CallbackFromGUIThread. See the (very simple) implmentation of 
  QmitkCallbackFromGUIThread for an example.
*/
class MITK_CORE_EXPORT CallbackFromGUIThreadImplementation
{
  public:
    
    /// Change the current application cursor
    virtual void CallThisFromGUIThread(itk::Command*, itk::EventObject*) = 0;

    virtual ~CallbackFromGUIThreadImplementation() {};

  protected:
  private:
};
  
/*!
  \brief Allows threads to call some method from within the GUI thread.

   This class is useful for use with GUI toolkits that are not thread-safe, e.g. Qt. Any thread that 
   needs to work with the GUI at some time during its execution (e.g. at the end, to display some results)
   can use this class to ask for a call to a member function from the GUI thread.

  <b>Usage example</b>

  We assume that you have a class ThreadedClass, that basically lives in a thread that is different
  from the GUI thread. Now this class has to change some element of the GUI to indicate its status. 
  This could be dangerous (with Qt it is for sure). 
  
  The solution is, that ThreadedClass asks mitk::CallbackFromGUIThread to call a method from the GUI 
  thread (main thread).

  Here is part of the header of ThreadedClass:

\code
class ThreadedClass : public ParentClass
{
  public:

    ... // All you need

    // This function runs in its own thread !
    void ThreadedFunction();

    // This should be called from the GUI thread
    void ChangeGUIElementsToIndicateProgress(const itk::EventObject&);

    ... 
};
\endcode

\code
#include "mitkCallbackFromGUIThread.h"
#include <itkCommand.h>


// This method runs in a thread of its own! So it can't manipulate GUI elements directly without causing trouble
void ThreadedClass::ThreadedFunction()
{

  ...

  // Create a command object (passing parameters comes later)
  itk::ReceptorMemberCommand<ThreadedClass>::Pointer command = itk::ReceptorMemberCommand<ThreadedClass>::New();
  command->SetCallbackFunction(this, &ThreadedClass::ChangeGUIElementsToIndicateProgress);

  // Ask to execute that command from the GUI thread
  mitk::CallbackFromGUIThread::GetInstance()->CallThisFromGUIThread(command);

  ...

}


// Do dangerous GUI changing stuff here
void ThreadedClass::ChangeGUIElementsToIndicateProgress(const itk::EventObject& e)
{
  Application::GetButtonGrid()->AddButton("Stop");    // this is pseudo code
}
\endcode

  This obviously won't allow you to pass parameters to ChangeGUIElementsToIndicateProgress. If you need to do that,
  you have to create a kind of itk::EventObject that can be asked for a parameter (this solution is not nice, if you see
  a better solution, please mail to mitk-users@lists.sourceforge.net).

  The itk::EventObject has to be created with "new" (which can also be done by calling MakeObject on an existing EventObject).

\code
    const mitk::OneParameterEvent* event = new mitk::OneParameterEvent(1); // this class is not yet defined but will be

    itk::ReceptorMemberCommand<ThreadedClass>::Pointer command = itk::ReceptorMemberCommand<ThreadedClass>::New();
    command->SetCallbackFunction(this, &ThreadedClass::ChangeGUIElementsToIndicateProgress);
    
    mitk::CallbackFromGUIThread::GetInstance()->CallThisFromGUIThread(command, event);

    // DO NOT delete event now. This will be done by CallThisFromGUIThread after the command will executed.
\endcode

\todo Create a set of "normal" parameter-event-objects that people might want to use.
*/
class MITK_CORE_EXPORT CallbackFromGUIThread
{
  public:

    /// This class is a singleton. 
    static CallbackFromGUIThread* GetInstance();

    /// To be called by a toolkit specific CallbackFromGUIThreadImplementation. 
    static void RegisterImplementation(CallbackFromGUIThreadImplementation* implementation);

    /// Change the current application cursor
    void CallThisFromGUIThread(itk::Command*, itk::EventObject* e = NULL);
    
  protected:

    /// Purposely hidden - singleton
    CallbackFromGUIThread();

  private:
    
    static CallbackFromGUIThreadImplementation* m_Implementation;
    static CallbackFromGUIThread* m_Instance;
};

} // namespace

#endif


