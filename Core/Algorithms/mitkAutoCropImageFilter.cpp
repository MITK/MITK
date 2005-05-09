#include "mitkAutoCropImageFilter.h"

#include "mitkImageCast.h"
#include "mitkImageTimeSelector.h"
#include "itkLinearInterpolateImageFunction.h"
#include <itkBinaryMedianImageFilter.h>
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>
#include <itkCropImageFilter.h>
#include <mitkImage.h>
#include <vtkLinearTransform.h>

mitk::AutoCropImageFilter::AutoCropImageFilter() : m_BackgroundValue(0), m_MarginFactor(1.0)
{
}


mitk::AutoCropImageFilter::~AutoCropImageFilter()
{
}

void mitk::AutoCropImageFilter::GenerateOutputInformation()
{
	mitk::Image::ConstPointer input  = this->GetInput();
	mitk::Image::Pointer output = this->GetOutput();

	itkDebugMacro(<<"GenerateOutputInformation()");

  int dim=(input->GetDimension()<3?input->GetDimension():3);
	output->Initialize(input->GetPixelType(), dim, input->GetDimensions());

  // initialize geometry
  output->SetGeometry(dynamic_cast<Geometry3D*>(input->GetSlicedGeometry()->Clone().GetPointer()));
  output->SetPropertyList(input->GetPropertyList()->Clone());
}

void mitk::AutoCropImageFilter::GenerateData()
{


  if ( this->GetInput()->GetDimension()==4)
  {
    mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
    timeSelector->SetInput( this->GetInput() );
    timeSelector->SetTimeNr( 0 );
    timeSelector->Update(); 
    this->Crop3DImage( timeSelector->GetOutput() );
  }
  else if (this->GetInput()->GetDimension() == 3 )
  {
    this->Crop3DImage( this->GetInput() );
  }

}

void mitk::AutoCropImageFilter::Crop3DImage(mitk::ImageToImageFilter::InputImageConstPointer img)
{
  typedef itk::Image<float,3>    ImageType;
  typedef ImageType::Pointer      ImagePointer;

  ImagePointer inputItk = ImageType::New();
  mitk::CastToItkImage( img , inputItk );
  typedef itk::ImageRegionConstIterator< ImageType > ConstIteratorType;
  ConstIteratorType inIt( inputItk,  inputItk->GetLargestPossibleRegion() );
  ImageType::IndexType minima,maxima;
  maxima = inputItk->GetLargestPossibleRegion().GetIndex();
  minima[0] = inputItk->GetLargestPossibleRegion().GetSize()[0];
  minima[1] = inputItk->GetLargestPossibleRegion().GetSize()[1];
  minima[2] = inputItk->GetLargestPossibleRegion().GetSize()[2];

  for ( inIt.GoToBegin(); !inIt.IsAtEnd(); ++inIt)
  {
    float pix_val = inIt.Get();
    if ( fabs(pix_val - m_BackgroundValue) > mitk::eps )
    {
      for (int i=0; i < 3; i++)
      {
        minima[i] = vnl_math_min((int)minima[i],(int)(inIt.GetIndex()[i]));
        maxima[i] = vnl_math_max((int)maxima[i],(int)(inIt.GetIndex()[i]));
      }
    }
  }

  ImageType::RegionType::SizeType regionSize;
  ImageType::RegionType::IndexType regionIndex;

  regionSize[0] = (ImageType::RegionType::SizeType::SizeValueType)(m_MarginFactor * (maxima[0] - minima[0]));
  regionSize[1] = (ImageType::RegionType::SizeType::SizeValueType)(m_MarginFactor * (maxima[1] - minima[1]));
  regionSize[2] = (ImageType::RegionType::SizeType::SizeValueType)(m_MarginFactor * (maxima[2] - minima[2]));
  regionIndex = minima;
  regionIndex[0] -= (regionSize[0] - maxima[0] + minima[0])/2 ;
  regionIndex[1] -= (regionSize[1] - maxima[1] + minima[1])/2 ;
  regionIndex[2] -= (regionSize[2] - maxima[2] + minima[2])/2 ;

  for (int i = 0; i < 3; i++)
  {
    if (regionIndex[i] < 0) regionIndex[i] = 0;
    if (regionIndex[i] + regionSize[i] > inputItk->GetLargestPossibleRegion().GetSize()[i]-1) regionSize[i] = inputItk->GetLargestPossibleRegion().GetSize()[i] - regionIndex[i] - 1;

    // check if there was enough segmentation to do the registration and return otherwise
    if (regionSize[i] < 1) return;
  }

  ImagePointer outputItk = ImageType::New();
  typedef itk::CropImageFilter<ImageType,ImageType> CropFilterType;
  CropFilterType::SizeType lowerBounds,upperBounds;

  lowerBounds[0] = regionIndex[0];
  lowerBounds[1] = regionIndex[1];
  lowerBounds[2] = regionIndex[2];

  upperBounds[0] = inputItk->GetLargestPossibleRegion().GetSize()[0]-regionSize[0]-regionIndex[0];
  upperBounds[1] = inputItk->GetLargestPossibleRegion().GetSize()[1]-regionSize[1]-regionIndex[1];
  upperBounds[2] = inputItk->GetLargestPossibleRegion().GetSize()[2]-regionSize[2]-regionIndex[2];

  CropFilterType::Pointer cropFilter = CropFilterType::New();
  cropFilter->SetLowerBoundaryCropSize( lowerBounds );
  cropFilter->SetUpperBoundaryCropSize( upperBounds );
  cropFilter->SetInput( inputItk );
  cropFilter->Update();
  outputItk = cropFilter->GetOutput();

  float origin[3];
  origin[0] = regionIndex[0];
  origin[1] = regionIndex[1];
  origin[2] = regionIndex[2];
  outputItk->SetOrigin( origin );
  outputItk->SetSpacing( inputItk->GetSpacing() );

  mitk::Image::Pointer output = this->GetOutput();
  mitk::CastToMitkImage(outputItk, output);
  output->SetPropertyList(this->GetInput()->GetPropertyList()->Clone());
  output->SetGeometry( dynamic_cast<Geometry3D*>(this->GetInput()->GetSlicedGeometry()->Clone().GetPointer()) );
  Vector3D originVector;
  vtk2itk(origin, originVector);
  output->GetGeometry()->Translate(originVector);
  this->SetOutput(output);  

}

void mitk::AutoCropImageFilter::GenerateInputRequestedRegion()
{
  //todo
}
