/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKSTDFUNCTIONCOMMAND_H
#define MITKSTDFUNCTIONCOMMAND_H

#include <MitkCoreExports.h>

// itk
#include <itkCommand.h>

// c++
#include <functional>

namespace mitk
{
  // define custom command to accept std::functions as "filter" and as "action"
  class MITKCORE_EXPORT StdFunctionCommand : public itk::Command
  {
  public:
    using Self    =   StdFunctionCommand;
    using Pointer =   itk::SmartPointer<Self>;

    using FilterFunction = std::function<bool(const itk::EventObject&)>;
    using ActionFunction = std::function<void(const itk::EventObject&)>;

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Run-time type information (and related methods). */
    itkTypeMacro(StdFunctionCommand, itk::Command);

    void SetCommandFilter(FilterFunction stdFunctionFilter)
    {
      m_StdFilterFunction = stdFunctionFilter;
    }

    void SetCommandAction(ActionFunction stdFunctionAction)
    {
      m_StdActionFunction = stdFunctionAction;
    }

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

#endif // MITKSTDFUNCTIONCOMMAND_H
