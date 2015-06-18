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

#ifndef _mitkToolCommand_H
#define _mitkToolCommand_H

#include "itkCommand.h"
#include "mitkCommon.h"
#include <MitkSegmentationExports.h>

namespace mitk {

  /**
  * \brief A command to get tool process feedback.
  *
  * \sa ProgressBar
  *
  */
  class MITKSEGMENTATION_EXPORT ToolCommand : public itk::Command
  {
    public:
      typedef  ToolCommand   Self;
      typedef  itk::Command                Superclass;
      typedef  itk::SmartPointer<Self>     Pointer;
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      /**
      * \brief Reacts on events from ITK filters.
      *
      */
      void Execute(itk::Object *caller, const itk::EventObject & event) override;

      /**
      * \brief Not implemented...
      *
      */
      void Execute(const itk::Object * object, const itk::EventObject & event) override;

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

#endif // _mitkToolCommand_H
