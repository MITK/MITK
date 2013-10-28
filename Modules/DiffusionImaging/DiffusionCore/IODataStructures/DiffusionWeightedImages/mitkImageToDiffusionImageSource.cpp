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

#ifndef __mitk_ImageToDiffusionImageSource_txx
#define __mitk_ImageToDiffusionImageSource_txx

#include "mitkImageToDiffusionImageSource.h"

#include <itkComposeImageFilter.h>
#include <itkExtractImageFilter.h>

#include "mitkImageTimeSelector.h"

#include <itkVectorImage.h>
#include <itkImage.h>

#include "mitkImageCast.h"

#include "mitkException.h"

template< typename TPixelType>
mitk::ImageToDiffusionImageSource<TPixelType>::ImageToDiffusionImageSource()
  : m_SourceImage(0),
    m_GradientDirections(0)
{

}

template< typename TPixelType>
void mitk::ImageToDiffusionImageSource<TPixelType>
::GenerateOutputInformation()
{
  // sanity checks ( input not null, timesteps corresponds to number of directions specified )
  if( m_GradientDirections.IsNull() || m_GradientDirections->empty() )
  {
    mitkThrow() << "No gradient directions were set. Cannot proceed.";
  }

  if( m_SourceImage.IsNull() )
  {
    mitkThrow() << "No input image was set. Cannot proceed.";
  }

  if( m_GradientDirections->size() != m_SourceImage->GetTimeSteps() )
  {
      mitkThrow() << "Size mismatch between container size " << m_GradientDirections->size() <<
                     "and image volumes."<< m_SourceImage->GetTimeSteps() <<" Cannot proceed.";
  }


  // already pass in the meta-data
  typename OutputType::Pointer metaImage = OutputType::New();

  // set directions and bvalue
  metaImage->SetDirections(this->m_GradientDirections);
  metaImage->SetB_Value(this->m_BValue);

  // set identity as measurement frame
  vnl_matrix_fixed< double, 3, 3 > measurement_frame;
  measurement_frame.set_identity();
  metaImage->SetMeasurementFrame( measurement_frame );


  m_OutputCache = metaImage;
  m_CacheTime.Modified();

}

template< typename TPixelType>
void mitk::ImageToDiffusionImageSource<TPixelType>
::GenerateData()
{
  if ( ( ! m_OutputCache ) || ( this->GetMTime( ) > m_CacheTime.GetMTime( ) ) )
  {
    this->GenerateOutputInformation();
    itkWarningMacro("Cache regenerated!");
  }

  // now cast the mitk image to the vector image and pass in as volume
  typedef itk::Image< TPixelType, 4 > InputItkType;
  typedef itk::Image< TPixelType, 3> SingleVolumeType;
  typedef itk::VectorImage< TPixelType, 3> VectorImageType;

  typedef itk::ComposeImageFilter< SingleVolumeType > ComposeFilterType;
  typename ComposeFilterType::Pointer vec_composer = ComposeFilterType::New();

  mitk::ImageTimeSelector::Pointer t_selector =
      mitk::ImageTimeSelector::New();

  t_selector->SetInput( m_SourceImage );

  for( unsigned int i=0; i< m_SourceImage->GetTimeSteps(); i++)
  {
    t_selector->SetTimeNr(i);
    t_selector->Update();

    typename SingleVolumeType::Pointer singleImageItk;
    mitk::CastToItkImage( t_selector->GetOutput(), singleImageItk );

    vec_composer->SetInput( i, singleImageItk );

  }

  try
  {
    vec_composer->Update();
  }
  catch( const itk::ExceptionObject& e)
  {
    MITK_ERROR << "Caugt exception while updating compose filter: " << e.what();
  }


  m_OutputCache->SetVectorImage( vec_composer->GetOutput() );

  // transfer to the output image
  static_cast<OutputType*>(this->GetOutput())
      ->SetVectorImage(m_OutputCache->GetVectorImage());
  static_cast<OutputType*>(this->GetOutput())
      ->SetB_Value(m_OutputCache->GetB_Value());
  static_cast<OutputType*>(this->GetOutput())
      ->SetDirections(m_OutputCache->GetDirections());
  static_cast<OutputType*>(this->GetOutput())
      ->SetMeasurementFrame(m_OutputCache->GetMeasurementFrame());
  static_cast<OutputType*>(this->GetOutput())
      ->InitializeFromVectorImage();


}

#endif //__mitk_ImageToDiffusionImageSource_txx
