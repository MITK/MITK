#include "mitkModelSignalImageGenerator.h"
#include "itkMultiOutputNaryFunctorImageFilter.h"
#include "mitkArbitraryTimeGeometry.h"
#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"
#include "mitkITKImageImport.h"
#include "mitkModelDataGenerationFunctor.h"
#include "mitkSimpleFunctorPolicy.h"


void mitk::ModelSignalImageGenerator::SetParameterInputImage(const ParametersIndexType parameterIndex, ParameterImageType parameterImage)
{
    m_ParameterInputMap.insert(std::make_pair(parameterIndex,parameterImage));

}

mitk::ModelSignalImageGenerator::ResultImageType mitk::ModelSignalImageGenerator::GetGeneratedImage()
{
    Generate();
    return m_ResultImage;
}

template <typename TPixel, unsigned int VDim>
void
  mitk::ModelSignalImageGenerator::DoPrepareMask(itk::Image<TPixel, VDim>* image)
{
  m_InternalMask = dynamic_cast<InternalMaskType*>(image);

  if (m_InternalMask.IsNull())
  {
    MITK_INFO << "Parameter Fit Generator. Need to cast mask for parameter fit.";
    typedef itk::Image<TPixel, VDim> InputImageType;
    typedef itk::CastImageFilter< InputImageType, InternalMaskType > CastFilterType;
    typename CastFilterType::Pointer  spImageCaster =  CastFilterType::New();

    spImageCaster->SetInput(image);

    m_InternalMask = spImageCaster->GetOutput();
    spImageCaster->Update();
  }
}

void mitk::ModelSignalImageGenerator::SortParameterImages()
{
    ParameterVectorType inputImages(this->m_ParameterInputMap.size());

    unsigned int i = 0;
    for (ParameterMapType::const_iterator pos = m_ParameterInputMap.begin(); pos != m_ParameterInputMap.end(); ++pos)
    {

      i = pos->first;
      inputImages[i] = pos->second;
    }

    this->m_InputParameterImages = inputImages;


}

void mitk::ModelSignalImageGenerator::Generate()
{
     SortParameterImages();

    if(this->m_Mask.IsNotNull())
    {
      AccessFixedDimensionByItk(m_Mask, mitk::ModelSignalImageGenerator::DoPrepareMask, 3);
    }
    else
    {
      this->m_InternalMask = NULL;
    }

    /** @todo #1 This did not work! The Access-Routine would be a much nicer solution, but for some reasons, the handling did not work.
     * Thats why the code for Generation the Data was pasted below
     */
//    mitk::Image::Pointer firstParameterImage = this->m_InputParameterImages[0];
//    AccessFixedDimensionByItk(firstParameterImage, mitk::ModelSignalImageGenerator::DoGenerateData, 3);


    typedef itk::Image<double, 3> InputFrameImageType;
    typedef itk::Image<double, 3> OutputImageType;

    typedef itk::MultiOutputNaryFunctorImageFilter<InputFrameImageType, OutputImageType, SimpleFunctorPolicy, InternalMaskType> FilterType;
     FilterType::Pointer filter = FilterType::New();

    for(unsigned int i=0; i<this->m_ParameterInputMap.size(); ++i)
    {
        InputFrameImageType::Pointer frameImage = InputFrameImageType::New();
        Image::Pointer parameterImage = m_InputParameterImages.at(i);

        mitk::CastToItkImage(parameterImage, frameImage);
        filter->SetInput(i,frameImage);
    }

    ModelDataGenerationFunctor::Pointer generationFunctor = ModelDataGenerationFunctor::New();
    generationFunctor->SetModelParameterizer(m_Parameterizer);

    SimpleFunctorPolicy functor;
    functor.SetFunctor(generationFunctor);
    filter->SetFunctor(functor);
    if (this->m_InternalMask.IsNotNull())
    {
     filter->SetMask(this->m_InternalMask);
    }
    filter->Update();

    if (filter->GetNumberOfOutputs() != generationFunctor->GetGrid().GetSize())
    {
      itkExceptionMacro("Error. Number of computed output Images does not match Grid size!");
    }

    /** @todo #1 Better solution than all this code!
     * This was copied from TestingHelper/TestArtifactGenerator. Just instantiating a mitk::Image and setting its Volumes
     * in the for-loop did not work somehow.
     * This was a work around
     */

    typedef itk::Image<double,4> DynamicITKImageType;

    Image::Pointer dynamicImage= Image::New();
    mitk::Image::Pointer tempImage = mitk::ImportItkImage(filter->GetOutput(0))->Clone();

    DynamicITKImageType::Pointer dynamicITKImage = DynamicITKImageType::New();
    DynamicITKImageType::RegionType dynamicITKRegion;
    DynamicITKImageType::PointType dynamicITKOrigin;
    DynamicITKImageType::IndexType dynamicITKIndex;
    DynamicITKImageType::SpacingType dynamicITKSpacing;

    dynamicITKSpacing[0] = tempImage->GetGeometry()->GetSpacing()[0];
    dynamicITKSpacing[1] = tempImage->GetGeometry()->GetSpacing()[1];
    dynamicITKSpacing[2] = tempImage->GetGeometry()->GetSpacing()[2];
    dynamicITKSpacing[3] = 3.0;

    dynamicITKIndex[0] = 0;  // The first pixel of the REGION
    dynamicITKIndex[1] = 0;
    dynamicITKIndex[2] = 0;
    dynamicITKIndex[3] = 0;

    dynamicITKRegion.SetSize( 0,tempImage->GetDimension(0));
    dynamicITKRegion.SetSize( 1,tempImage->GetDimension(1));
    dynamicITKRegion.SetSize( 2,tempImage->GetDimension(2));
    dynamicITKRegion.SetSize(3, filter->GetNumberOfOutputs());

    dynamicITKRegion.SetIndex( dynamicITKIndex );

    dynamicITKOrigin[0]=tempImage->GetGeometry()->GetOrigin()[0];
    dynamicITKOrigin[1]=tempImage->GetGeometry()->GetOrigin()[1];
    dynamicITKOrigin[2]=tempImage->GetGeometry()->GetOrigin()[2];

    dynamicITKImage->SetOrigin(dynamicITKOrigin);
    dynamicITKImage->SetSpacing(dynamicITKSpacing);
    dynamicITKImage->SetRegions( dynamicITKRegion);
    dynamicITKImage->Allocate();
    dynamicITKImage->FillBuffer(0); //not sure if this is necessary

    // Convert
    mitk::CastToMitkImage(dynamicITKImage, dynamicImage);



    ArbitraryTimeGeometry::Pointer timeGeometry = ArbitraryTimeGeometry::New();
    timeGeometry->ClearAllGeometries();

    auto nrOfOutputs = filter->GetNumberOfOutputs();
    auto grid = generationFunctor->GetGrid();
    for (unsigned int i = 0; i<nrOfOutputs; ++i)
    {
      mitk::Image::Pointer frame = mitk::ImportItkImage(filter->GetOutput(i))->Clone();
      mitk::ImageReadAccessor accessor(frame);
      dynamicImage->SetVolume(accessor.GetData(), i);

      double tmax = 0;
      if (i<(nrOfOutputs - 1))
      {
        tmax = grid[i + 1] * 1000;
      }
      else
      {
        tmax = grid[i] * 1000;
      }
      timeGeometry->AppendNewTimeStepClone(frame->GetGeometry(), grid[i] * 1000, tmax);
     }

    dynamicImage->SetTimeGeometry(timeGeometry);

    this->m_ResultImage = dynamicImage->Clone();

}
