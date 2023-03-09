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
   * @brief Adapter that overwrites the standard vtk logging output window and sends the logging messages to the MITK
   * logging instead.
   * @ingroup IO
   */
  class MITKCORE_EXPORT VtkLoggingAdapter : public vtkOutputWindow
  {
  public:
    static VtkLoggingAdapter *New();

    /** @brief Initializes the logging adapter. Vtk logging
      *        messages are redirected to MITK logging afterwards.
      */
    static void Initialize();

    void DisplayText(const char *t) override;

    void DisplayErrorText(const char *t) override;

    void DisplayWarningText(const char *t) override;

    void DisplayGenericWarningText(const char *t) override;

    void DisplayDebugText(const char *t) override;

  protected:
  };

} // namespace mitk

#endif
