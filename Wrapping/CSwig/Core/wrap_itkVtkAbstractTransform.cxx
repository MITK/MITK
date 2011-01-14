#include "itkVtkAbstractTransform.h"
#include "itkImage.h"


#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="VtkAbstractTransform";
     namespace wrappers
     {
         typedef itk::VtkAbstractTransform<itk::Image<float, 2>::Image> VtkAbstractTransformF2;
         typedef itk::VtkAbstractTransform<itk::Image<float, 2>::Image>::Pointer::SmartPointer VtkAbstractTransformF2_Pointer;
         typedef itk::VtkAbstractTransform<itk::Image<unsigned char, 2>::Image> VtkAbstractTransformUC2;
         typedef itk::VtkAbstractTransform<itk::Image<unsigned char, 2>::Image>::Pointer::SmartPointer VtkAbstractTransformUC2_Pointer;
         typedef itk::VtkAbstractTransform<itk::Image<unsigned short, 2>::Image> VtkAbstractTransformUS2;
         typedef itk::VtkAbstractTransform<itk::Image<unsigned short, 2>::Image>::Pointer::SmartPointer VtkAbstractTransformUS2_Pointer;
         typedef itk::VtkAbstractTransform<itk::Image<unsigned long, 2>::Image> VtkAbstractTransformUL2;
         typedef itk::VtkAbstractTransform<itk::Image<unsigned long, 2>::Image>::Pointer::SmartPointer VtkAbstractTransformUL2_Pointer;

         typedef itk::VtkAbstractTransform<itk::Image<float, 3>::Image> VtkAbstractTransformF3;
         typedef itk::VtkAbstractTransform<itk::Image<float, 3>::Image>::Pointer::SmartPointer VtkAbstractTransformF3_Pointer;
         typedef itk::VtkAbstractTransform<itk::Image<unsigned char, 3>::Image> VtkAbstractTransformUC3;
         typedef itk::VtkAbstractTransform<itk::Image<unsigned char, 3>::Image>::Pointer::SmartPointer VtkAbstractTransformUC3_Pointer;
         typedef itk::VtkAbstractTransform<itk::Image<unsigned short, 3>::Image> VtkAbstractTransformUS3;
         typedef itk::VtkAbstractTransform<itk::Image<unsigned short, 3>::Image>::Pointer::SmartPointer VtkAbstractTransformUS3_Pointer;
         typedef itk::VtkAbstractTransform<itk::Image<unsigned long, 3>::Image> VtkAbstractTransformUL3;
         typedef itk::VtkAbstractTransform<itk::Image<unsigned long, 3>::Image>::Pointer::SmartPointer VtkAbstractTransformUL3_Pointer;
     }
}

#endif
