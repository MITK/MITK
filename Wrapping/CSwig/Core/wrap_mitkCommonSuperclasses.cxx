#include "mitkCommon.h"
#include "mitkBaseProcess.h"
#include "mitkCSwigMacros.h"
#include "mbilog.h"
#include "itkObject.h"
#include "itkDataObject.h"
#include "itkProcessObject.h"
#include "itkLightObject.h"
#include "itkObjectFactoryBase.h"
#pragma GCC visibility push(default)
#include <itkEventObject.h>
#pragma GCC visibility pop
#include "itkSmartPointerForwardReference.h"
#include "itkAffineTransform.h"
#include "itkMatrixOffsetTransformBase.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="CommonSuperclasses";
     namespace wrappers
     {
         ITK_WRAP_OBJECT(Object)
         ITK_WRAP_OBJECT(DataObject)
         ITK_WRAP_OBJECT(ProcessObject)
         ITK_WRAP_OBJECT(LightObject)
         ITK_WRAP_OBJECT(ObjectFactoryBase)
         typedef itk::EndEvent::EndEvent EndEvent;
         //typedef mbilog::AbstractBackend::AbstractBackend AbstractBackend;
         typedef itk::AffineTransform<float, 3 >::AffineTransform AffineTransform_F3U;
         typedef itk::AffineTransform<float, 3 >::Pointer::SmartPointer AffineTransform_F3U_Pointer;
         typedef itk::MatrixOffsetTransformBase<float, 3, 3 >::MatrixOffsetTransformBase MatrixOffsetTransformBase_F3U3U;
         typedef itk::MatrixOffsetTransformBase<float, 3, 3 >::Pointer::SmartPointer MatrixOffsetTransformBase_F3U3U_Pointer;
     }
}

#endif

