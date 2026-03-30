/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVtkLoggingAdapter_h
#define mitkVtkLoggingAdapter_h

#include <MitkCoreExports.h>
#include <vtkOutputWindow.h>
#include <vtkSmartPointer.h>

namespace mitk
{
  /**
   * \brief Adapter that redirects VTK logging output to the MITK logging system.
   *
   * Replaces the standard vtkOutputWindow so that all VTK log messages
   * (text, error, warning, debug) are forwarded to the MITK logging framework.
   *
   * \ingroup IO
   */
  class MITKCORE_EXPORT VtkLoggingAdapter : public vtkOutputWindow
  {
  public:
    static VtkLoggingAdapter *New();

    /**
     * \brief Initialize the logging adapter.
     *
     * After calling this method, all VTK logging messages are redirected
     * to the MITK logging system.
     */
    static void Initialize();

    /**
     * \brief Display a general text message via MITK_INFO.
     * \param[in] t The text message to display.
     */
    void DisplayText(const char *t) override;

    /**
     * \brief Display an error message via MITK_ERROR.
     * \param[in] t The error text to display.
     */
    void DisplayErrorText(const char *t) override;

    /**
     * \brief Display a warning message via MITK_WARN.
     * \param[in] t The warning text to display.
     */
    void DisplayWarningText(const char *t) override;

    /**
     * \brief Display a generic warning message via MITK_WARN.
     * \param[in] t The warning text to display.
     */
    void DisplayGenericWarningText(const char *t) override;

    /**
     * \brief Display a debug message via MITK_DEBUG.
     * \param[in] t The debug text to display.
     */
    void DisplayDebugText(const char *t) override;

  protected:
  };

} // namespace mitk

#endif
