/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSerializerMacros_h
#define mitkSerializerMacros_h

/**
 * \file mitkSerializerMacros.h
 * \brief Provides the MITK_REGISTER_SERIALIZER macro for automatic registration of serializer classes.
 *
 * This header defines a macro that generates an ITK object factory and a static
 * registration helper for a given serializer class. The generated factory allows
 * the serializer to be discovered and instantiated at runtime through the ITK
 * object factory mechanism.
 *
 * \sa mitk::BaseDataSerializer, mitk::BasePropertySerializer
 */

#include <itkObjectFactoryBase.h>
#include <itkVersion.h>

/**
 * \brief Registers a serializer class with the ITK object factory system.
 *
 * This macro generates two helper classes for the given serializer class name:
 * - A factory class (classnameFactory) derived from itk::ObjectFactoryBase
 *   that creates instances of the serializer.
 * - A registration class (classnameRegistrationMethod) whose static instance
 *   registers the factory at program startup and unregisters it on shutdown.
 *
 * Usage: Place this macro in the global scope (outside any namespace) after
 * including the serializer header, typically in the serializer's .cpp file.
 *
 * \param classname The unqualified name of the serializer class to register.
 *                  The class must reside in the mitk namespace.
 *
 * \code
 * // In mitkMyPropertySerializer.cpp:
 * MITK_REGISTER_SERIALIZER(MyPropertySerializer);
 * \endcode
 */
#define MITK_REGISTER_SERIALIZER(classname)                                                                            \
  \
\
namespace mitk                                                                                                         \
  \
{                                                                                                                 \
    \
class classname##Factory : public ::itk::ObjectFactoryBase                                                             \
    \
{                                                                                                               \
    public:                                                                                                            \
      /* ITK typedefs */                                                                                               \
      typedef classname##Factory Self;                                                                                 \
      typedef itk::ObjectFactoryBase Superclass;                                                                       \
      typedef itk::SmartPointer<Self> Pointer;                                                                         \
      typedef itk::SmartPointer<const Self> ConstPointer;                                                              \
                                                                                                                       \
      /* Methods from ObjectFactoryBase */                                                                             \
      virtual const char *GetITKSourceVersion() const override { return ITK_SOURCE_VERSION; }                          \
      virtual const char *GetDescription() const override { return "Generated factory for " #classname; }              \
      /* Method for class instantiation. */                                                                            \
      itkFactorylessNewMacro(Self);                                                                                    \
                                                                                                                       \
      /* Run-time type information (and related methods). */                                                           \
      itkTypeMacro(classname##Factory, itkObjectFactoryBase);                                                          \
                                                                                                                       \
    protected:                                                                                                         \
      classname##Factory()                                                                                             \
      {                                                                                                                \
        itk::ObjectFactoryBase::RegisterOverride(#classname,                                                           \
                                                 #classname,                                                           \
                                                 "Generated factory for " #classname,                                  \
                                                 1,                                                                    \
                                                 itk::CreateObjectFunction<classname>::New());                         \
      }                                                                                                                \
                                                                                                                       \
      ~classname##Factory() {}                                                                                         \
    private:                                                                                                           \
      classname##Factory(const Self &); /* purposely not implemented */                                                \
      void operator=(const Self &);     /* purposely not implemented */                                                \
    \
};                                                                                                                     \
                                                                                                                       \
    class classname##RegistrationMethod                                                                                \
    {                                                                                                                  \
    public:                                                                                                            \
      classname##RegistrationMethod()                                                                                  \
      {                                                                                                                \
        m_Factory = classname##Factory::New();                                                                         \
        itk::ObjectFactoryBase::RegisterFactory(m_Factory);                                                            \
      }                                                                                                                \
                                                                                                                       \
      ~classname##RegistrationMethod() { itk::ObjectFactoryBase::UnRegisterFactory(m_Factory); }                       \
    private:                                                                                                           \
      classname##Factory::Pointer m_Factory;                                                                           \
    };                                                                                                                 \
  \
}                                                                                                                 \
  \
static mitk::classname##RegistrationMethod somestaticinitializer_##classname;

#endif
