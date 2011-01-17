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



void mitk::ImageCaster::CastToItkImage(const mitk::Image * mitkImage, itk::SmartPointer<itk::Image<signed short, 3> > & itkOutputImage){
    mitk::CastToItkImage<itk::Image<signed short, 3> >(mitkImage, itkOutputImage);
}

void mitk::ImageCaster::CastToMitkImage(const itk::Image<signed short, 3>* itkimage, itk::SmartPointer<mitk::Image>& mitkoutputimage){
    mitk::CastToMitkImage<itk::Image<signed short, 3> >(itkimage, mitkoutputimage);
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
