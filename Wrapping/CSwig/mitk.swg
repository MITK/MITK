/* This is an itk-specific typemap used by CableSwig.  Also see comments
 * and "throws" code in CableSwig.cxx.
 *   -- Charl P. Botha <cpbotha AT ieee.org>
 */

#ifdef SWIGPYTHON

/* ------------------------------------------------------------
 * PyObject *  - Just pass straight through unmodified
 * This is default behaviour for python.swg, but Cable passes
 * a PyObject * through as a "p._object", so we redo the typemap
 * ------------------------------------------------------------ */
  
%typemap(in)   p._object "$1 = $input;";
%typemap(out)  p._object "$result = $1;";

#define %runtime     %insert("runtime")

%insert(runtime) %{
#if defined(MITK_WINDOWS_PYTHON_DEBUGGABLE)
# include "Python.h"
#else
# ifdef _DEBUG
#  undef _DEBUG
#  if defined(_MSC_VER) && _MSC_VER >= 1400
#    define _CRT_NOFORCE_MANIFEST 1
#  endif
#  include "Python.h"
#  define _DEBUG
# else
#  include "Python.h"
# endif
#endif
%}

#endif

%include exception.i

/* A "throws" attribute with the "std::exception" type is added synthetically
 * to each method node by CableSwig.cxx.  When gcc_xml starts passing through
 * correct throws types, this typemap could be optionally extended to
 * account for more different types.  For now this is sufficient though.
 */
 
%typemap(throws) std::exception {
   SWIG_exception(SWIG_RuntimeError, const_cast<char*>(_e.what()));
}

%include std_string.i

/* disable this c linkage warning on windows */
%{
#ifdef _WIN32
#pragma warning ( disable : 4190 )
#pragma warning ( disable : 4049 )
#endif
%}
