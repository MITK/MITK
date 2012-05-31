#include "itkImage.h"
#include "mitkITKImageImport.h"

#ifdef CABLE_CONFIGURATION

namespace _cable_
{
     const char* const group="ITKImageImport";
     namespace wrappers
     {
         typedef mitk::ITKImageImport<itk::Image<double, 2>::Image> ITKImageImportD2;
         typedef mitk::ITKImageImport<itk::Image<double, 2>::Image>::Pointer::SmartPointer ITKImageImportD2_Pointer;
         typedef mitk::ITKImageImport<itk::Image<double, 3>::Image> ITKImageImportD3;
         typedef mitk::ITKImageImport<itk::Image<double, 3>::Image>::Pointer::SmartPointer ITKImageImportD3_Pointer;
         typedef mitk::ITKImageImport<itk::Image<float, 2>::Image> ITKImageImportF2;
         typedef mitk::ITKImageImport<itk::Image<float, 2>::Image>::Pointer::SmartPointer ITKImageImportF2_Pointer;
         typedef mitk::ITKImageImport<itk::Image<float, 3>::Image> ITKImageImportF3;
         typedef mitk::ITKImageImport<itk::Image<float, 3>::Image>::Pointer::SmartPointer ITKImageImportF3_Pointer;
         typedef mitk::ITKImageImport<itk::Image<unsigned short, 2>::Image> ITKImageImportUS2;
         typedef mitk::ITKImageImport<itk::Image<unsigned short, 2>::Image>::Pointer::SmartPointer ITKImageImportUS2_Pointer;
         typedef mitk::ITKImageImport<itk::Image<unsigned short, 3>::Image> ITKImageImportUS3;
         typedef mitk::ITKImageImport<itk::Image<unsigned short, 3>::Image>::Pointer::SmartPointer ITKImageImportUS3_Pointer;
         typedef mitk::ITKImageImport<itk::Image<unsigned char, 2>::Image> ITKImageImportUC2;
         typedef mitk::ITKImageImport<itk::Image<unsigned char, 2>::Image>::Pointer::SmartPointer ITKImageImportUC2_Pointer;
         typedef mitk::ITKImageImport<itk::Image<unsigned char, 3>::Image> ITKImageImportUC3;
         typedef mitk::ITKImageImport<itk::Image<unsigned char, 3>::Image>::Pointer::SmartPointer ITKImageImportUC3_Pointer;
         typedef mitk::ITKImageImport<itk::Image<unsigned int, 2>::Image> ITKImageImportUI2;
         typedef mitk::ITKImageImport<itk::Image<unsigned int, 2>::Image>::Pointer::SmartPointer ITKImageImportUI2_Pointer;
         typedef mitk::ITKImageImport<itk::Image<unsigned int, 3>::Image> ITKImageImportUI3;
         typedef mitk::ITKImageImport<itk::Image<unsigned int, 3>::Image>::Pointer::SmartPointer ITKImageImportUI3_Pointer;
         typedef mitk::ITKImageImport<itk::Image<unsigned long, 2>::Image> ITKImageImportUL2;
         typedef mitk::ITKImageImport<itk::Image<unsigned long, 2>::Image>::Pointer::SmartPointer ITKImageImportUL2_Pointer;
         typedef mitk::ITKImageImport<itk::Image<unsigned long, 3>::Image> ITKImageImportUL3;
         typedef mitk::ITKImageImport<itk::Image<unsigned long, 3>::Image>::Pointer::SmartPointer ITKImageImportUL3_Pointer;
         typedef mitk::ITKImageImport<itk::Image<signed char, 2>::Image> ITKImageImportSC2;
         typedef mitk::ITKImageImport<itk::Image<signed char, 2>::Image>::Pointer::SmartPointer ITKImageImportSC2_Pointer;
         typedef mitk::ITKImageImport<itk::Image<signed char, 3>::Image> ITKImageImportSC3;
         typedef mitk::ITKImageImport<itk::Image<signed char, 3>::Image>::Pointer::SmartPointer ITKImageImportSC3_Pointer;
         typedef mitk::ITKImageImport<itk::Image<signed int, 2>::Image> ITKImageImportSI2;
         typedef mitk::ITKImageImport<itk::Image<signed int, 2>::Image>::Pointer::SmartPointer ITKImageImportSI2_Pointer;
         typedef mitk::ITKImageImport<itk::Image<signed int, 3>::Image> ITKImageImportSI3;
         typedef mitk::ITKImageImport<itk::Image<signed int, 3>::Image>::Pointer::SmartPointer ITKImageImportSI3_Pointer;
         typedef mitk::ITKImageImport<itk::Image<signed short, 2>::Image> ITKImageImportSS2;
         typedef mitk::ITKImageImport<itk::Image<signed short, 2>::Image>::Pointer::SmartPointer ITKImageImportSS2_Pointer;
         typedef mitk::ITKImageImport<itk::Image<signed short, 3>::Image> ITKImageImportSS3;
         typedef mitk::ITKImageImport<itk::Image<signed short, 3>::Image>::Pointer::SmartPointer ITKImageImportSS3_Pointer;
     }
}

#endif
