#include "itkImage.h"
#include "mitkImageToItk.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="ImageToItk";
     namespace wrappers
     {
         typedef mitk::ImageToItk<itk::Image<double, 2>::Image> ImageToItkD2;
         typedef mitk::ImageToItk<itk::Image<double, 2>::Image>::Pointer::SmartPointer ImageToItkD2_Pointer;
         typedef mitk::ImageToItk<itk::Image<double, 3>::Image> ImageToItkD3;
         typedef mitk::ImageToItk<itk::Image<double, 3>::Image>::Pointer::SmartPointer ImageToItkD3_Pointer;
         typedef mitk::ImageToItk<itk::Image<float, 2>::Image> ImageToItkF2;
         typedef mitk::ImageToItk<itk::Image<float, 2>::Image>::Pointer::SmartPointer ImageToItkF2_Pointer;
         typedef mitk::ImageToItk<itk::Image<float, 3>::Image> ImageToItkF3;
         typedef mitk::ImageToItk<itk::Image<float, 3>::Image>::Pointer::SmartPointer ImageToItkF3_Pointer;
         typedef mitk::ImageToItk<itk::Image<unsigned short, 2>::Image> ImageToItkUS2;
         typedef mitk::ImageToItk<itk::Image<unsigned short, 2>::Image>::Pointer::SmartPointer ImageToItkUS2_Pointer;
         typedef mitk::ImageToItk<itk::Image<unsigned short, 3>::Image> ImageToItkUS3;
         typedef mitk::ImageToItk<itk::Image<unsigned short, 3>::Image>::Pointer::SmartPointer ImageToItkUS3_Pointer;
         typedef mitk::ImageToItk<itk::Image<unsigned char, 2>::Image> ImageToItkUC2;
         typedef mitk::ImageToItk<itk::Image<unsigned char, 2>::Image>::Pointer::SmartPointer ImageToItkUC2_Pointer;
         typedef mitk::ImageToItk<itk::Image<unsigned char, 3>::Image> ImageToItkUC3;
         typedef mitk::ImageToItk<itk::Image<unsigned char, 3>::Image>::Pointer::SmartPointer ImageToItkUC3_Pointer;
         typedef mitk::ImageToItk<itk::Image<unsigned int, 2>::Image> ImageToItkUI2;
         typedef mitk::ImageToItk<itk::Image<unsigned int, 2>::Image>::Pointer::SmartPointer ImageToItkUI2_Pointer;
         typedef mitk::ImageToItk<itk::Image<unsigned int, 3>::Image> ImageToItkUI3;
         typedef mitk::ImageToItk<itk::Image<unsigned int, 3>::Image>::Pointer::SmartPointer ImageToItkUI3_Pointer;
         typedef mitk::ImageToItk<itk::Image<unsigned long, 2>::Image> ImageToItkUL2;
         typedef mitk::ImageToItk<itk::Image<unsigned long, 2>::Image>::Pointer::SmartPointer ImageToItkUL2_Pointer;
         typedef mitk::ImageToItk<itk::Image<unsigned long, 3>::Image> ImageToItkUL3;
         typedef mitk::ImageToItk<itk::Image<unsigned long, 3>::Image>::Pointer::SmartPointer ImageToItkUL3_Pointer;
         typedef mitk::ImageToItk<itk::Image<signed char, 2>::Image> ImageToItkSC2;
         typedef mitk::ImageToItk<itk::Image<signed char, 2>::Image>::Pointer::SmartPointer ImageToItkSC2_Pointer;
         typedef mitk::ImageToItk<itk::Image<signed char, 3>::Image> ImageToItkSC3;
         typedef mitk::ImageToItk<itk::Image<signed char, 3>::Image>::Pointer::SmartPointer ImageToItkSC3_Pointer;
         typedef mitk::ImageToItk<itk::Image<signed int, 2>::Image> ImageToItkSI2;
         typedef mitk::ImageToItk<itk::Image<signed int, 2>::Image>::Pointer::SmartPointer ImageToItkSI2_Pointer;
         typedef mitk::ImageToItk<itk::Image<signed int, 3>::Image> ImageToItkSI3;
         typedef mitk::ImageToItk<itk::Image<signed int, 3>::Image>::Pointer::SmartPointer ImageToItkSI3_Pointer;
         typedef mitk::ImageToItk<itk::Image<signed short, 2>::Image> ImageToItkSS2;
         typedef mitk::ImageToItk<itk::Image<signed short, 2>::Image>::Pointer::SmartPointer ImageToItkSS2_Pointer;
         typedef mitk::ImageToItk<itk::Image<signed short, 3>::Image> ImageToItkSS3;
         typedef mitk::ImageToItk<itk::Image<signed short, 3>::Image>::Pointer::SmartPointer ImageToItkSS3_Pointer;
     }
}

#endif
