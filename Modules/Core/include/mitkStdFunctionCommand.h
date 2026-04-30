/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkStdFunctionCommand_h
#define mitkStdFunctionCommand_h

#include <MitkCoreExports.h>

// itk
#include <itkCommand.h>

// c++
#include <functional>

namespace mitk
{
  /**
   * \brief ITK command that uses std::function objects as filter and action callbacks.
   *
   * This command accepts a filter function that decides whether to handle an event,
   * and an action function that is executed when the filter returns true.
   * Both functions must be set before the command will execute.
   */
  class MITKCORE_EXPORT StdFunctionCommand : public itk::Command
  {
  public:
    using Self    =   StdFunctionCommand;
    using Pointer =   itk::SmartPointer<Self>;

    /** \brief Function type for filtering events. Returns true if the event should be handled. */
    using FilterFunction = std::function<bool(const itk::EventObject&)>;

    /** \brief Function type for the action to execute when the filter passes. */
    using ActionFunction = std::function<void(const itk::EventObject&)>;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Run-time type information (and related methods). */
    itkTypeMacro(StdFunctionCommand, itk::Command);

    /**
     * \brief Set the filter function that decides whether to handle an event.
     *
     * \param[in] stdFunctionFilter the filter function.
     */
    void SetCommandFilter(FilterFunction stdFunctionFilter)
    {
      m_StdFilterFunction = stdFunctionFilter;
    }

    /**
     * \brief Set the action function to execute when the filter passes.
     *
     * \param[in] stdFunctionAction the action function.
     */
    void SetCommandAction(ActionFunction stdFunctionAction)
    {
      m_StdActionFunction = stdFunctionAction;
    }

    /**
     * \brief Execute the command for a non-const caller.
     *
     * If both filter and action functions are set and the filter returns true
     * for the given event, the action function is called.
     */
    void Execute(Object*, const itk::EventObject& event) override
    {
      if (m_StdFilterFunction && m_StdActionFunction)
      {
        if (m_StdFilterFunction(event))
        {
          m_StdActionFunction(event);
        }
      }
    }

    /**
     * \brief Execute the command for a const caller.
     *
     * If both filter and action functions are set and the filter returns true
     * for the given event, the action function is called.
     */
    void Execute(const Object*, const itk::EventObject& event) override
    {
      if (m_StdFilterFunction && m_StdActionFunction)
      {
        if (m_StdFilterFunction(event))
        {
          m_StdActionFunction(event);
        }
      }
    }

  protected:
    FilterFunction m_StdFilterFunction;
    ActionFunction m_StdActionFunction;

    StdFunctionCommand()
      : m_StdFilterFunction(nullptr)
      , m_StdActionFunction(nullptr)
    {}

    ~StdFunctionCommand() override {}

  private:
    ITK_DISALLOW_COPY_AND_ASSIGN(StdFunctionCommand);
  };

} // end namespace mitk

#endif
