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

#include "mitkCollectionGrayOpening.h"
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>

#include "itkGrayscaleMorphologicalOpeningImageFilter.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkFlatStructuringElement.h"




void mitk::CollectionGrayOpening::PerformGrayOpening(mitk::DataCollection *dataCollection, std::string name, std::string suffix)
{
  for (size_t patient = 0; patient < dataCollection->Size(); ++patient)
  {
    DataCollection* dataPatient = dynamic_cast<DataCollection*>(dataCollection->GetData(patient).GetPointer());
    if (dataPatient == NULL)
      MITK_ERROR << "PerformGrayOpening - Structure of DataCollection is invalid at patient level. Data inconsistent!";

    if (dataPatient->Size() == 0)
      MITK_ERROR << "Empty Patient Collective. Probably Fatal.";

    for (size_t timeStep = 0; timeStep < dataPatient->Size(); ++timeStep)
    {
      DataCollection* dataTimeStep = dynamic_cast<DataCollection*>(dataPatient->GetData(timeStep).GetPointer());
      if (dataTimeStep == NULL)
        MITK_ERROR << "DilateBinaryByName- Structure of DataCollection is invalid at time step level. Data inconsistent!";


      //BinaryImage::Pointer itkImage = BinaryImage::New();
      ImageType::Pointer itkImage =  ImageType::New();
      Image::Pointer tmp = dataTimeStep->GetMitkImage(name).GetPointer();
      if (tmp.IsNull())
        MITK_ERROR << "null";
      CastToItkImage(tmp,itkImage);
      if (itkImage.IsNull())
        MITK_ERROR << "Image " << name << " does not exist. Fatal.";

      typedef itk::FlatStructuringElement<3> StructuringElementType;
      StructuringElementType::RadiusType elementRadius;
      elementRadius.Fill(1);
      elementRadius[2] = 0;
      StructuringElementType structuringElement = StructuringElementType::Box(elementRadius);

      typedef itk::GrayscaleMorphologicalOpeningImageFilter <ImageType, ImageType, StructuringElementType> DilateImageFilterType;

      DilateImageFilterType::Pointer dilateFilter0 = DilateImageFilterType::New();
      dilateFilter0->SetInput(itkImage);
      dilateFilter0->SetKernel(structuringElement);
      dilateFilter0->Update();

      DilateImageFilterType::Pointer dilateFilter1 = DilateImageFilterType::New();
      dilateFilter1->SetInput(dilateFilter0->GetOutput());
      dilateFilter1->SetKernel(structuringElement);
      dilateFilter1->Update();

      Image::Pointer dil =  GrabItkImageMemory ( dilateFilter1->GetOutput() );
      dataTimeStep->AddData(dil.GetPointer(), name+suffix, "");

    }
  }
}
