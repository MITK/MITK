#include "mitkBoundingObjectCutter.h"

#include "mitkImageToItkMultiplexer.h"
#include "mitkImage.h"
#include "mitkBoundingObject.h"
#include "mitkGeometry3D.h"

#include <mitkStatusBar.h>

#include <vtkTransform.h>


mitk::BoundingObjectCutter::BoundingObjectCutter()
: m_UseInsideValue(false), m_OutsideValue(0), m_InsideValue(1), m_BoundingObject(NULL)
{  
}

mitk::BoundingObjectCutter::~BoundingObjectCutter()
{
}

//void mitk::BoundingObjectCutter::SetBoundingObject(mitk::Ellipsoid* _arg)
//{  
//  if (m_BoundingObject != _arg)
//    _arg = _arg;
//}
//const mitk::Ellipsoid* mitk::BoundingObjectCutter::GetBoundingObject()
//{
//  return m_BoundingObject.GetPointer();
//}

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
  itkDebugMacro(<<"GenerateData()");

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
  start[0] = posPoint[0] + minPoint[0]; // first index on X
  start[1] = posPoint[1] + minPoint[1]; // first index on Y
  start[2] = posPoint[2] + minPoint[2]; // first index on Z
  ItkImageType::SizeType size;
  size[0] = maxPoint[0] - minPoint[0]; // size along X
  size[1] = maxPoint[1] - minPoint[1]; // size along Y
  size[2] = maxPoint[2] - minPoint[2]; // size along Z
  ItkRegionType regionOfInterest;
  regionOfInterest.SetSize( size );
  regionOfInterest.SetIndex( start );

  ItkRegionOfInterestFilterType::Pointer regionOfInterestFilter = ItkRegionOfInterestFilterType::New();
  regionOfInterestFilter->SetRegionOfInterest(regionOfInterest);
  regionOfInterestFilter->SetInput(itkImage);  
  ItkImageType::Pointer itkImageCut = regionOfInterestFilter->GetOutput();
  
  itkImageCut->Update();  // Cut the region of interest out of the source image

  ItkImageIteratorType it(itkImageCut, itkImageCut->GetRequestedRegion());

//  ItkImageType::IndexType index;
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
      // index in weltkoordinaten umrechnen und testen, ob IsInside() in boundingobject      
      itkImageCut->TransformIndexToPhysicalPoint(it.GetIndex(), p);
      //p = ;
      if(!m_BoundingObject->IsInside(p))
        it.Value() = m_OutsideValue;
    }
    // convert the itk image back to an mitk image and set it as outout for this filter
    mitk::Image::Pointer outputImage = this->GetOutput();
    outputImage->InitializeByItk(itkImageCut.GetPointer());
    outputImage->SetVolume(itkImageCut->GetBufferPointer());
    //this->GraftOutput(outputImage);
    m_ItkImage = itkImageCut;

    //ItkImageWriter::Pointer writer = ItkImageWriter::New();
    //writer->SetInput(itkImageCut);
    //writer->SetFileName("c:\\cutimage.vtk");
    //writer->Update();

    //CommonFunctionality::AddItkImageToDataTree(itkImageCut,	mitk::DataTreeIterator *  	  iterator, "ausgeschnittenes Bild")  	
}



