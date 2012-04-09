#ifndef USERVICESBASEOBJECT_H
#define USERVICESBASEOBJECT_H

#include <itkLightObject.h>

#include <mitkServiceInterface.h>

template<> inline const char* us_service_impl_name(itk::LightObject* impl)
{ return impl->GetNameOfClass(); }

#endif // USERVICESBASEOBJECT_H
