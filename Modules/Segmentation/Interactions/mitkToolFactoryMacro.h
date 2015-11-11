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


#define MITK_TOOL_MACRO(EXPORT_SPEC, CLASS_NAME, DESCRIPTION) \
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
}; \
   \
class CLASS_NAME ## RegistrationMethod \
  { \
    public: \
 \
    CLASS_NAME ## RegistrationMethod() \
    { \
      /*MITK_INFO("tools") << "Registered " #CLASS_NAME; */ \
      m_Factory = CLASS_NAME ## Factory::New(); \
      itk::ObjectFactoryBase::RegisterFactory( m_Factory ); \
    } \
 \
    ~CLASS_NAME ## RegistrationMethod() \
    { \
      /*MITK_INFO("tools") << "UnRegistered " #CLASS_NAME; */ \
      itk::ObjectFactoryBase::UnRegisterFactory( m_Factory ); \
    } \
 \
    private: \
 \
    CLASS_NAME ## Factory::Pointer m_Factory; \
  }; \
 \
static mitk::CLASS_NAME ## RegistrationMethod somestaticinitializer_ ## CLASS_NAME ;




#define MITK_DERIVED_SM_TOOL_MACRO(EXPORT_SPEC, BASE_CLASS, CLASS_NAME, DESCRIPTION) \
 \
class EXPORT_SPEC CLASS_NAME ## Tool : public BASE_CLASS \
{ \
  public:   \
 \
    typedef CLASS_NAME ## Tool  Self; \
    typedef BASE_CLASS  Superclass; \
    typedef itk::SmartPointer<Self>  Pointer; \
    typedef itk::SmartPointer<const Self>  ConstPointer; \
 \
    itkFactorylessNewMacro(Self) \
    itkCloneMacro(Self) \
 \
  protected: \
 \
    CLASS_NAME ## Tool() \
    { \
      m_SegmentationGenerator = CLASS_NAME::New(); \
    } \
 \
    void RegisterProgressObserver() \
    { \
      itk::ReceptorMemberCommand< CLASS_NAME ## Tool >::Pointer command = itk::ReceptorMemberCommand< CLASS_NAME ## Tool >::New(); \
      command->SetCallbackFunction(this, &CLASS_NAME ## Tool::OnProgressEvent); \
      m_SegmentationGenerator->AddSegmentationProgressObserver< CLASS_NAME ## Tool >( command ); \
    } \
 \
    void RegisterFinishedSegmentationObserver() \
    { \
      itk::ReceptorMemberCommand< CLASS_NAME ## Tool >::Pointer command = itk::ReceptorMemberCommand< CLASS_NAME ## Tool >::New(); \
      command->SetCallbackFunction(this, &CLASS_NAME ## Tool::OnSegmentationFinished); \
      m_SegmentationGenerator->AddSegmentationFinishedObserver< CLASS_NAME ## Tool >( command ); \
    } \
 \
    ~CLASS_NAME ## Tool() \
    { \
    } \
}; \
MITK_TOOL_MACRO(EXPORT_SPEC, CLASS_NAME ## Tool, DESCRIPTION);

/* GUI classes are _not_ exported! */

#define MITK_TOOL_GUI_MACRO(EXPORT_SPEC, CLASS_NAME, DESCRIPTION) \
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
}; \
 \
class CLASS_NAME ## RegistrationMethod \
  { \
    public: \
 \
    CLASS_NAME ## RegistrationMethod() \
    { \
      /*MITK_INFO("tools") << "Registered " #CLASS_NAME; */ \
      m_Factory = CLASS_NAME ## Factory::New(); \
      itk::ObjectFactoryBase::RegisterFactory( m_Factory ); \
    } \
 \
    ~CLASS_NAME ## RegistrationMethod() \
     { \
       /*MITK_INFO("tools") << "UnRegistered " #CLASS_NAME; */ \
       itk::ObjectFactoryBase::UnRegisterFactory( m_Factory ); \
     } \
 \
     private: \
 \
     CLASS_NAME ## Factory::Pointer m_Factory; \
  }; \
 \
static CLASS_NAME ## RegistrationMethod somestaticinitializer_ ## CLASS_NAME ;


#define MITK_EXTERNAL_TOOL_GUI_HEADER_MACRO(EXPORT_SPEC, CLASS_NAME, DESCRIPTION) \
extern "C" { \
EXPORT_SPEC itk::ObjectFactoryBase* itkLoad(); \
}

#define MITK_EXTERNAL_TOOL_GUI_CPP_MACRO(EXPORT_SPEC, CLASS_NAME, DESCRIPTION) \
MITK_TOOL_GUI_MACRO(EXPORT_SPEC, CLASS_NAME, DESCRIPTION) \
extern "C" { \
EXPORT_SPEC itk::ObjectFactoryBase* itkLoad() { \
  static CLASS_NAME ## Factory::Pointer p = CLASS_NAME ## Factory::New(); \
  return p; \
} \
}



