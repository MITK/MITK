/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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

    virtual void Execute(Object*, const itk::EventObject& event) override
    {
      if (m_StdFilterFunction && m_StdActionFunction)
      {
        if (m_StdFilterFunction(event))
        {
          m_StdActionFunction(event);
        }
      }
    }

    virtual void Execute(const Object*, const itk::EventObject& event) override
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

    virtual ~StdFunctionCommand() {}

  private:
    ITK_DISALLOW_COPY_AND_ASSIGN(StdFunctionCommand);
  };

} // end namespace mitk

#endif // MITKSTDFUNCTIONCOMMAND_H
