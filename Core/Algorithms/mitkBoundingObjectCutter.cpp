#include "mitkBoundingObjectCutter.h"

#include "mitkImageToItkMultiplexer.h"
#include "mitkImage.h"
#include "mitkBoundingObject.h"
#include "mitkGeometry3D.h"

#include <mitkStatusBar.h>

#include <vtkTransform.h>


mitk::BoundingObjectCutter::BoundingObjectCutter()
: m_UseInsideValue(false), m_OutsideValue(0), m_InsideValue(1), m_BoundingObject(NULL), 
  m_ConfidenceFactor(1.92), m_UseRegionGrower(true), m_SegmentationFilter(NULL)
{  
}

mitk::BoundingObjectCutter::~BoundingObjectCutter()
{
}

/**
 * \todo check if this is no conflict to the ITK filter writing rules -> ITK SoftwareGuide p.512
 */  
void mitk::BoundingObjectCutter::GenerateOutputInformation()
{
  itkDebugMacro(<<"GenerateOutputInformation()");
  //if(input.IsNull()) return;
}

void mitk::BoundingObjectCutter::GenerateData()
{
  if(m_BoundingObject.IsNull())
    return;  // return empty image/source image?

  ImageToImageFilter::InputImageConstPointer inputImageMitkPointer = this->GetInput();
  if(inputImageMitkPointer.IsNull()) 
    return;  // Eigentlich: eigenes Spacing/dimensions festlegen und neues Binärbild erzeugen
  
  mitk::Image::Pointer inputImageMitk = const_cast<mitk::Image*>(inputImageMitkPointer.GetPointer());  // const away cast, because FixedInput...Multiplexer Macro needs non const Pointer

  // convert input mitk image to itk image    
  ItkImageType::Pointer itkImage = ItkImageType::New();
  FixedInputDimensionMitkToItkFunctionMultiplexer(itkImage, ItkImageType , inputImageMitk, 3, MakeCastImageFilter);  
  
  // check if image conversion failed
  if (itkImage.IsNull())
  {
    mitk::StatusBar::DisplayErrorText ("An internal error occurred. Can't convert Image. Please report to bugs@mitk.org");
    std::cout << " image is NULL...returning" << std::endl;
    return; // return and do nothing in case of failure
  }
  // calculate region of interest
  m_BoundingObject->UpdateOutputInformation();
  mitk::BoundingBox::Pointer boundingBox = const_cast<mitk::BoundingBox*>(m_BoundingObject->GetGeometry()->GetBoundingBox());
  const mitk::BoundingBox::PointType minPoint = boundingBox->GetMinimum();
  const mitk::BoundingBox::PointType maxPoint = boundingBox->GetMaximum();
  float* posPoint = m_BoundingObject->GetGeometry()->GetVtkTransform()->GetPosition();
  ItkImageType::IndexType start;
  start[0] = static_cast<ItkImageType::IndexType::IndexValueType>(posPoint[0] + minPoint[0] + 0.5); // first index on X
  start[1] = static_cast<ItkImageType::IndexType::IndexValueType>(posPoint[1] + minPoint[1] + 0.5); // first index on Y
  start[2] = static_cast<ItkImageType::IndexType::IndexValueType>(posPoint[2] + minPoint[2] + 0.5); // first index on Z
  ItkImageType::SizeType size;
  size[0] = static_cast<ItkImageType::SizeType::SizeValueType>(maxPoint[0] - minPoint[0]); // size along X
  size[1] = static_cast<ItkImageType::SizeType::SizeValueType>(maxPoint[1] - minPoint[1]); // size along Y
  size[2] = static_cast<ItkImageType::SizeType::SizeValueType>(maxPoint[2] - minPoint[2]); // size along Z
  ItkRegionType regionOfInterest;
  regionOfInterest.SetSize( size );
  regionOfInterest.SetIndex( start );

  ItkRegionOfInterestFilterType::Pointer regionOfInterestFilter = ItkRegionOfInterestFilterType::New();
  regionOfInterestFilter->SetRegionOfInterest(regionOfInterest);
  regionOfInterestFilter->SetInput(itkImage);  
  ItkImageType::Pointer itkImageCut = regionOfInterestFilter->GetOutput();  
  itkImageCut->Update();  // Cut the region of interest out of the source image

  ItkImageIteratorType it(itkImageCut, itkImageCut->GetRequestedRegion());
  mitk::ITKPoint3D p;
  if (GetUseInsideValue()) // use a fixed value for each inside pixel(create a binary mask of the bounding object)
    for (it.GoToBegin(); !it.IsAtEnd(); ++it)
    {          
      itkImageCut->TransformIndexToPhysicalPoint(it.GetIndex(), p);  // transform index of current pixel to world coordinate point
      if(m_BoundingObject->IsInside(p))
        it.Value() = m_InsideValue;
      else
        it.Value() = m_OutsideValue;
    }
  else // no fixed value for inside, but the pixel value of the original image (normal cutting)
    for ( it.GoToBegin(); !it.IsAtEnd(); ++it)
    {    
      // transform to world coordinates and check if p is inside the bounding object
      itkImageCut->TransformIndexToPhysicalPoint(it.GetIndex(), p);
      if(!m_BoundingObject->IsInside(p))
        it.Value() = m_OutsideValue;
    }

    mitk::Image::Pointer outputImage = this->GetOutput();

    if (m_UseRegionGrower)
    {
      std::cout << " cutting done, now starting region grower.\n";
      //// now start a region growing filter
      //ConnectedFilterType::Pointer confidenceConnected = ConnectedFilterType::New();
      //confidenceConnected->SetInput( itkImageCut );
      ////confidenceConnected->SetMultiplier( 1.88 );
      //confidenceConnected->SetMultiplier( m_ConfidenceFactor );
      //confidenceConnected->SetNumberOfIterations( 5 );
      //confidenceConnected->SetReplaceValue( 10000 );
      //
      //// Set seedpoint to center of image
      //ItkImageType::IndexType index; 
      //index[0] = static_cast<ItkImageType::IndexType::IndexValueType>(size[0] / 2.0);
      //index[1] = static_cast<ItkImageType::IndexType::IndexValueType>(size[1] / 2.0);
      //index[2] = static_cast<ItkImageType::IndexType::IndexValueType>(size[2] / 2.0);
      //confidenceConnected->SetSeed( index );
      //
      //confidenceConnected->SetInitialNeighborhoodRadius( 2 );
    
      //MultiplyImageFilterType::Pointer multiplyFilter = MultiplyImageFilterType::New();
      //multiplyFilter->SetInput1(confidenceConnected->GetOutput());
      //multiplyFilter->SetInput2(itkImageCut);

      m_SegmentationFilter->SetInput( itkImageCut );

      try
      {
        //multiplyFilter->Update();
        //confidenceConnected->Update();
        m_SegmentationFilter->Update();
      }
      catch( itk::ExceptionObject & excep )
      {
        std::cerr << "Exception while updating RegiongrowingFilter!" << std::endl;
        std::cerr << excep << std::endl;
      }
      // convert the itk image back to an mitk image and set it as output for this filter
      //outputImage->InitializeByItk(confidenceConnected->GetOutput());
      //outputImage->SetVolume(confidenceConnected->GetOutput()->GetBufferPointer());
      outputImage->InitializeByItk(m_SegmentationFilter->GetOutput());
      outputImage->SetVolume(m_SegmentationFilter->GetOutput()->GetBufferPointer());

    }
    else
    {
      // convert the itk image back to an mitk image and set it as output for this filter
      outputImage->InitializeByItk(itkImageCut.GetPointer());
      outputImage->SetVolume(itkImageCut->GetBufferPointer());
    }
    //this->GraftOutput(outputImage);
}
