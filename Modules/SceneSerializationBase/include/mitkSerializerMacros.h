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
    virtual const char* GetITKSourceVersion() const override \
    { \
      return ITK_SOURCE_VERSION; \
    } \
     \
    virtual const char* GetDescription() const override \
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
      m_Factory = classname ## Factory::New(); \
      itk::ObjectFactoryBase::RegisterFactory( m_Factory ); \
    } \
 \
    ~classname ## RegistrationMethod() \
    { \
      itk::ObjectFactoryBase::UnRegisterFactory( m_Factory ); \
    } \
 \
    private: \
 \
    classname ## Factory::Pointer m_Factory; \
 \
  }; \
} \
 \
static mitk::classname ## RegistrationMethod somestaticinitializer_ ## classname ;

