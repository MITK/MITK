/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkToolCommand_h
#define mitkToolCommand_h

#include "itkCommand.h"
#include "mitkCommon.h"
#include <MitkSegmentationExports.h>

namespace mitk
{
  /**
  * \brief A command to get tool process feedback.
  *
  * \sa ProgressBar
  *
  */
  class MITKSEGMENTATION_EXPORT ToolCommand : public itk::Command
  {
  public:
    typedef ToolCommand Self;
    typedef itk::Command Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /**
      * \brief Reacts on events from ITK filters.
      *
      */
      void Execute(itk::Object *caller, const itk::EventObject &event) override;

    /**
    * \brief Not implemented...
    *
    */
    void Execute(const itk::Object *object, const itk::EventObject &event) override;

    /**
    * \brief Add new steps to the progress bar.
    *
    */
    void AddStepsToDo(int steps);

    /**
    * \brief Sets the remaining progress to the progress bar when the optimization process is done.
    *
    */
    void SetProgress(int steps);

    /**
    * \brief Returns the current progress value.
    *
    */
    double GetCurrentProgressValue();

    /**
    * \brief Sets the stop processing flag, which is used to call ...
    *
    */
    void SetStopProcessing(bool value);

  protected:
    ToolCommand();

  private:
    double m_ProgressValue;
    bool m_StopProcessing;
  };

} // namespace mitk

#endif
