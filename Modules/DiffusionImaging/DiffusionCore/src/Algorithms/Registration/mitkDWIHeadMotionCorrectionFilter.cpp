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


#ifndef MITKDWIHEADMOTIONCORRECTIONFILTER_CPP
#define MITKDWIHEADMOTIONCORRECTIONFILTER_CPP

#include "mitkDWIHeadMotionCorrectionFilter.h"

#include "itkSplitDWImageFilter.h"
#include "itkB0ImageExtractionToSeparateImageFilter.h"

#include "mitkImageTimeSelector.h"

#include <itkExtractDwiChannelFilter.h>
#include <mitkMultiModalAffineDefaultRegistrationAlgorithm.h>
#include <mitkAlgorithmHelper.h>
#include <mitkMaskedAlgorithmHelper.h>
#include <mitkImageMappingHelper.h>
#include <mitkRegistrationHelper.h>
#include <mitkRegistrationWrapper.h>
#include <mitkRegistrationWrapperMapper3D.h>

#include <itkComposeImageFilter.h>
#include <mitkDiffusionPropertyHelper.h>
#include <mitkImageCast.h>
#include <mitkDiffusionImageCorrectionFilter.h>
#include <mitkImageWriteAccessor.h>
#include <mitkProperties.h>
#include <mitkBValueMapProperty.h>

#include <vector>

#include "mitkIOUtil.h"
#include <itkImage.h>

typedef mitk::DiffusionPropertyHelper DPH;
typedef itk::ExtractDwiChannelFilter< short > ExtractorType;

mitk::DWIHeadMotionCorrectionFilter::DWIHeadMotionCorrectionFilter()
{

}

mitk::Image::Pointer mitk::DWIHeadMotionCorrectionFilter::GetCorrectedImage() const
{
  return m_CorrectedImage;
}

void mitk::DWIHeadMotionCorrectionFilter::UpdateOutputInformation()
  {
  Superclass::UpdateOutputInformation();
        }

void mitk::DWIHeadMotionCorrectionFilter::GenerateData()
  {
  InputImageType::Pointer input = const_cast<InputImageType*>(this->GetInput(0));

  if (!DPH::IsDiffusionWeightedImage(input))
    mitkThrow() << "Input is not a diffusion-weighted image!";

  ITKDiffusionImageType::Pointer itkVectorImagePointer = DPH::GetItkVectorImage(input);
  int num_gradients = itkVectorImagePointer->GetVectorLength();

  typedef itk::ComposeImageFilter < ITKDiffusionVolumeType > ComposeFilterType;
  ComposeFilterType::Pointer composer = ComposeFilterType::New();

  // Extract unweighted volumes
  mitk::BValueMapProperty::BValueMap bval_map = DPH::GetBValueMap(input);
  int first_unweighted_index = bval_map.at(0).front();

  ExtractorType::Pointer filter = ExtractorType::New();
  filter->SetInput( itkVectorImagePointer);
  filter->SetChannelIndex(first_unweighted_index);
  filter->Update();

  mitk::Image::Pointer fixedImage = mitk::Image::New();
  fixedImage->InitializeByItk( filter->GetOutput() );
  fixedImage->SetImportChannel( filter->GetOutput()->GetBufferPointer() );
  composer->SetInput(0, filter->GetOutput());

  mitk::MultiModalAffineDefaultRegistrationAlgorithm< ITKDiffusionVolumeType >::Pointer algo = mitk::MultiModalAffineDefaultRegistrationAlgorithm< ITKDiffusionVolumeType >::New();
  mitk::MITKAlgorithmHelper helper(algo);

  typedef vnl_matrix_fixed< double, 3, 3> TransformMatrixType;
  std::vector< TransformMatrixType > estimated_transforms;
  std::vector< ITKDiffusionVolumeType::Pointer > registered_itk_images;
  for (int i=0; i<num_gradients; ++i)
  {
    if (i==first_unweighted_index)
      continue;

    MITK_INFO << "Correcting volume " << i;

    ExtractorType::Pointer filter = ExtractorType::New();
    filter->SetInput( itkVectorImagePointer);
    filter->SetChannelIndex(i);
    filter->Update();

    mitk::Image::Pointer movingImage = mitk::Image::New();
    movingImage->InitializeByItk( filter->GetOutput() );
    movingImage->SetImportChannel( filter->GetOutput()->GetBufferPointer() );

    helper.SetData(movingImage, fixedImage);
    mitk::MAPRegistrationWrapper::Pointer reg = helper.GetMITKRegistrationWrapper();
    mitk::MITKRegistrationHelper::Affine3DTransformType::Pointer affine = mitk::MITKRegistrationHelper::getAffineMatrix(reg, false);
    estimated_transforms.push_back(affine->GetMatrix().GetVnlMatrix());

    mitk::Image::Pointer registered_mitk_image = mitk::ImageMappingHelper::map(movingImage, reg, false, 0, nullptr, false, 0, mitk::ImageMappingInterpolator::BSpline_3);
    ITKDiffusionVolumeType::Pointer registered_itk_image = ITKDiffusionVolumeType::New();
    mitk::CastToItkImage(registered_mitk_image, registered_itk_image);
    registered_itk_images.push_back(registered_itk_image);
    }

  int i=1;
  for (auto image : registered_itk_images)
  {
    composer->SetInput(i, image);
    ++i;
  }
  composer->Update();

  m_CorrectedImage = mitk::GrabItkImageMemory( composer->GetOutput() );
  DPH::CopyProperties(input, m_CorrectedImage, true);

  typedef mitk::DiffusionImageCorrectionFilter CorrectionFilterType;
  CorrectionFilterType::Pointer corrector = CorrectionFilterType::New();
  corrector->SetImage( m_CorrectedImage );
  corrector->CorrectDirections( estimated_transforms );

  DPH propertyHelper( m_CorrectedImage );
  propertyHelper.InitializeImage();
}

#endif // MITKDWIHEADMOTIONCORRECTIONFILTER_CPP
