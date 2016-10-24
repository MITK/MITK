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

#ifndef MITK_COMMON_H_DEFINED
#define MITK_COMMON_H_DEFINED

#ifdef _MSC_VER
// This warns about truncation to 255 characters in debug/browse info
#pragma warning(disable : 4786)
#pragma warning(disable : 4068) /* disable unknown pragma warnings */
#endif

// add only those headers here that are really necessary for all classes!
#include "itkObject.h"
#include "mitkConfig.h"
#include "mitkExceptionMacro.h"
#include "mitkGetClassHierarchy.h"
#include "mitkLogMacros.h"

#ifndef MITK_UNMANGLE_IPPIC
#define mitkIpPicDescriptor mitkIpPicDescriptor
#endif

typedef unsigned int MapperSlotId;

/** From ITK 4.7 version, the TypeMacro overrides (by using the explicit attribute) the GetNameOfClass
 * hence the SuperClass must provide one.
 *
 * If not, use the mitkClassMacroNoParent version
 */
#define mitkClassMacro(className, SuperClassName)                                                                      \
  typedef className Self;                                                                                              \
  typedef SuperClassName Superclass;                                                                                   \
  typedef itk::SmartPointer<Self> Pointer;                                                                             \
  typedef itk::SmartPointer<const Self> ConstPointer;                                                                  \
  static const char *GetStaticNameOfClass() { return #className; }                                                     \
  virtual std::vector<std::string> GetClassHierarchy() const override { return mitk::GetClassHierarchy<Self>(); }      \
  itkTypeMacro(className, SuperClassName)

#define mitkClassMacroItkParent(className, SuperClassName)                                                             \
  typedef className Self;                                                                                              \
  typedef SuperClassName Superclass;                                                                                   \
  typedef itk::SmartPointer<Self> Pointer;                                                                             \
  typedef itk::SmartPointer<const Self> ConstPointer;                                                                  \
  static const char *GetStaticNameOfClass() { return #className; }                                                     \
  virtual std::vector<std::string> GetClassHierarchy() const { return mitk::GetClassHierarchy<Self>(); }               \
  itkTypeMacro(className, SuperClassName)

/** At version 4.7 provides two type macros, the normal one expects the Superclass to provide the
 *  GetNameOfClass explicitely, the NoParent deos not expect anything.
 */
#define mitkClassMacroNoParent(className)                                                                              \
  typedef className Self;                                                                                              \
  typedef itk::SmartPointer<Self> Pointer;                                                                             \
  typedef itk::SmartPointer<const Self> ConstPointer;                                                                  \
  static const char *GetStaticNameOfClass() { return #className; }                                                     \
  virtual std::vector<std::string> GetClassHierarchy() const { return mitk::GetClassHierarchy<Self>(); }               \
  itkTypeMacroNoParent(className)

/**
* Macro for Constructors with one parameter for classes derived from itk::Lightobject
**/
#define mitkNewMacro1Param(classname, type)                                                                            \
                                                                                                                       \
  static Pointer New(type _arg)                                                                                        \
                                                                                                                       \
  {                                                                                                                    \
    Pointer smartPtr = new classname(_arg);                                                                            \
    smartPtr->UnRegister();                                                                                            \
    return smartPtr;                                                                                                   \
  }

/**
* Macro for Constructors with two parameters for classes derived from itk::Lightobject
**/
#define mitkNewMacro2Param(classname, typea, typeb)                                                                    \
                                                                                                                       \
  static Pointer New(typea _arga, typeb _argb)                                                                         \
                                                                                                                       \
  {                                                                                                                    \
    Pointer smartPtr = new classname(_arga, _argb);                                                                    \
    smartPtr->UnRegister();                                                                                            \
    return smartPtr;                                                                                                   \
  }

/**
* Macro for Constructors with three parameters for classes derived from itk::Lightobject
**/
#define mitkNewMacro3Param(classname, typea, typeb, typec)                                                             \
                                                                                                                       \
  static Pointer New(typea _arga, typeb _argb, typec _argc)                                                            \
                                                                                                                       \
  {                                                                                                                    \
    Pointer smartPtr = new classname(_arga, _argb, _argc);                                                             \
    smartPtr->UnRegister();                                                                                            \
    return smartPtr;                                                                                                   \
  }

/**
* Macro for Constructors with four parameters for classes derived from itk::Lightobject
**/
#define mitkNewMacro4Param(classname, typea, typeb, typec, typed)                                                      \
                                                                                                                       \
  static Pointer New(typea _arga, typeb _argb, typec _argc, typed _argd)                                               \
                                                                                                                       \
  {                                                                                                                    \
    Pointer smartPtr = new classname(_arga, _argb, _argc, _argd);                                                      \
    smartPtr->UnRegister();                                                                                            \
    return smartPtr;                                                                                                   \
  }

/**
* Macro for Constructors with five parameters for classes derived from itk::Lightobject
**/
#define mitkNewMacro5Param(classname, typea, typeb, typec, typed, typee)                                               \
                                                                                                                       \
  static Pointer New(typea _arga, typeb _argb, typec _argc, typed _argd, typee _arge)                                  \
                                                                                                                       \
  {                                                                                                                    \
    Pointer smartPtr = new classname(_arga, _argb, _argc, _argd, _arge);                                               \
    smartPtr->UnRegister();                                                                                            \
    return smartPtr;                                                                                                   \
  }

/**
* Macro for Constructors with six parameters for classes derived from itk::Lightobject
**/
#define mitkNewMacro6Param(classname, typea, typeb, typec, typed, typee, typef)                                        \
                                                                                                                       \
  static Pointer New(typea _arga, typeb _argb, typec _argc, typed _argd, typee _arge, typef _argf)                     \
                                                                                                                       \
  {                                                                                                                    \
    Pointer smartPtr = new classname(_arga, _argb, _argc, _argd, _arge, _argf);                                        \
    smartPtr->UnRegister();                                                                                            \
    return smartPtr;                                                                                                   \
  }

/** Get a smart const pointer to an object.  Creates the member
 * Get"name"() (e.g., GetPoints()). */
#define mitkGetObjectMacroConst(name, type)                                                                            \
  virtual type *Get##name() const                                                                                      \
  {                                                                                                                    \
    itkDebugMacro("returning " #name " address " << this->m_##name);                                                   \
    return this->m_##name.GetPointer();                                                                                \
  }

/** Creates a Clone() method for "Classname". Returns a smartPtr of a clone of the
calling object*/
#define mitkCloneMacro(classname)                                                                                      \
  virtual itk::LightObject::Pointer InternalClone() const override                                                     \
                                                                                                                       \
  {                                                                                                                    \
    Pointer smartPtr = new classname(*this);                                                                           \
    smartPtr->UnRegister();                                                                                            \
    return smartPtr.GetPointer();                                                                                      \
  }

/** cross-platform deprecation macro

  \todo maybe there is something in external toolkits (ITK, VTK,...) that we could reulse -- would be much preferable
*/
#ifdef MITK_NO_DEPRECATED_WARNINGS
#define DEPRECATED(func) func
#elif defined(__GNUC__)
#define DEPRECATED(...) __VA_ARGS__ __attribute__((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED(...) __declspec(deprecated)##__VA_ARGS__
#else
#pragma message("WARNING: You need to implement DEPRECATED for your compiler!")
#define DEPRECATED(func) func
#endif

/**
 * Mark templates as exported to generate public RTTI symbols which are
 * needed for GCC and Clang to support e.g. dynamic_cast between DSOs.
 */
#if defined(__clang__) || defined(__GNUC__)
#define MITK_EXPORT __attribute__((visibility("default")))
#define MITK_IMPORT __attribute__((visibility("default")))
#define MITK_LOCAL __attribute__((visibility("hidden")))
#elif defined(WIN32)
#define MITK_EXPORT __declspec(dllexport)
#define MITK_IMPORT __declspec(dllimport)
#define MITK_LOCAL
#else
#define MITK_EXPORT
#define MITK_IMPORT
#define MITK_LOCAL
#endif

#endif // MITK_COMMON_H_DEFINED
