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

#include "mitkImageToContourModelFilter.h"
//#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"
#include "mitkProgressBar.h"

mitk::ImageToContourModelFilter::ImageToContourModelFilter() :
m_UseProgressBar(false),
m_ProgressStepSize(1)
{
}

mitk::ImageToContourModelFilter::~ImageToContourModelFilter()
{

}

void mitk::ImageToContourModelFilter::SetInput ( const mitk::ImageToContourModelFilter::InputType* input )
{
  this->SetInput( 0, input );
}

void mitk::ImageToContourModelFilter::SetInput ( unsigned int idx, const mitk::ImageToContourModelFilter::InputType* input )
{
  if ( idx + 1 > this->GetNumberOfInputs() )
  {
    this->SetNumberOfRequiredInputs(idx + 1);
  }
  if ( input != static_cast<InputType*> ( this->ProcessObject::GetInput ( idx ) ) )
  {
    this->ProcessObject::SetNthInput ( idx, const_cast<InputType*> ( input ) );
    this->Modified();
  }
}



const mitk::ImageToContourModelFilter::InputType* mitk::ImageToContourModelFilter::GetInput( void )
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;
  return static_cast<const mitk::ImageToContourModelFilter::InputType*>(this->ProcessObject::GetInput(0));
}



const mitk::ImageToContourModelFilter::InputType* mitk::ImageToContourModelFilter::GetInput( unsigned int idx )
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;
  return static_cast<const mitk::ImageToContourModelFilter::InputType*>(this->ProcessObject::GetInput(idx));
}


void mitk::ImageToContourModelFilter::GenerateData()
{
  mitk::Image::ConstPointer sliceImage = ImageToContourModelFilter::GetInput();

  if ( !sliceImage )
  {
    MITK_ERROR << "mitk::ImageToContourFilter: No input available. Please set the input!" << std::endl;
    itkExceptionMacro("mitk::ImageToContourFilter: No input available. Please set the input!");
    return;
  }

  if ( sliceImage->GetDimension() > 2 || sliceImage->GetDimension() < 2)
  {
    MITK_ERROR << "mitk::ImageToImageFilter::GenerateData() works only with 2D images. Please assure that your input image is 2D!" << std::endl;
    itkExceptionMacro("mitk::ImageToImageFilter::GenerateData() works only with 2D images. Please assure that your input image is 2D!");
    return;
  }

  m_SliceGeometry = sliceImage->GetGeometry();

  AccessFixedDimensionByItk(sliceImage, ExtractContoursITKProcessing, 2);
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::ImageToContourModelFilter::ExtractContoursITKProcessing (itk::Image<TPixel, VImageDimension>* sliceImage)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::ContourExtractor2DImageFilter<ImageType> ContourExtractorType;

  typename ContourExtractorType::Pointer contourExtractor = ContourExtractorType::New();
  contourExtractor->SetInput(sliceImage);
  contourExtractor->SetContourValue(0.5);
  contourExtractor->Update();

  unsigned int foundPaths = contourExtractor->GetNumberOfOutputs();

  mitk::ContourModel::Pointer output = ImageToContourModelFilter::GetOutput();
  output->Initialize();

  unsigned int pointId (0);

//  for (unsigned int i = 0; i < foundPaths; i++)
//  {
  if (!contourExtractor->GetOutput(0)) return;

  const ContourPath* currentPath = contourExtractor->GetOutput(0)->GetVertexList();

    if (m_UseProgressBar)
      mitk::ProgressBar::GetInstance()->AddStepsToDo( currentPath->Size() );

    for (unsigned int j = 0; j < currentPath->Size(); j++)
    {
      mitk::Point3D currentWorldPoint;
      currentWorldPoint[0] = currentPath->ElementAt(j)[0];
      currentWorldPoint[1] = currentPath->ElementAt(j)[1];
      currentWorldPoint[2] = 0;

      m_SliceGeometry->IndexToWorld(currentWorldPoint, currentWorldPoint);
      output->AddVertex(currentWorldPoint);

      if (m_UseProgressBar)
        mitk::ProgressBar::GetInstance()->Progress(1);
    }
}

void mitk::ImageToContourModelFilter::GenerateOutputInformation()
{
  Superclass::GenerateOutputInformation();
}

void mitk::ImageToContourModelFilter::SetUseProgressBar(bool status)
{
  m_UseProgressBar = status;
}
