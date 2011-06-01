%include exception.i

%{
#include "vtkImageImport.h"
#include "vtkImageExport.h"
#include "vtkImageData.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
%}

#ifdef SWIGPYTHON
%{
#include "vtkPythonUtil.h"
%}

%typemap(out) vtkImageExport* {
  PyImport_ImportModule("vtk");
  $result = vtkPythonGetObjectFromPointer ( (vtkImageExport*)$1 );
}

%typemap(out) vtkImageImport* {
  PyImport_ImportModule("vtk");
  $result = vtkPythonGetObjectFromPointer ( (vtkImageImport*)$1 );
}

%typemap(out) vtkImageData* {
  PyImport_ImportModule("vtk");
  $result = vtkPythonGetObjectFromPointer ( (vtkImageData*)$1 );
}

%typemap(in) vtkImageData* {
  $1 = NULL;
  $1 = (vtkImageData*) vtkPythonGetPointerFromObject ( $input, "vtkImageData" );
  if ( $1 == NULL ) { SWIG_fail; }
}

%typemap(out) vtkRenderWindow* {
  PyImport_ImportModule("vtk");
  $result = vtkPythonGetObjectFromPointer ( (vtkRenderWindow*)$1 );
}

%typemap(in) vtkRenderWindow* {
  $1 = NULL;
  $1 = (vtkRenderWindow*) vtkPythonGetPointerFromObject ( $input, "vtkRenderWindow" );
  if ( $1 == NULL ) { SWIG_fail; }
}

%typemap(out) vtkRenderer* {
  PyImport_ImportModule("vtk");
  $result = vtkPythonGetObjectFromPointer ( (vtkRenderer*)$1 );
}

%typemap(in) vtkRenderer* {
  $1 = NULL;
  $1 = (vtkRenderer*) vtkPythonGetPointerFromObject ( $input, "vtkRenderer" );
  if ( $1 == NULL ) { SWIG_fail; }
}

#endif
