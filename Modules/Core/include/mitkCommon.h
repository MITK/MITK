/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCommon_h
#define mitkCommon_h

// add only those headers here that are really necessary for all classes!
#include <itkObject.h>
#include <mitkConfig.h>
#include <mitkExceptionMacro.h>
#include <mitkGetClassHierarchy.h>
#include <mitkLog.h>

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
  itkTypeMacro(className, SuperClassName);

#define mitkClassMacroItkParent(className, SuperClassName)                                                             \
  typedef className Self;                                                                                              \
  typedef SuperClassName Superclass;                                                                                   \
  typedef itk::SmartPointer<Self> Pointer;                                                                             \
  typedef itk::SmartPointer<const Self> ConstPointer;                                                                  \
  static const char *GetStaticNameOfClass() { return #className; }                                                     \
  virtual std::vector<std::string> GetClassHierarchy() const { return mitk::GetClassHierarchy<Self>(); }               \
  itkTypeMacro(className, SuperClassName);

/** At version 4.7 provides two type macros, the normal one expects the Superclass to provide the
 *  GetNameOfClass explicitly, the NoParent deos not expect anything.
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

/** Cross-platform scoped suppression of deprecation warnings.
 *  Use when a deprecated API must still be called intentionally (e.g. a
 *  deprecated wrapper forwarding to its replacement, or a known-buggy
 *  third-party filter kept until migration is possible).
 */
#if defined(__clang__)
#define MITK_IGNORE_DEPRECATED_WARNING_BEGIN                                                                           \
  _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wdeprecated-declarations\"")
#define MITK_IGNORE_DEPRECATED_WARNING_END _Pragma("clang diagnostic pop")
#elif defined(__GNUC__)
#define MITK_IGNORE_DEPRECATED_WARNING_BEGIN                                                                           \
  _Pragma("GCC diagnostic push") _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#define MITK_IGNORE_DEPRECATED_WARNING_END _Pragma("GCC diagnostic pop")
#elif defined(_MSC_VER)
#define MITK_IGNORE_DEPRECATED_WARNING_BEGIN __pragma(warning(push)) __pragma(warning(disable : 4996))
#define MITK_IGNORE_DEPRECATED_WARNING_END __pragma(warning(pop))
#else
#define MITK_IGNORE_DEPRECATED_WARNING_BEGIN
#define MITK_IGNORE_DEPRECATED_WARNING_END
#endif

/** Override-aware counterparts of the ITK property macros.
 *
 *  Use these in subclasses that implement a string / const-object
 *  property whose signature is already declared as a virtual method by
 *  a base class (e.g. mitk::FileReader::GetFileName() = 0). ITK's own
 *  itkSet/GetStringMacro and itkSet/GetConstObjectMacro expand to
 *  virtual methods without an explicit override keyword, which Apple
 *  Clang flags under -Winconsistent-missing-override. Rather than
 *  suppress the warning at every call site, these macros reimplement
 *  the ITK expansions so the override keyword is part of the method
 *  declaration.
 */
#define mitkOverrideSetStringMacro(name)                                                                               \
  void Set##name(const char *_arg) override                                                                            \
  {                                                                                                                    \
    if (_arg && (_arg == this->m_##name))                                                                              \
    {                                                                                                                  \
      return;                                                                                                          \
    }                                                                                                                  \
    if (_arg)                                                                                                          \
    {                                                                                                                  \
      this->m_##name = _arg;                                                                                           \
    }                                                                                                                  \
    else                                                                                                               \
    {                                                                                                                  \
      this->m_##name = "";                                                                                             \
    }                                                                                                                  \
    this->Modified();                                                                                                  \
  }                                                                                                                    \
  void Set##name(const std::string &_arg) { this->Set##name(_arg.c_str()); }                                           \
  ITK_MACROEND_NOOP_STATEMENT

#define mitkOverrideGetStringMacro(name)                                                                               \
  const char *Get##name() const override { return this->m_##name.c_str(); }                                            \
  ITK_MACROEND_NOOP_STATEMENT

#define mitkOverrideSetConstObjectMacro(name, type)                                                                    \
  void Set##name(const type *_arg) override                                                                            \
  {                                                                                                                    \
    itkDebugMacro("setting " << #name " to " << _arg);                                                                 \
    if (this->m_##name != _arg)                                                                                        \
    {                                                                                                                  \
      this->m_##name = _arg;                                                                                           \
      this->Modified();                                                                                                \
    }                                                                                                                  \
  }                                                                                                                    \
  ITK_MACROEND_NOOP_STATEMENT

#define mitkOverrideGetConstObjectMacro(name, type)                                                                    \
  const type *Get##name() const override { return this->m_##name.GetPointer(); }                                       \
  ITK_MACROEND_NOOP_STATEMENT

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

#endif
