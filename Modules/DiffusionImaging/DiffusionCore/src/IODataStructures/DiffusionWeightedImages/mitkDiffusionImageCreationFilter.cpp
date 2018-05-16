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

#include "mitkDiffusionImageCreationFilter.h"
#include "mitkProperties.h"
#include "mitkImageTimeSelector.h"

#include "mitkImageCast.h"
#include "mitkImageToItk.h"
#include "mitkImageAccessByItk.h"
#include "mitkITKImageImport.h"

#include "mitkIOUtil.h"

#include <itkImageFileWriter.h>

#include <itkComposeImageFilter.h>


/**
 * @brief RemapIntoVectorImage Take a 3d+t image and reinterpret it as vector image
 * @return vectoriamge
 */
mitk::DiffusionImageCreationFilter::VectorImageType::Pointer
mitk::DiffusionImageCreationFilter::RemapIntoVectorImage( mitk::Image::Pointer input)
{
  typedef itk::Image<mitk::DiffusionPropertyHelper::DiffusionPixelType, 3> ImageVolumeType;
  typedef itk::ComposeImageFilter< ImageVolumeType > ComposeFilterType;

  ComposeFilterType::Pointer vec_composer = ComposeFilterType::New();
  mitk::ImageTimeSelector::Pointer t_selector = mitk::ImageTimeSelector::New();
  t_selector->SetInput( input );

  for( unsigned int i=0; i< input->GetTimeSteps(); i++)
  {
    t_selector->SetTimeNr(i);
    t_selector->Update();

    ImageVolumeType::Pointer singleImageItk;
    mitk::CastToItkImage( t_selector->GetOutput(), singleImageItk );

    vec_composer->SetInput( i, singleImageItk );
  }

  try
  {
    vec_composer->Update();
  }
  catch( const itk::ExceptionObject& e)
  {
    MITK_ERROR << "Caught exception while updating compose filter: " << e.what();
  }

  mitk::DiffusionImageCreationFilter::VectorImageType::Pointer vector_image = vec_composer->GetOutput();
  vector_image->GetPixelContainer()->ContainerManageMemoryOff();

  return vector_image;
}

mitk::DiffusionImageCreationFilter::DiffusionImageCreationFilter()
  : m_ReferenceImage( nullptr )
{
  m_HeaderDescriptorSet = false;

  this->SetNumberOfRequiredInputs(1);
  this->SetNumberOfRequiredOutputs(1);
}

mitk::DiffusionImageCreationFilter::~DiffusionImageCreationFilter()
{

}

void mitk::DiffusionImageCreationFilter::SetReferenceImage( mitk::Image::Pointer reference_image )
{
  if( reference_image.IsNull() )
  {
    mitkThrow() << "Null-pointer image provided as reference. ";
  }

  if( ! DPH::IsDiffusionWeightedImage(reference_image) )
  {
    mitkThrow() << "The image provided as reference is not a diffusion-weighted image. Cannot proceed. ";
  }

  this->m_ReferenceImage = reference_image;
}

void mitk::DiffusionImageCreationFilter::GenerateData()
{
  const mitk::Image::Pointer input_image = this->GetInput(0);
  if( input_image.IsNull() )
  {
    mitkThrow() << "No input specified. Cannot proceed ";
  }

  if( !( m_HeaderDescriptorSet ^ m_ReferenceImage.IsNotNull() ) )
  {
    mitkThrow() << "Either a header descriptor or a reference diffusion-weighted image have to be provided. Terminating.";
  }

  mitk::Image::Pointer outputForCache = this->GetOutput();

  if( input_image->GetTimeSteps() > 1 )
  {
    mitk::Image::Pointer mitkvectorimage = mitk::GrabItkImageMemory<DPH::ImageType>( RemapIntoVectorImage( input_image ));
    outputForCache->Initialize( mitkvectorimage );
  }
  // no need to remap, we expect to have a vector image directly
  else
  {
    outputForCache->Initialize( input_image );
  }

  // header information
  GradientDirectionContainerType::Pointer DiffusionVectors =  this->InternalGetGradientDirections( );
  MeasurementFrameType MeasurementFrame = this->InternalGetMeasurementFrame();
  float BValue = this->InternalGetBValue();

  // create BValueMap
  mitk::BValueMapProperty::BValueMap BValueMap = mitk::BValueMapProperty::CreateBValueMap( DiffusionVectors, BValue );
  outputForCache->GetPropertyList()->ReplaceProperty( DPH::GRADIENTCONTAINERPROPERTYNAME.c_str(), mitk::GradientDirectionsProperty::New( DiffusionVectors ) );
  outputForCache->GetPropertyList()->ReplaceProperty( DPH::MEASUREMENTFRAMEPROPERTYNAME.c_str(), mitk::MeasurementFrameProperty::New( MeasurementFrame ) );
  outputForCache->GetPropertyList()->ReplaceProperty( DPH::BVALUEMAPPROPERTYNAME.c_str(), mitk::BValueMapProperty::New( BValueMap ) );
  outputForCache->GetPropertyList()->ReplaceProperty( DPH::REFERENCEBVALUEPROPERTYNAME.c_str(), mitk::FloatProperty::New( BValue ) );

  outputForCache->Modified();
}

void mitk::DiffusionImageCreationFilter::SetHeaderDescriptor(DiffusionImageHeaderDescriptor header_descriptor)
{
  this->m_HeaderDescriptor = header_descriptor;
  this->m_HeaderDescriptorSet = true;
}

mitk::DiffusionImageCreationFilter::MeasurementFrameType
mitk::DiffusionImageCreationFilter::InternalGetMeasurementFrame()
{
  MeasurementFrameType MeasurementFrame;

  if( m_ReferenceImage.IsNotNull() )
  {
    MeasurementFrame = DPH::GetMeasurementFrame( m_ReferenceImage );
  }
  else if ( m_HeaderDescriptorSet )
  {
    MeasurementFrame = m_HeaderDescriptor.m_MeasurementFrame;
  }
  else
  {
    MeasurementFrame(0,0) = 1;
    MeasurementFrame(0,1) = 0;
    MeasurementFrame(0,2) = 0;
    MeasurementFrame(1,0) = 0;
    MeasurementFrame(1,1) = 1;
    MeasurementFrame(1,2) = 0;
    MeasurementFrame(2,0) = 0;
    MeasurementFrame(2,1) = 0;
    MeasurementFrame(2,2) = 1;

    MITK_WARN << "Created default measurement frame as non provided ( no reference image or header information provided)";
  }

  return MeasurementFrame;
}

mitk::DiffusionImageCreationFilter::GradientDirectionContainerType::Pointer
mitk::DiffusionImageCreationFilter::InternalGetGradientDirections()
{
  GradientDirectionContainerType::Pointer DiffusionVectors = GradientDirectionContainerType::New();

  if( this->m_ReferenceImage )
  {
    DiffusionVectors = DPH::GetGradientContainer( this->m_ReferenceImage );
  }
  else if ( m_HeaderDescriptorSet )
  {
    DiffusionVectors = m_HeaderDescriptor.m_GradientDirections;
  }

  return DiffusionVectors;

}

float mitk::DiffusionImageCreationFilter::InternalGetBValue()
{
  float bvalue = -1;

  if( m_ReferenceImage.IsNotNull() )
  {
    bvalue = DPH::GetReferenceBValue( m_ReferenceImage );
  }
  else if ( m_HeaderDescriptorSet )
  {
    bvalue = m_HeaderDescriptor.m_BValue;
  }
  else
  {
    MITK_ERROR << "No reference image and no header descriptor provided.";
  }

  return bvalue;
}





