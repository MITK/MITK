#define MITK_WRAP_OBJECT(name) \
typedef mitk::name::name name; \
typedef mitk::name::Pointer::SmartPointer name##_Pointer;

#define MITK_WRAP_OBJECT_WITH_SUPERCLASS(name) \
MITK_WRAP_OBJECT(name); \
typedef mitk::name::Superclass::Self name##_Superclass; \
typedef mitk::name::Superclass::Pointer::SmartPointer name##_Superclass_Pointer;

#define MITK_WRAP_OBJECT1(name, wrapname) \
typedef mitk::name::name wrapname; \
typedef mitk::name::Pointer::SmartPointer wrapname##_Pointer;

#define MITK_WRAP_OBJECT_WITH_SUPERCLASS1(name, wrapname) \
MITK_WRAP_OBJECT1(name, wrapname); \
typedef mitk::name::Superclass::Self wrapname##_Superclass; \
typedef mitk::name::Superclass::Pointer::SmartPointer wrapname##_Superclass_Pointer;


#define ITK_WRAP_OBJECT(name) \
typedef itk::name::name name; \
typedef itk::name::Pointer::SmartPointer name##_Pointer;

#define ITK_WRAP_OBJECT1(name, arg1, wrapname) \
typedef itk::name<arg1 >::name wrapname; \
typedef itk::name<arg1 >::Pointer::SmartPointer wrapname##_Pointer
