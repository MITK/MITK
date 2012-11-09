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


#ifndef VtkLoggingAdapter_H_HEADER_INCLUDED
#define VtkLoggingAdapter_H_HEADER_INCLUDED

#include <MitkExports.h>
#include <vtkOutputWindow.h>
#include <vtkSmartPointer.h>

namespace mitk {
//##Documentation
//## @brief Adapter that overwrites the standard vtk logging output window and sends the logging messages to the MITK logging instead.
//## @ingroup IO
class MITK_CORE_EXPORT VtkLoggingAdapter : public vtkOutputWindow
{
public:

  static VtkLoggingAdapter* New();

  /** @brief Initializes the logging adapter. Vtk logging
    *        messages are redirected to MITK logging afterwards.
    */
  static void Initialize();

  virtual void DisplayText(const char* t);

  virtual void DisplayErrorText(const char *t);

  virtual void DisplayWarningText(const char *t);

  virtual void DisplayGenericWarningText(const char *t);

  virtual void DisplayDebugText(const char *t);


protected:

};

} // namespace mitk

#endif /* VtkLoggingAdapter_H_HEADER_INCLUDED */
