#ifndef MITKITKIMAGETOIMAGEFILTERADAPTER_TXX
#define MITKITKIMAGETOIMAGEFILTERADAPTER_TXX

#include "mitkItkImageToImageFilterAdapter.h"
#include "mitkImageToItkMultiplexer.h"
#include "mitkImage.h"
#include "vtkTransform.h"

namespace mitk
{

template <typename TPixel>
ItkImageToImageFilterAdapter< TPixel>::ItkImageToImageFilterAdapter()
: m_FirstFilter(NULL), m_LastFilter(NULL)
{
}

template <typename TPixel>
ItkImageToImageFilterAdapter< TPixel>::~ItkImageToImageFilterAdapter()
{
}

/**
 * \todo check if this is no conflict to the ITK filter writing rules -> ITK SoftwareGuide p.512
 */  
template <typename TPixel>
void ItkImageToImageFilterAdapter< TPixel>::GenerateOutputInformation()
{
  itkDebugMacro(<<"GenerateOutputInformation()");
}

template <typename TPixel>
void ItkImageToImageFilterAdapter< TPixel>::GenerateData()
{
  mitk::Image::Pointer outputImage = this->GetOutput();
  typename ImageToImageFilter::InputImageConstPointer inputImage = this->GetInput();
  if(inputImage.IsNull())
  {
    outputImage = NULL;
    return;
  }
  /* Check if there is an input Filter */
  if(m_FirstFilter.IsNull() || m_LastFilter.IsNull())
  {
    outputImage = const_cast<mitk::Image*>(inputImage.GetPointer());
    return;
  }
  /* convert input mitk image to itk image */
  mitk::Image::Pointer inputImageMitk = const_cast<mitk::Image*>(inputImage.GetPointer());  // const away cast, because FixedInput...Multiplexer Macro needs non const Pointer
  typename ItkImageType::Pointer itkImage = ItkImageType::New();
  //FixedInputDimensionMitkToItkFunctionMultiplexer(itkImage, ItkImageType , inputImageMitk, 3, MakeCastImageFilter);    
  CastToItkImage(inputImageMitk, itkImage);
  /* check if image conversion failed */
  if (itkImage.IsNull())
  {
    std::cout << " can't convert input image to itk" << std::endl;
    outputImage = const_cast<mitk::Image*>(inputImage.GetPointer());
    return;
  }
  //m_ItkImageToImageFilter->SetInput(itkImage);
  m_FirstFilter->SetInput(itkImage);
  try
  {
    //m_ItkImageToImageFilter->Update();
    m_LastFilter->UpdateLargestPossibleRegion();
  }
  catch( itk::ExceptionObject & excep )
  {
    std::cerr << "mitk::ItkImageToImageFilterAdapter: Exception while executing ITK filter" << std::endl;
    std::cerr << excep << std::endl;
  }
  /* convert the itk image back to an mitk image and set it as output for this filter */
  //outputImage->InitializeByItk(m_LastFilter->GetOutput());
  //outputImage->SetVolume(m_LastFilter->GetOutput()->GetBufferPointer());
  //ItkImageType::Pointer itkOutputImage = m_LastFilter->GetOutput();
  //CastToMitkImage(itkOutputImage, outputImage);
  CastToMitkImage(m_LastFilter->GetOutput(), outputImage);
  /* copy the transform from source to result image */
  outputImage->GetGeometry()->GetVtkTransform()->SetMatrix(inputImage->GetGeometry()->GetVtkTransform()->GetMatrix());
}

template <typename TPixel>
void ItkImageToImageFilterAdapter<TPixel>::SetSingleFilter(typename ImageToImageFilterType::Pointer filter)
{
  SetFirstFilter(filter);
  SetLastFilter(filter);
}
}
#endif // of MITKITKIMAGETOIMAGEFILTERADAPTER_TXX
