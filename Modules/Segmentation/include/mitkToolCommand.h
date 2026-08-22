/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkToolCommand_h
#define mitkToolCommand_h

#include <MitkSegmentationExports.h>

#include <itkCommand.h>

namespace mitk
{
  class ProgressTask;

  /**
   * \brief Forwards the progress of an observed ITK filter to a ProgressTask.
   *
   * Attach it to a filter as an observer of itk::ProgressEvent. If the task it
   * reports into is cancelled, the command aborts the observed filter, which
   * is the only place that can act on the request.
   *
   * \sa SegWithPreviewTool, ProgressTask
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
     * \brief Report the progress of the caller and honour a cancel request.
     *
     * \param[in] caller The ITK object that triggered the event.
     * \param[in] event The event object.
     */
    void Execute(itk::Object *caller, const itk::EventObject &event) override;

    /**
     * \brief Report the progress of the caller.
     *
     * \param[in] caller The ITK object that triggered the event.
     * \param[in] event The event object.
     */
    void Execute(const itk::Object *caller, const itk::EventObject &event) override;

    /**
     * \brief Report into the given task, or nowhere if it is nullptr.
     *
     * The command does not own the task and must not outlive it.
     *
     * \param[in] task The task of the operation the observed filter is part of.
     */
    void SetProgressTask(ProgressTask *task);

  protected:
    ToolCommand();

  private:
    ProgressTask *m_ProgressTask;
  };

} // namespace mitk

#endif
