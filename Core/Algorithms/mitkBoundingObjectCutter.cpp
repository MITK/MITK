#include "mitkBoundingObjectCutter.h"
#include "mitkImageToItkMultiplexer.h"
#include "mitkImage.h"
#include "mitkBoundingObject.h"
#include "mitkGeometry3D.h"

#include <mitkStatusBar.h>

#include <vtkTransform.h>


mitk::BoundingObjectCutter::BoundingObjectCutter()
: m_UseInsideValue(false), m_OutsideValue(0), m_InsideValue(1), m_BoundingObject(NULL), 
  m_UseRegionGrower(true), m_SegmentationFilter(NULL), m_OutsidePixelCount(0), m_InsidePixelCount(0),
  m_ResampleFactor(1.0)
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
  mitk::Image::Pointer outputImage = this->GetOutput();
 
  // Check if there is an input bounding object
  if(m_BoundingObject.IsNull())
  {
    outputImage = NULL;
    return;  // return empty image/source image?
  }

  ImageToImageFilter::InputImageConstPointer inputImageMitkPointer = this->GetInput();
  if(inputImageMitkPointer.IsNull())
  {
    outputImage = NULL;
    return;  // Eigentlich: eigenes Spacing/dimensions festlegen und neues Binärbild erzeugen
  }

  // convert input mitk image to itk image
  mitk::Image::Pointer inputImageMitk = const_cast<mitk::Image*>(inputImageMitkPointer.GetPointer());  // const away cast, because FixedInput...Multiplexer Macro needs non const Pointer
  ItkImageType::Pointer itkImage = ItkImageType::New();
  FixedInputDimensionMitkToItkFunctionMultiplexer(itkImage, ItkImageType , inputImageMitk, 3, MakeCastImageFilter);    
  // check if image conversion failed
  if (itkImage.IsNull())
  {
    mitk::StatusBar::DisplayErrorText ("An internal error occurred. Can't convert Image. Please report to bugs@mitk.org");
    std::cout << " image is NULL...returning" << std::endl;
    outputImage = NULL; 
    return; // return and do nothing in case of failure
  }
  //std::cout << "itk Spacing:  " << itkImage->GetSpacing()[0] << ", " << itkImage->GetSpacing()[1] << ", " << itkImage->GetSpacing()[2] << ".\n";
  //std::cout << "mitk Spacing: " << inputImageMitk->GetSlicedGeometry()->GetSpacing()[0] << ", " << inputImageMitk->GetSlicedGeometry()->GetSpacing()[1] << ", " << inputImageMitk->GetSlicedGeometry()->GetSpacing()[2] << ".\n";

  // calculate region of interest
  m_BoundingObject->UpdateOutputInformation();
  mitk::BoundingBox::Pointer boundingBox = const_cast<mitk::BoundingBox*>(m_BoundingObject->GetGeometry()->GetBoundingBox());
  
  /* get BoundingBox min max and center in world coordinates */

  mitk::BoundingBox::PointType minPoint = boundingBox->GetMinimum();
  mitk::ScalarType min[3];
  min[0] = minPoint[0];   min[1] = minPoint[1];   min[2] = minPoint[2];
  m_BoundingObject->GetGeometry()->GetVtkTransform()->TransformPoint(min, min);
  mitk::ITKPoint3D globalMinPoint;
  mitk::ITKPoint3D globalMaxPoint;
  globalMinPoint[0] = min[0];   globalMinPoint[1] = min[1];   globalMinPoint[2] = min[2];  
  globalMaxPoint[0] = min[0];   globalMaxPoint[1] = min[1];   globalMaxPoint[2] = min[2];
  /* create all 8 points of the bounding box */
  mitk::BoundingBox::PointsContainerPointer points = mitk::BoundingBox::PointsContainer::New();
  mitk::ITKPoint3D p;
  p = boundingBox->GetMinimum();
  points->InsertElement(0, p);
  p[0] = -p[0];
  points->InsertElement(1, p);
  p = boundingBox->GetMinimum();
  p[1] = -p[1];    
  points->InsertElement(2, p);
  p = boundingBox->GetMinimum();
  p[2] = -p[2];    
  points->InsertElement(3, p);
  p = boundingBox->GetMaximum();
  points->InsertElement(4, p);
  p[0] = -p[0];
  points->InsertElement(5, p);
  p = boundingBox->GetMaximum();
  p[1] = -p[1];    
  points->InsertElement(6, p);
  p = boundingBox->GetMaximum();
  p[2] = -p[2];    
  points->InsertElement(7, p);
  mitk::BoundingBox::PointsContainerConstIterator pointsIterator = points->Begin();
  mitk::BoundingBox::PointsContainerConstIterator pointsIteratorEnd = points->End();
  while (pointsIterator != pointsIteratorEnd) // for each vertex of the bounding box
  {
    minPoint = pointsIterator->Value();
    min[0] = minPoint[0];   min[1] = minPoint[1];   min[2] = minPoint[2];
    m_BoundingObject->GetGeometry()->GetVtkTransform()->TransformPoint(min, min);  // transform vertex point to world coordinates

    globalMinPoint[0] = (min[0] < globalMinPoint[0]) ? min[0] : globalMinPoint[0];  // check if world point
    globalMinPoint[1] = (min[1] < globalMinPoint[1]) ? min[1] : globalMinPoint[1];  // has a lower or a
    globalMinPoint[2] = (min[2] < globalMinPoint[2]) ? min[2] : globalMinPoint[2];  // higher value as
    globalMaxPoint[0] = (min[0] > globalMaxPoint[0]) ? min[0] : globalMaxPoint[0];  // the last known highest
    globalMaxPoint[1] = (min[1] > globalMaxPoint[1]) ? min[1] : globalMaxPoint[1];  // value
    globalMaxPoint[2] = (min[2] > globalMaxPoint[2]) ? min[2] : globalMaxPoint[2];  // in each axis
    pointsIterator++;
  }

  /* calculate regíon of interest in pixel values */
  ItkImageType::IndexType start;
  itkImage->TransformPhysicalPointToIndex(globalMinPoint, start);
  
  ItkImageType::SizeType size;  
  size[0] = static_cast<ItkImageType::SizeType::SizeValueType>((globalMaxPoint[0] - globalMinPoint[0])/ itkImage->GetSpacing()[0]); // number of pixels along X axis
  size[1] = static_cast<ItkImageType::SizeType::SizeValueType>((globalMaxPoint[1] - globalMinPoint[1])/ itkImage->GetSpacing()[1]); // number of pixels along Y axis
  size[2] = static_cast<ItkImageType::SizeType::SizeValueType>((globalMaxPoint[2] - globalMinPoint[2])/ itkImage->GetSpacing()[2]); // number of pixels along Z axis
  ItkRegionType regionOfInterest;
  regionOfInterest.SetSize(size);
  regionOfInterest.SetIndex(start);

  ItkRegionOfInterestFilterType::Pointer regionOfInterestFilter = ItkRegionOfInterestFilterType::New();
  regionOfInterestFilter->SetRegionOfInterest(regionOfInterest);
  regionOfInterestFilter->SetInput(itkImage);  
  ItkImageType::Pointer itkImageCut = regionOfInterestFilter->GetOutput();  
  try 
  {
    itkImageCut->Update();  // Cut the region of interest out of the source image
  } 
  catch (itk::ExceptionObject & error) 
  {
    std::cout << "Cutting not possible, bounding box is outside of the source image.\n";
    std::cout << error;
    (mitk::StatusBar::GetInstance())->DisplayText("Cutting not possible, bounding box is outside of the source image.\n", 5000);
    outputImage = NULL;
    return;
  }

  ItkImageIteratorType imageIterator(itkImageCut, itkImageCut->GetRequestedRegion());
//  mitk::ITKPoint3D p;
  bool inside = false;
  m_OutsidePixelCount = 0;
  m_InsidePixelCount = 0;
  //m_UseInsideValue = true;  // just for testing
  //m_InsideValue = 30000;  // just for testing
  //m_OutsideValue = -30000;  // just for testing
  if (GetUseInsideValue()) // use a fixed value for each inside pixel(create a binary mask of the bounding object)
    for (imageIterator.GoToBegin(); !imageIterator.IsAtEnd(); ++imageIterator)
    {          
      itkImageCut->TransformIndexToPhysicalPoint(imageIterator.GetIndex(), p);  // transform index of current pixel to world coordinate point
      if(m_BoundingObject->IsInside(p))
      {
        imageIterator.Value() = m_InsideValue;
        m_InsidePixelCount++;
      }
      else
      {
        imageIterator.Value() = m_OutsideValue;
        m_OutsidePixelCount++;
      }
    }
  else // no fixed value for inside, but the pixel value of the original image (normal cutting)
    for ( imageIterator.GoToBegin(); !imageIterator.IsAtEnd(); ++imageIterator)
    {    
      // transform to world coordinates and check if p is inside the bounding object
      itkImageCut->TransformIndexToPhysicalPoint(imageIterator.GetIndex(), p);
      if(m_BoundingObject->IsInside(p))
      {
        m_InsidePixelCount++;
      }
      else
      {
        imageIterator.Value() = m_OutsideValue;
        m_OutsidePixelCount++;
      }
    }
    // convert the itk image back to an mitk image and set it as output for this filter
    outputImage->InitializeByItk(itkImageCut.GetPointer());
    outputImage->SetVolume(itkImageCut->GetBufferPointer());
}
