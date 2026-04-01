/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCallbackFromGUIThread_h
#define mitkCallbackFromGUIThread_h

#include <MitkCoreExports.h>
#include <itkCommand.h>
#include <itkEventObject.h>

namespace mitk
{
  /**
   * \brief ITK event that carries a single typed parameter for CallbackFromGUIThread.
   *
   * \tparam T The type of the parameter to carry with the event.
   *
   * \sa CallbackFromGUIThread
   */
  template <class T>
  class CallbackEventOneParameter : public itk::AnyEvent
  {
  public:
    typedef CallbackEventOneParameter Self;
    typedef itk::AnyEvent Superclass;

    /** \brief Construct with the given data payload.  */
    CallbackEventOneParameter(const T t) : m_Data(t) {}
    ~CallbackEventOneParameter() override {}
    const char *GetEventName() const override { return "CallbackEventOneParameter"; }
    bool CheckEvent(const ::itk::EventObject *e) const override { return dynamic_cast<const Self *>(e); }
    ::itk::EventObject *MakeObject() const override { return new Self(m_Data); }

    /** \brief Retrieve the stored data payload.
     * \return The data value passed at construction time.
     */
    const T GetData() const { return m_Data; }
    CallbackEventOneParameter(const Self &s) : itk::AnyEvent(s), m_Data(s.m_Data){};

  protected:
    const T m_Data;

  private:
    void operator=(const Self &);
  };

  /**
   * \brief Toolkit-specific implementation of mitk::CallbackFromGUIThread.
   *
   * For any GUI toolkit, this class must be sub-classed. One instance of that
   * sub-class must be registered with mitk::CallbackFromGUIThread. See the
   * implementation of QmitkCallbackFromGUIThread for an example.
   *
   * \sa CallbackFromGUIThread
   */
  class MITKCORE_EXPORT CallbackFromGUIThreadImplementation
  {
  public:
    /**
     * \brief Execute the given command from the GUI thread.
     *
     * Implementations must schedule the command for execution on the GUI
     * (main) thread.
     */
    virtual void CallThisFromGUIThread(itk::Command *, itk::EventObject *) = 0;

    virtual ~CallbackFromGUIThreadImplementation(){};

  protected:
  private:
  };

  /**
   * \brief Allows threads to call some method from within the GUI thread.
   *
   * This class is useful for use with GUI toolkits that are not thread-safe,
   * e.g. Qt. Any thread that needs to work with the GUI at some time during
   * its execution (e.g. at the end, to display some results) can use this
   * class to ask for a call to a member function from the GUI thread.
   *
   * <b>Usage example</b>

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
  #include <mitkCallbackFromGUIThread.h>
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
    you have to create a kind of itk::EventObject that can be asked for a parameter (this solution is not nice, if you
  see
    a better solution, please mail to mitk-users@lists.sourceforge.net).

    The itk::EventObject has to be created with "new" (which can also be done by calling MakeObject on an existing
  EventObject).

  \code
      const mitk::OneParameterEvent* event = new mitk::OneParameterEvent(1); // this class is not yet defined but will
  be

      itk::ReceptorMemberCommand<ThreadedClass>::Pointer command = itk::ReceptorMemberCommand<ThreadedClass>::New();
      command->SetCallbackFunction(this, &ThreadedClass::ChangeGUIElementsToIndicateProgress);

      mitk::CallbackFromGUIThread::GetInstance()->CallThisFromGUIThread(command, event);

      // DO NOT delete event now. This will be done by CallThisFromGUIThread after the command will executed.
  \endcode

   * \todo Create a set of "normal" parameter-event-objects that people might want to use.
   *
   * \sa CallbackFromGUIThreadImplementation CallbackEventOneParameter
   */
  class MITKCORE_EXPORT CallbackFromGUIThread
  {
  public:
    /**
     * \brief Return the singleton instance.
     * \return Pointer to the single CallbackFromGUIThread instance.
     */
    static CallbackFromGUIThread *GetInstance();

    /**
     * \brief Register a toolkit-specific implementation.
     *
     * Must be called once by a toolkit-specific sub-class of
     * CallbackFromGUIThreadImplementation before CallThisFromGUIThread
     * can be used.
     *
     * \param implementation The toolkit-specific implementation to register.
     */
    static void RegisterImplementation(CallbackFromGUIThreadImplementation *implementation);

    /**
     * \brief Schedule the given command for execution on the GUI thread.
     *
     * The optional event object can carry parameters that are passed to
     * the command. If provided, ownership of the event is transferred
     * and it will be deleted after the command executes.
     *
     * \param e Optional event object carrying parameters (may be nullptr).
     */
    void CallThisFromGUIThread(itk::Command *, itk::EventObject *e = nullptr);

  protected:
    /** \brief Hidden constructor (singleton pattern). */
    CallbackFromGUIThread();

  private:
    static CallbackFromGUIThreadImplementation *m_Implementation;
    static CallbackFromGUIThread *m_Instance;
  };

} // namespace

#endif
