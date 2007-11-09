#define MITK_TOOL_FACTORY_MACRO(CLASS_NAME, DESCRIPTION) \
 \
class CLASS_NAME ## Factory : public ::itk::ObjectFactoryBase \
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

#define MITK_TOOL_GUI_FACTORY_MACRO(CLASS_NAME, DESCRIPTION) \
 \
class CLASS_NAME ## Factory : public ::itk::ObjectFactoryBase \
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



