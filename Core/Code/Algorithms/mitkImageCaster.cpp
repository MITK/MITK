/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-06-18 20:12:43 +0200 (Fr, 18 Jun 2010) $
Version:   $Revision: 23881 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkImageCaster.h"
#include "mitkImageAccessByItk.h"

// shorts
void mitk::ImageCaster::CastToItkImage(const mitk::Image * mitkImage, itk::SmartPointer<itk::Image<signed short, 3> > & itkOutputImage){
    mitk::CastToItkImage<itk::Image<signed short, 3> >(mitkImage, itkOutputImage);
}

void mitk::ImageCaster::CastToMitkImage(const itk::Image<signed short, 3>* itkimage, itk::SmartPointer<mitk::Image>& mitkoutputimage){
    mitk::CastToMitkImage<itk::Image<signed short, 3> >(itkimage, mitkoutputimage);
}

void mitk::ImageCaster::CastToItkImage(const mitk::Image * mitkImage, itk::SmartPointer<itk::Image<unsigned short, 3> > & itkOutputImage){
    mitk::CastToItkImage<itk::Image<unsigned short, 3> >(mitkImage, itkOutputImage);
}

void mitk::ImageCaster::CastToMitkImage(const itk::Image<unsigned short, 3>* itkimage, itk::SmartPointer<mitk::Image>& mitkoutputimage){
    mitk::CastToMitkImage<itk::Image<unsigned short, 3> >(itkimage, mitkoutputimage);
}

// char
void mitk::ImageCaster::CastToItkImage(const mitk::Image * mitkImage, itk::SmartPointer<itk::Image<char, 3> > & itkOutputImage){
    mitk::CastToItkImage<itk::Image<char, 3> >(mitkImage, itkOutputImage);
}

void mitk::ImageCaster::CastToMitkImage(const itk::Image<char, 3>* itkimage, itk::SmartPointer<mitk::Image>& mitkoutputimage){
    mitk::CastToMitkImage<itk::Image<char, 3> >(itkimage, mitkoutputimage);
}

void mitk::ImageCaster::CastToItkImage(const mitk::Image * mitkImage, itk::SmartPointer<itk::Image<unsigned char, 3> > & itkOutputImage){
    mitk::CastToItkImage<itk::Image<unsigned char, 3> >(mitkImage, itkOutputImage);
}

void mitk::ImageCaster::CastToMitkImage(const itk::Image<unsigned char, 3>* itkimage, itk::SmartPointer<mitk::Image>& mitkoutputimage){
    mitk::CastToMitkImage<itk::Image<unsigned char, 3> >(itkimage, mitkoutputimage);
}

// int
void mitk::ImageCaster::CastToItkImage(const mitk::Image * mitkImage, itk::SmartPointer<itk::Image<signed int, 3> > & itkOutputImage){
    mitk::CastToItkImage<itk::Image<signed int, 3> >(mitkImage, itkOutputImage);
}

void mitk::ImageCaster::CastToMitkImage(const itk::Image<signed int, 3>* itkimage, itk::SmartPointer<mitk::Image>& mitkoutputimage){
    mitk::CastToMitkImage<itk::Image<signed int, 3> >(itkimage, mitkoutputimage);
}

void mitk::ImageCaster::CastToItkImage(const mitk::Image * mitkImage, itk::SmartPointer<itk::Image<unsigned int, 3> > & itkOutputImage){
    mitk::CastToItkImage<itk::Image<unsigned int, 3> >(mitkImage, itkOutputImage);
}

void mitk::ImageCaster::CastToMitkImage(const itk::Image<unsigned int, 3>* itkimage, itk::SmartPointer<mitk::Image>& mitkoutputimage){
    mitk::CastToMitkImage<itk::Image<unsigned int, 3> >(itkimage, mitkoutputimage);
}

// float
void mitk::ImageCaster::CastToItkImage(const mitk::Image * mitkImage, itk::SmartPointer<itk::Image<float, 3> > & itkOutputImage){
    mitk::CastToItkImage<itk::Image<float, 3> >(mitkImage, itkOutputImage);
}

void mitk::ImageCaster::CastToMitkImage(const itk::Image<float, 3>* itkimage, itk::SmartPointer<mitk::Image>& mitkoutputimage){
    mitk::CastToMitkImage<itk::Image<float, 3> >(itkimage, mitkoutputimage);
}

// int
void mitk::ImageCaster::CastToItkImage(const mitk::Image * mitkImage, itk::SmartPointer<itk::Image<double, 3> > & itkOutputImage){
    mitk::CastToItkImage<itk::Image<double, 3> >(mitkImage, itkOutputImage);
}

void mitk::ImageCaster::CastToMitkImage(const itk::Image<double, 3>* itkimage, itk::SmartPointer<mitk::Image>& mitkoutputimage){
    mitk::CastToMitkImage<itk::Image<double, 3> >(itkimage, mitkoutputimage);
}

void mitk::ImageCaster::CastBaseData(mitk::BaseData* const mitkBaseData, itk::SmartPointer<mitk::Image>& mitkoutputimage){
    try
    {
        mitkoutputimage = dynamic_cast<mitk::Image*>(mitkBaseData);
    }
    catch(...)
    {
        return;
    }
}
