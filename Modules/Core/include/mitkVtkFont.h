#pragma once

/// Utilities to use common unicode font in vtkTextProperty

#include <vtkTextProperty.h>

#include <MitkCoreExports.h>

namespace mitk {
  /// Configure vtkTextProperty object to use unicode font DejaVuSans from file
  MITKCORE_EXPORT void setUnicodeFont(vtkTextProperty* prop);
}