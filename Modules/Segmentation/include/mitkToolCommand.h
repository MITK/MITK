/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkToolCommand_h
#define mitkToolCommand_h

#include <itkCommand.h>
#include <mitkCommon.h>
#include <MitkSegmentationExports.h>

namespace mitk
{
  /**
   * \brief ITK command for tracking progress of segmentation tool operations.
   *
   * This command can be registered as an observer on ITK filters to receive
   * ProgressEvent and IterationEvent notifications. It updates the MITK
   * progress bar accordingly.
   *
   * \sa SegWithPreviewTool, ProgressBar
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
     * \brief Handles events from ITK filters (e.g. ProgressEvent, IterationEvent).
     * \param[in] caller The ITK object that triggered the event.
     * \param[in] event The event object.
     */
    void Execute(itk::Object *caller, const itk::EventObject &event) override;

    /**
     * \brief Const version of Execute (not implemented).
     * \param[in] object The ITK object that triggered the event.
     * \param[in] event The event object.
     */
    void Execute(const itk::Object *object, const itk::EventObject &event) override;

    /**
     * \brief Adds new steps to the progress bar total.
     * \param[in] steps Number of steps to add.
     */
    void AddStepsToDo(int steps);

    /**
     * \brief Sets the progress to the given number of completed steps.
     * \param[in] steps Number of completed steps.
     */
    void SetProgress(int steps);

    /**
     * \brief Returns the current progress value.
     * \return Current progress as a double.
     */
    double GetCurrentProgressValue();

    /**
     * \brief Sets the stop processing flag to abort the current operation.
     * \param[in] value If true, signals the operation to stop.
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
