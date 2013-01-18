%include exception.i

%{
#include "vtkImageImport.h"
#include "vtkImageExport.h"
#include "vtkImageData.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkPythonUtil.h"
%}

#ifdef SWIGPYTHON
%{
#include "vtkPythonUtil.h"
%}

%typemap(out) vtkImageExport* {
  PyImport_ImportModule("vtk");
  $result = vtkPythonUtil::GetObjectFromPointer ( (vtkImageExport*)$1 );
}

%typemap(out) vtkImageImport* {
  PyImport_ImportModule("vtk");
  $result = vtkPythonUtil::GetObjectFromPointer ( (vtkImageImport*)$1 );
}

%typemap(out) vtkImageData* {
  PyImport_ImportModule("vtk");
  $result = vtkPythonUtil::GetObjectFromPointer ( (vtkImageData*)$1 );
}

%typemap(in) vtkImageData* {
  $1 = NULL;
  $1 = (vtkImageData*) vtkPythonUtil::GetPointerFromObject ( $input, "vtkImageData" );
  if ( $1 == NULL ) { SWIG_fail; }
}

%typemap(out) vtkRenderWindow* {
  PyImport_ImportModule("vtk");
  $result = vtkPythonUtil::GetObjectFromPointer ( (vtkRenderWindow*)$1 );
}

%typemap(in) vtkRenderWindow* {
  $1 = NULL;
  $1 = (vtkRenderWindow*) vtkPythonUtil::GetPointerFromObject ( $input, "vtkRenderWindow" );
  if ( $1 == NULL ) { SWIG_fail; }
}

%typemap(out) vtkRenderer* {
  PyImport_ImportModule("vtk");
  $result = vtkPythonUtil::GetObjectFromPointer ( (vtkRenderer*)$1 );
}

%typemap(in) vtkRenderer* {
  $1 = NULL;
  $1 = (vtkRenderer*) vtkPythonUtil::GetPointerFromObject ( $input, "vtkRenderer" );
  if ( $1 == NULL ) { SWIG_fail; }
}

#endif
