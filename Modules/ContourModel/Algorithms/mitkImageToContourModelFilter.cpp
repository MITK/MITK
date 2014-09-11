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
#include "mitkImageAccessByItk.h"

#include <itkContourExtractor2DImageFilter.h>



mitk::ImageToContourModelFilter::ImageToContourModelFilter()
  : m_SliceGeometry(0)
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
  mitk::Image::ConstPointer sliceImage = this->GetInput();

  if ( !sliceImage )
  {
    MITK_ERROR << "mitk::ImageToContourModelFilter: No input available. Please set the input!" << std::endl;
    itkExceptionMacro("mitk::ImageToContourModelFilter: No input available. Please set the input!");
    return;
  }

  if ( sliceImage->GetDimension() > 2 || sliceImage->GetDimension() < 2)
  {
    MITK_ERROR << "mitk::ImageToContourModelFilter::GenerateData() works only with 2D images. Please assure that your input image is 2D!" << std::endl;
    itkExceptionMacro("mitk::ImageToContourModelFilter::GenerateData() works only with 2D images. Please assure that your input image is 2D!");
    return;
  }

  m_SliceGeometry = sliceImage->GetGeometry();

  AccessFixedDimensionByItk(sliceImage, Itk2DContourExtraction, 2);
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::ImageToContourModelFilter::Itk2DContourExtraction (const itk::Image<TPixel, VImageDimension>* sliceImage)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::ContourExtractor2DImageFilter<ImageType> ContourExtractor;

  typedef itk::PolyLineParametricPath<2> PolyLineParametricPath2D;
  typedef PolyLineParametricPath2D::VertexListType ContourPath;

  typename ContourExtractor::Pointer contourExtractor = ContourExtractor::New();
  contourExtractor->SetInput(sliceImage);
  contourExtractor->SetContourValue(0.5);

  contourExtractor->Update();

  unsigned int foundPaths = contourExtractor->GetNumberOfOutputs();
  this->SetNumberOfIndexedOutputs(foundPaths);

  for (unsigned int i = 0; i < foundPaths; i++)
  {
    const ContourPath* currentPath = contourExtractor->GetOutput(i)->GetVertexList();

    mitk::Point3D currentPoint;
    mitk::Point3D currentWorldPoint;

    mitk::ContourModel::Pointer contour = this->GetOutput(i);

    for (unsigned int j = 0; j < currentPath->Size(); j++)
    {

      currentPoint[0] = currentPath->ElementAt(j)[0];
      currentPoint[1] = currentPath->ElementAt(j)[1];
      currentPoint[2] = 0;

      m_SliceGeometry->IndexToWorld(currentPoint, currentWorldPoint);

      contour->AddVertex(currentWorldPoint);
    }//for2

    contour->Close();

  }//for1
}
