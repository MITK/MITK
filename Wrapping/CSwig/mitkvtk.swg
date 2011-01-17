%include exception.i

%{
#include "vtkImageImport.h"
#include "vtkImageExport.h"
#include "vtkImageData.h"
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

#endif
