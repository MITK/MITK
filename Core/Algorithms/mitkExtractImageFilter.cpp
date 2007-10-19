#include "mitkExtractImageFilter.h"
#include "mitkImageCast.h"
#include "mitkPlaneGeometry.h"

#include <itkExtractImageFilter.h>

mitk::ExtractImageFilter::ExtractImageFilter()
:m_SliceIndex(0),
 m_SliceDimension(0)
{
}

mitk::ExtractImageFilter::~ExtractImageFilter()
{
}

void mitk::ExtractImageFilter::GenerateData()
{
   Image::ConstPointer input = ImageToImageFilter::GetInput(0);

   if ( (input->GetDimension() != 3) && (input->GetDimension() != 2) )
   {
     //std::cerr << "mitk::ExtractImageFilter works only with 3D images, sorry." << std::endl;
     itkExceptionMacro("mitk::ExtractImageFilter works only with 3D images, sorry.");
     return;
   }
   else if (input->GetDimension() == 2)
   {
     Image::Pointer resultImage = ImageToImageFilter::GetOutput();
     resultImage = const_cast<Image*>(input.GetPointer());
     ImageToImageFilter::SetNthOutput( 0, resultImage );
     return;
   }

   if ( m_SliceDimension >= input->GetDimension() )
   {
     //std::cerr << "mitk::ExtractImageFilter:  m_SliceDimension == " << m_SliceDimension << " makes no sense with an " << input->GetDimension() << "D image." << std::endl;
     itkExceptionMacro("This is not a sensible value for m_SliceDimension.");
     return;
   }

   AccessFixedDimensionByItk( input, ItkImageProcessing, 3 );
}
    
template<typename TPixel, unsigned int VImageDimension>
void mitk::ExtractImageFilter::ItkImageProcessing( itk::Image<TPixel,VImageDimension>* itkImage )
{
  // use the itk::ExtractImageFilter to get a 2D image
  typedef itk::Image< TPixel, VImageDimension >   ImageType3D;
  typedef itk::Image< TPixel, VImageDimension-1 > ImageType2D;

  typename ImageType3D::RegionType inSliceRegion = itkImage->GetLargestPossibleRegion();
  
  inSliceRegion.SetSize( m_SliceDimension, 0 );

  typedef itk::ExtractImageFilter<ImageType3D, ImageType2D> ExtractImageFilterType;

  typename ExtractImageFilterType::Pointer sliceExtractor = ExtractImageFilterType::New();
  sliceExtractor->SetInput( itkImage );
    
  inSliceRegion.SetIndex( m_SliceDimension, m_SliceIndex );

  sliceExtractor->SetExtractionRegion( inSliceRegion );

  // redirect ITK filter's output to this filter's output
  /* TODO make this work 
  std::cout << "UpdateOutputInformation" << std::endl;
  sliceExtractor->UpdateOutputInformation();
  std::cout << "GetOutput" << std::endl;
  Image::Pointer resultImage = ImageToImageFilter::GetOutput();
  std::cout << "InitializeByItk" << std::endl;
  resultImage->InitializeByItk( sliceExtractor->GetOutput() );
  ImageType2D::Pointer itkresult = ImageType2D::New();
  CastToItkImage( resultImage, itkresult );
  std::cout << "GraftOutput" << std::endl;
  sliceExtractor->GraftOutput( itkresult );
  */

  // calculate the output
  sliceExtractor->UpdateLargestPossibleRegion();

  typename ImageType2D::Pointer slice = sliceExtractor->GetOutput();
  // possible bug in itk::ExtractImageFilter: or in CastToItkImage ?
  // direction maxtrix is wrong/broken/not working with mitk::Image::InitializeByItkImage
  // solution here: we overwrite it with an unity matrix
  typename ImageType2D::DirectionType imageDirection;
  imageDirection.SetIdentity();
  slice->SetDirection(imageDirection);

  // re-import to MITK
  Image::Pointer resultImage = ImageToImageFilter::GetOutput();
  resultImage->InitializeByItk( slice.GetPointer() );
  resultImage->SetImportVolume( slice->GetBufferPointer(), 0, 0, Image::ManageMemory );
  slice->GetPixelContainer()->ContainerManageMemoryOff(); 
  // set a nice geometry for display and point transformations
  itk::Point< float, VImageDimension > origin;
  itkImage->TransformIndexToPhysicalPoint( inSliceRegion.GetIndex(), origin );

  Geometry3D* inputImageGeometry = ImageToImageFilter::GetInput(0)->GetGeometry();
  if (!inputImageGeometry)
  {
    std::cerr << "In ExtractImageFilter::ItkImageProcessing: Input image has no geometry!" << std::endl;
    return;
  }

  PlaneGeometry::PlaneOrientation orientation = PlaneGeometry::Transversal;

  switch ( m_SliceDimension )
  {
    default:
    case 2: 
      orientation = PlaneGeometry::Transversal;
      break;
    case 1: 
      orientation = PlaneGeometry::Frontal;
      break;
    case 0: 
      orientation = PlaneGeometry::Sagittal;
      break;
   }
 
  PlaneGeometry::Pointer planeGeometry = PlaneGeometry::New();
  planeGeometry->InitializeStandardPlane( inputImageGeometry, orientation, (ScalarType)m_SliceIndex + 0.5 , true, false );
  resultImage->SetGeometry( planeGeometry );
}


