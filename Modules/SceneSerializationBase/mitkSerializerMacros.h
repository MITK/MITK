/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <itkObjectFactoryBase.h>
#include <itkVersion.h>

#define MITK_REGISTER_SERIALIZER(classname) \
\
namespace mitk \
{ \
 \
class classname ## Factory : public ::itk::ObjectFactoryBase \
{ \
  public:   \
 \
    /* ITK typedefs */ \
    typedef classname ## Factory  Self; \
    typedef itk::ObjectFactoryBase  Superclass; \
    typedef itk::SmartPointer<Self>  Pointer; \
    typedef itk::SmartPointer<const Self>  ConstPointer; \
      \
    /* Methods from ObjectFactoryBase */ \
    virtual const char* GetITKSourceVersion() const \
    { \
      return ITK_SOURCE_VERSION; \
    } \
     \
    virtual const char* GetDescription() const \
    { \
      return "Generated factory for " #classname; \
    } \
     \
    /* Method for class instantiation. */ \
    itkFactorylessNewMacro(Self); \
       \
    /* Run-time type information (and related methods). */ \
    itkTypeMacro(classname ## Factory, itkObjectFactoryBase); \
 \
    /* Register one factory of this type  */ \
    static void RegisterOneFactory() \
    { \
      classname ## Factory::Pointer factory = classname ## Factory::New(); \
      itk::ObjectFactoryBase::RegisterFactory(factory); \
    } \
     \
  protected: \
 \
    classname ## Factory() \
    { \
      itk::ObjectFactoryBase::RegisterOverride(#classname, \
                              #classname, \
                              "Generated factory for " #classname, \
                              1, \
                              itk::CreateObjectFunction<classname>::New()); \
    } \
 \
    ~classname ## Factory() \
    { \
    } \
 \
  private: \
 \
    classname ## Factory(const Self&);    /* purposely not implemented */ \
    void operator=(const Self&);    /* purposely not implemented */ \
 \
}; \
 \
 \
 \
  class classname ## RegistrationMethod \
  { \
    public: \
 \
    classname ## RegistrationMethod() \
    { \
      itk::ObjectFactoryBase::RegisterFactory( classname ## Factory::New() ); \
    } \
  }; \
} \
 \
static mitk::classname ## RegistrationMethod somestaticinitializer_ ## classname ;

