/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/


#define MITK_TOOL_FACTORY_MACRO(EXPORT_SPEC, CLASS_NAME, DESCRIPTION) \
 \
class EXPORT_SPEC CLASS_NAME ## Factory : public ::itk::ObjectFactoryBase \
{ \
  public:   \
 \
    /* ITK typedefs */ \
    typedef CLASS_NAME ## Factory   Self; \
    typedef itk::ObjectFactoryBase  Superclass; \
    typedef itk::SmartPointer<Self>  Pointer; \
    typedef itk::SmartPointer<const Self>  ConstPointer; \
      \
    /* Methods from ObjectFactoryBase */ \
    virtual const char* GetITKSourceVersion() const \
    { \
      return ITK_SOURCE_VERSION; \
    }\
     \
    virtual const char* GetDescription() const \
    { \
      return DESCRIPTION; \
    }\
     \
    /* Method for class instantiation. */ \
    itkFactorylessNewMacro(Self); \
       \
    /* Run-time type information (and related methods). */ \
    itkTypeMacro(CLASS_NAME ## Factory, itkObjectFactoryBase); \
 \
    /* Register one factory of this type  */ \
    static void RegisterOneFactory() \
    { \
      CLASS_NAME ## Factory::Pointer CLASS_NAME ## Factory = CLASS_NAME ## Factory::New(); \
      itk::ObjectFactoryBase::RegisterFactory(CLASS_NAME ## Factory); \
    } \
     \
  protected: \
 \
    CLASS_NAME ## Factory() \
    { \
      itk::ObjectFactoryBase::RegisterOverride("mitkTool", \
                              #CLASS_NAME, \
                              DESCRIPTION, \
                              1, \
                              itk::CreateObjectFunction<CLASS_NAME>::New()); \
    } \
 \
    ~CLASS_NAME ## Factory() \
    { \
    } \
 \
  private: \
 \
    CLASS_NAME ## Factory(const Self&); /* purposely not implemented */ \
    void operator=(const Self&);    /* purposely not implemented */ \
 \
};

/* GUI classes are _not_ exported! */

#define MITK_TOOL_GUI_FACTORY_MACRO(EXPORT_SPEC, CLASS_NAME, DESCRIPTION) \
 \
class EXPORT_SPEC CLASS_NAME ## Factory : public ::itk::ObjectFactoryBase \
{ \
  public:   \
 \
    /* ITK typedefs */ \
    typedef CLASS_NAME ## Factory   Self; \
    typedef itk::ObjectFactoryBase  Superclass; \
    typedef itk::SmartPointer<Self>  Pointer; \
    typedef itk::SmartPointer<const Self>  ConstPointer; \
      \
    /* Methods from ObjectFactoryBase */ \
    virtual const char* GetITKSourceVersion() const \
    { \
      return ITK_SOURCE_VERSION; \
    }\
     \
    virtual const char* GetDescription() const \
    { \
      return DESCRIPTION; \
    }\
     \
    /* Method for class instantiation. */ \
    itkFactorylessNewMacro(Self); \
       \
    /* Run-time type information (and related methods). */ \
    itkTypeMacro(CLASS_NAME ## Factory, itkObjectFactoryBase); \
 \
    /* Register one factory of this type  */ \
    static void RegisterOneFactory() \
    { \
      CLASS_NAME ## Factory::Pointer CLASS_NAME ## Factory = CLASS_NAME ## Factory::New(); \
      itk::ObjectFactoryBase::RegisterFactory(CLASS_NAME ## Factory); \
    } \
     \
  protected: \
 \
    CLASS_NAME ## Factory() \
    { \
      itk::ObjectFactoryBase::RegisterOverride(#CLASS_NAME, \
                              #CLASS_NAME, \
                              DESCRIPTION, \
                              1, \
                              itk::CreateObjectFunction<CLASS_NAME>::New()); \
    } \
 \
    ~CLASS_NAME ## Factory() \
    { \
    } \
 \
  private: \
 \
    CLASS_NAME ## Factory(const Self&); /* purposely not implemented */ \
    void operator=(const Self&);    /* purposely not implemented */ \
 \
};




