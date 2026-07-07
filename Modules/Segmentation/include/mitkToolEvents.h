/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkToolEvents_h
#define mitkToolEvents_h

#ifdef __GNUC__
#pragma GCC visibility push(default)
#endif
#include <itkEventObject.h>
#ifdef __GNUC__
#pragma GCC visibility pop
#endif

namespace mitk
{
/**
  \brief Basic tool event without any parameters
         Can simply be inherited using the itkEventMacro, e.g.
\code
namespace mitk
{

class MyTool : public Tool
{
  public:

    itkEventMacro(MySpecialEvent, ToolEvent);

  [...]

  protected:

  // Invoke your event like this
  void YourExampleMethod()
  {
    InvokeEvent( MySpecialEvent() );
  }
};

}
\endcode
*/

#ifdef __GNUC__
#pragma GCC visibility push(default)
#endif
  itkEventMacroDeclaration(ToolEvent, itk::ModifiedEvent); // Definition is in mitkTool.cpp
#ifdef __GNUC__
#pragma GCC visibility pop
#endif

  /**
    \brief Tool event with 1 parameter

    Can store one parameter for use within an observer. To derive your own special events, use the
  mitkToolEventMacro1Param macro.
  \code
  namespace mitk
  {

  class MyTool : public Tool
  {
    public:

      mitkToolEventMacro1Param(FooToolEvent, int);

    [...]

    protected:

    // Invoke your event like this
    void YourExampleMethod()
    {
      InvokeEvent( FooToolEvent(32) );
    }
  };

  }
  \endcode
  */
  /**
   * \brief Tool event carrying a single typed parameter.
   *
   * Use the mitkToolEventMacro1Param macro to derive named event types from this template.
   *
   * \tparam T The type of the parameter carried by the event.
   * \sa ToolEvent, TwoParameterToolEvent
   */
  template <typename T>
  class ParameterToolEvent : public ToolEvent
  {
  public:
    typedef ParameterToolEvent Self;
    typedef ToolEvent Superclass;

    /**
     * \brief Constructor with parameter value.
     * \param[in] parameter The parameter value to store.
     */
    ParameterToolEvent(const T parameter) : m_Parameter(parameter) {}

    /** \brief Copy constructor. */
    ParameterToolEvent(const Self &s) : ToolEvent(s), m_Parameter(s.m_Parameter) {}
    ~ParameterToolEvent() override {}
    const char *GetEventName() const override { return "ParameterToolEvent"; }
    bool CheckEvent(const ::itk::EventObject *e) const override { return dynamic_cast<const Self *>(e); }
    ::itk::EventObject *MakeObject() const override { return new Self(m_Parameter); }

    /**
     * \brief Returns the parameter value carried by this event.
     * \return The parameter value.
     */
    const T GetParameter() const { return m_Parameter; }
  protected:
    const T m_Parameter;

  private:
    ParameterToolEvent();

    void operator=(const Self &);
  };

  /**
    \brief Tool event with 1 parameter

    Can store one parameter for use within an observer. To derive your own special events, use the
  mitkToolEventMacro1Param macro.
  \code
  namespace mitk
  {

  class MyTool : public Tool
  {
    public:

      mitkToolEventMacro1Param(FooToolEvent, int);

    [...]

    protected:

    // Invoke your event like this
    void YourExampleMethod()
    {
      InvokeEvent( BarToolEvent(32, false) );
    }
  };

  }
  \endcode
  */
  /**
   * \brief Tool event carrying two typed parameters.
   *
   * Use the mitkToolEventMacro2Param macro to derive named event types from this template.
   *
   * \tparam T Type of the first parameter.
   * \tparam U Type of the second parameter.
   * \sa ToolEvent, ParameterToolEvent
   */
  template <typename T, typename U>
  class TwoParameterToolEvent : public ToolEvent
  {
  public:
    typedef TwoParameterToolEvent Self;
    typedef ToolEvent Superclass;

    /**
     * \brief Constructor with two parameter values.
     * \param[in] parameter1 The first parameter value.
     * \param[in] parameter2 The second parameter value.
     */
    TwoParameterToolEvent(const T parameter1, const U parameter2) : m_Parameter1(parameter1), m_Parameter2(parameter2)
    {
    }

    /** \brief Copy constructor. */
    TwoParameterToolEvent(const Self &s) : ToolEvent(s), m_Parameter1(s.m_Parameter1), m_Parameter2(s.m_Parameter2) {}
    ~TwoParameterToolEvent() override {}
    const char *GetEventName() const override { return "TwoParameterToolEvent"; }
    bool CheckEvent(const ::itk::EventObject *e) const override { return dynamic_cast<const Self *>(e); }
    ::itk::EventObject *MakeObject() const override { return new Self(m_Parameter1, m_Parameter2); }

    /** \brief Returns the first parameter value. */
    const T GetParameter1() const { return m_Parameter1; }

    /** \brief Returns the second parameter value. */
    const U GetParameter2() const { return m_Parameter2; }
  protected:
    const T m_Parameter1;
    const U m_Parameter2;

  private:
    TwoParameterToolEvent();

    void operator=(const Self &);
  };

  /** \brief Convenience typedef for a tool event carrying an int parameter. */
  typedef ParameterToolEvent<int> IntegerToolEvent;

  /** \brief Convenience typedef for a tool event carrying a float parameter. */
  typedef ParameterToolEvent<float> FloatToolEvent;

  /** \brief Convenience typedef for a tool event carrying a bool parameter. */
  typedef ParameterToolEvent<bool> BoolToolEvent;

} // namespace

// some macros to let tools define their own event classes as inner classes (should then inherit from something like
// FloatToolEvent
// inheritance, because it allows observers to distinguish events

#define mitkToolEventMacro(eventname, baseevent)                                                                       \
  \
class eventname : public baseevent\
{virtual const char * GetEventName() const {return #eventname;                                                         \
  }                                                                                                                    \
  \
}                                                                                                                 \
  ;

#define mitkToolEventMacro1Param(eventname, paramtype1)                                                                \
  \
class eventname : public ParameterToolEvent<paramtype1>                                                                \
  \
{                                                                                                                 \
  public:                                                                                                              \
    virtual const char *GetEventName() const { return #eventname "(" #paramtype1 ")"; }                                \
    eventname(const paramtype1 parameter) : ParameterToolEvent<paramtype1>(parameter) {}                               \
  private:                                                                                                             \
    eventname();                                                                                                       \
  \
};

#define mitkToolEventMacro2Param(eventname, paramtype1, paramtype2)                                                    \
  \
class eventname : public TwoParameterToolEvent<paramtype1, paramtype2>                                                 \
  \
{                                                                                                                 \
  public:                                                                                                              \
    virtual const char *GetEventName() const { return #eventname "(" #paramtype1 "," #paramtype2 ")"; }                \
    eventname(const paramtype1 parameter1, const paramtype2 parameter2)                                                \
      : TwoParameterToolEvent<paramtype1, paramtype2>(parameter1, parameter2)                                          \
    {                                                                                                                  \
    }                                                                                                                  \
                                                                                                                       \
  private:                                                                                                             \
    eventname();                                                                                                       \
  \
};

#endif
