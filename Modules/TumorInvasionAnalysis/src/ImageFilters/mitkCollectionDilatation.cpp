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
#include "mitkCollectionDilatation.h"


#include <mitkImageCast.h>
#include <mitkITKImageImport.h>

#include "itkGrayscaleDilateImageFilter.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryErodeImageFilter.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkFlatStructuringElement.h"


void mitk::CollectionDilation::DilateBinaryByName(mitk::DataCollection *dataCollection, std::string name, unsigned int xy, unsigned int z , std::string suffix)
{
  for (size_t patient = 0; patient < dataCollection->Size(); ++patient)
  {
    DataCollection* dataPatient = dynamic_cast<DataCollection*>(dataCollection->GetData(patient).GetPointer());
    if (dataPatient == NULL)
      MITK_ERROR << "DilateBinaryByName - Structure of DataCollection is invalid at patient level. Data inconsistent!";

    if (dataPatient->Size() == 0)
      MITK_ERROR << "Empty Patient Collective. Probably Fatal.";


    for (size_t timeStep = 0; timeStep < dataPatient->Size(); ++timeStep)
    {
      DataCollection* dataTimeStep = dynamic_cast<DataCollection*>(dataPatient->GetData(timeStep).GetPointer());
      if (dataTimeStep == NULL)
        MITK_ERROR << "DilateBinaryByName- Structure of DataCollection is invalid at time step level. Data inconsistent!";


      BinaryImage::Pointer itkTumorSeed = BinaryImage::New();
      Image* tumorSeed = dynamic_cast<Image*> (dataTimeStep->GetMitkImage(name).GetPointer());
      if (tumorSeed == NULL)
        MITK_ERROR << "Image " << name << " does not exits. Fatal.";

      CastToItkImage(tumorSeed,itkTumorSeed);

      typedef itk::FlatStructuringElement<3> StructuringElementType;
      StructuringElementType::RadiusType elementRadius;
      elementRadius.Fill(xy);
      elementRadius[2] = z;


      StructuringElementType structuringElement = StructuringElementType::Box(elementRadius);
      typedef itk::BinaryDilateImageFilter <BinaryImage, BinaryImage, StructuringElementType> BinaryDilateImageFilterType;

      BinaryDilateImageFilterType::Pointer dilateFilter0 = BinaryDilateImageFilterType::New();
      dilateFilter0->SetInput(itkTumorSeed);
      dilateFilter0->SetKernel(structuringElement);
      dilateFilter0->SetForegroundValue(1);
      dilateFilter0->Update();

      Image::Pointer dil =  GrabItkImageMemory ( dilateFilter0->GetOutput() );

      dil->SetGeometry(tumorSeed->GetGeometry());
      dataTimeStep->AddData(dil.GetPointer(), name+suffix, "Dilated Binary");
    }
  }
}

void mitk::CollectionDilation::ErodeBinaryByName(mitk::DataCollection *dataCollection, std::string name, unsigned int xy, unsigned int z, std::string suffix)
{
  for (size_t patient = 0; patient < dataCollection->Size(); ++patient)
  {
    DataCollection* dataPatient = dynamic_cast<DataCollection*>(dataCollection->GetData(patient).GetPointer());
    if (dataPatient == NULL)
      MITK_ERROR << "DilateBinaryByName - Structure of DataCollection is invalid at patient level. Data inconsistent!";

    if (dataPatient->Size() == 0)
      MITK_ERROR << "Empty Patient Collective. Probably Fatal.";


    for (size_t timeStep = 0; timeStep < dataPatient->Size(); ++timeStep)
    {
      DataCollection* dataTimeStep = dynamic_cast<DataCollection*>(dataPatient->GetData(timeStep).GetPointer());
      if (dataTimeStep == NULL)
        MITK_ERROR << "DilateBinaryByName- Structure of DataCollection is invalid at time step level. Data inconsistent!";


      BinaryImage::Pointer itkTumorSeed = BinaryImage::New();
      Image* tumorSeed = dynamic_cast<Image*> (dataTimeStep->GetMitkImage(name).GetPointer());
      if (tumorSeed == NULL)
        MITK_ERROR << "Image " << name << " does not exits. Fatal.";

      CastToItkImage(tumorSeed,itkTumorSeed);

      typedef itk::FlatStructuringElement<3> StructuringElementType;
      StructuringElementType::RadiusType elementRadius;
      elementRadius.Fill(xy);
      elementRadius[2] = z;


      StructuringElementType structuringElement = StructuringElementType::Box(elementRadius);


      typedef itk::BinaryErodeImageFilter <BinaryImage, BinaryImage, StructuringElementType> BinaryErodeImageFilterType;

      BinaryErodeImageFilterType::Pointer dilateFilter0 = BinaryErodeImageFilterType::New();
      dilateFilter0->SetInput(itkTumorSeed);
      dilateFilter0->SetKernel(structuringElement);
      dilateFilter0->SetForegroundValue(1);
      dilateFilter0->Update();

      Image::Pointer dil =  GrabItkImageMemory ( dilateFilter0->GetOutput() );

      dil->SetGeometry(tumorSeed->GetGeometry());
      dataTimeStep->AddData(dil.GetPointer(), name+suffix, "Dilated Binary");

    }
  }
}
