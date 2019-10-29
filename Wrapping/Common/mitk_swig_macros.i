//
// This file contains macros for swig.
//



//
// MITKSWIG_ADD_HEADERFILE includes a header-file into SWIG
//
%define MITKSWIG_ADD_HEADERFILE( classinclude )
  // Include the given header, where the class definition is found
  %include < ## classinclude ## >

  // Include the include file in the generated cpp file
  %{
   #include < ## classinclude ## >
  %}

%enddef

//
// MITKSWIG_ADD_CLASS is a helper macro in order to do
// all important stuff in order to wrap an existing
// class
//
%define MITKSWIG_ADD_CLASS(classname, classinclude, nspace)
  MITKSWIG_ADD_HEADERFILE( classinclude )
  // Using class name in order to remove ambigiouties
  %{
    typedef nspace ## :: ## classname classname ## ;
    using nspace ## :: ## classname ;
  %}
  using nspace ##:: ## classname ;

  // Typedef is necessary to overcome ambigiouties resulting in the fact that SWIG
  // ignores namespaces. This can lead to some problems with templates.
  typedef nspace ## :: ## classname classname ## ;
%enddef


//
// MITKSWIG_AUTOMATED_CASTING is a helper macro in order to
// provide a convinience interface for up/downcasting of
// classes
//
%define MITKSWIG_AUTOMATED_CASTING(classname, classinclude, nspace)

  // Define a conversion method to convert from and to types
  %template(ConvertTo ## classname) ConvertTo< nspace ##:: ## classname ## >;

  // This extend is necessary to have the automatic cast methods available
%extend itk::SmartPointer< nspace ## :: ## classname ## ::Self> {
  %pythoncode %{
      def _GetListOfValidItems(self):
        return [str(k) for k in self.GetClassHierarchy() if str(k) in convertion_list.keys() ]
  %}
  %pythoncode %{
    def __getattr__(self, item):
        if type(item)==str:
            if (len(item) > 9) and ('ConvertTo' in item):
                searchString=item[9:]
                if searchString in self._GetListOfValidItems():
                    def func_t():
                        return convertion_list[searchString](self)
                    return func_t
  %}
  %pythoncode %{
    def __dir__(self):
        return super().__dir__() + ['ConvertTo'+k for k in self._GetListOfValidItems()]
  %}
}

%extend  std::vector< nspace ## :: ## classname *>::value_type {
  %pythoncode %{
      def _GetListOfValidItems(self):
        return [str(k) for k in self.GetClassHierarchy() if str(k) in convertion_list.keys() ]
  %}
  %pythoncode %{
    def __getattr__(self, item):
        if type(item)==str:
            if (len(item) > 9) and ('ConvertTo' in item):
                searchString=item[9:]
                if searchString in self._GetListOfValidItems():
                    def func_t():
                        return convertion_list[searchString](self)
                    return func_t
  %}
  %pythoncode %{
    def __dir__(self):
        return super().__dir__() + ['ConvertTo'+k for k in self._GetListOfValidItems()]
  %}
}

%pythoncode %{
 convertion_list['classname'] = ConvertTo ## classname
%}

%enddef

//
// MITKSWIG_SMARTPOINTERVECTOR : Wrapper for Vectors of Smartpointer-Classes
//
%define MITKSWIG_SMARTPOINTERVECTOR(classname, classinclude, nspace)
  // Initianziation of std. vectors containing pointers to these classes. This allows to use
  // vectors of these types as target language arrays.
  %template(Vector ## classname ## Pointer) std::vector< nspace ## :: ## classname ## ::Pointer >;
  %template(Vector ## classname) std::vector< nspace ## :: ## classname ## ::Self *>;
%enddef

//
// MITKSWIG_POINTERVECTOR : Wrapper for Vectors of Classes
//
%define MITKSWIG_POINTERVECTOR(classname, classinclude, nspace)
  // Initianziation of std. vectors containing pointers to these classes. This allows to use
  // vectors of these types as target language arrays.
  %template(Vector ## classname ## Pointer) std::vector< nspace ## :: ## classname * >;
  %template(Vector ## classname) std::vector< nspace ## :: ## classname  >;
%enddef

//
// MITKSWIG_MITKSMARTPOINTER_INITIALIZATION : Wrapper for Vectors of Smartpointer-Classes
//
%define MITKSWIG_MITKSMARTPOINTER_INITIALIZATION(classname, classinclude, nspace)

  // Declaring that this class is a smart-pointer class, in order to handle
  // online upcasting where necessary (for example python)
  %feature("smartptr", noblock=1) nspace ##:: ## classname { itk::SmartPointer<nspace ## :: ## classname ## ::Self> }
%enddef

//
// MITKSWIG_MITKSMARTPOINTER_TEMPLATE : Wrapper for Vectors of Smartpointer-Classes
//
%define MITKSWIG_MITKSMARTPOINTER_TEMPLATE(classname, classinclude, nspace)
  // Defining the Smartpointer, allows easy access in target language
  %template(classname ## Pointer) itk::SmartPointer<nspace ## :: ## classname ## ::Self>;

%enddef


//
// SWIG_ADD_MITK_CLASS is a helper macro in order to do
// all important stuff before an mitk::Class is included.
// Requires the name of the class as it is in c++ as classname
// and the include file, in which the class is defined.
// It is assumed that the class is somehow inherited from
// mitk::BaseData, and supports smartpointers.
//
%define SWIG_ADD_MITK_CLASS_VECTORFREE(classname, classinclude, nspace)
  %include < ## classinclude ## >
  MITKSWIG_MITKSMARTPOINTER_INITIALIZATION(classname, classinclude, nspace)

  MITKSWIG_ADD_CLASS( classname, classinclude, nspace )

  class nspace ## :: ## classname ## ;
  class nspace ## :: ## classname ## ::Pointer;

  MITKSWIG_MITKSMARTPOINTER_TEMPLATE(classname, classinclude, nspace)

  MITKSWIG_AUTOMATED_CASTING(classname, classinclude, nspace)
%enddef

//
// SWIG_ADD_MITK_CLASS is a helper macro in order to do
// all important stuff before an mitk::Class is included.
// Requires the name of the class as it is in c++ as classname
// and the include file, in which the class is defined.
// It is assumed that the class is somehow inherited from
// mitk::BaseData, and supports smartpointers.
//
%define SWIG_ADD_MITK_CLASS(classname, classinclude, nspace)
  %include < ## classinclude ## >
  MITKSWIG_MITKSMARTPOINTER_INITIALIZATION(classname, classinclude, nspace)

  MITKSWIG_ADD_CLASS( classname, classinclude, nspace )


  class nspace ## :: ## classname ## ;
  class nspace ## :: ## classname ## ::Pointer;

  // It is important to first define the Vectors and
  // then define the Smartpointer. Otherwise a SWIG-bug ...
  MITKSWIG_SMARTPOINTERVECTOR(classname, classinclude, nspace)

  MITKSWIG_MITKSMARTPOINTER_TEMPLATE(classname, classinclude, nspace)

  MITKSWIG_AUTOMATED_CASTING(classname, classinclude, nspace)
%enddef


//
// SWIG_ADD_NONOBJECT_CLASS is a helper macro in order to do
// all important stuff before an mitk::Class is included.
// Requires the name of the class as it is in c++ as classname
// and the include file, in which the class is defined.
// It is assumed that the class is somehow inherited from
// mitk::BaseData, and supports smartpointers.
//
%define SWIG_ADD_NONOBJECT_CLASS(classname, classinclude, nspace)
  %include < ## classinclude ## >
  MITKSWIG_ADD_CLASS( classname, classinclude, nspace )

  // Typedef is necessary to overcome ambigiouties resulting in the fact that SWIG
  // ignores namespaces. This can lead to some problems with templates.
  typedef nspace ## :: ## classname classname ## ;

  class nspace ## :: ## classname ## ;

  MITKSWIG_POINTERVECTOR(classname, classinclude, nspace)
%enddef


//
// SWIG_ADD_NONOBJECT_CLASS is a helper macro in order to do
// all important stuff before an mitk::Class is included.
// Requires the name of the class as it is in c++ as classname
// and the include file, in which the class is defined.
// It is assumed that the class is somehow inherited from
// mitk::BaseData, and supports smartpointers.
//
%define SWIG_ADD_NONOBJECT_NOVECTOR_CLASS(classname, classinclude, nspace)
  %include < ## classinclude ## >
  MITKSWIG_ADD_CLASS( classname, classinclude, nspace )

  // Typedef is necessary to overcome ambigiouties resulting in the fact that SWIG
  // ignores namespaces. This can lead to some problems with templates.
  typedef nspace ## :: ## classname classname ## ;

  // Initianziation of std. vectors containing pointers to these classes. This allows to use
  // vectors of these types as target language arrays.
  %template(Vector ## classname ## Pointer) std::vector< nspace ## :: ## classname * >;

%enddef
