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
#ifndef mitkCLPolyToNrrd_cpp
#define mitkCLPolyToNrrd_cpp

#include "time.h"
#include <sstream>
#include <fstream>

#include <mitkIOUtil.h>
#include "mitkCommandLineParser.h"

#include <mitkSplitParameterToVector.h>
#include <mitkGlobalImageFeaturesParameter.h>

#include <mitkGIFCooccurenceMatrix.h>
#include <mitkGIFCooccurenceMatrix2.h>
#include <mitkGIFGreyLevelRunLength.h>
#include <mitkGIFFirstOrderStatistics.h>
#include <mitkGIFFirstOrderHistogramStatistics.h>
#include <mitkGIFFirstOrderNumericStatistics.h>
#include <mitkGIFVolumetricStatistics.h>
#include <mitkGIFVolumetricDensityStatistics.h>
#include <mitkGIFGreyLevelSizeZone.h>
#include <mitkGIFGreyLevelDistanceZone.h>
#include <mitkGIFImageDescriptionFeatures.h>
#include <mitkGIFLocalIntensity.h>
#include <mitkGIFCurvatureStatistic.h>
#include <mitkGIFIntensityVolumeHistogramFeatures.h>
#include <mitkGIFNeighbourhoodGreyToneDifferenceFeatures.h>
#include <mitkGIFNeighbouringGreyLevelDependenceFeatures.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>
#include <mitkConvert2Dto3DImageFilter.h>

#include <mitkCLResultWritter.h>
#include <mitkVersion.h>

#include <iostream>
#include <locale>

#include <itkImageDuplicator.h>
#include <itkImageRegionIterator.h>


#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkResampleImageFilter.h"

#include <QApplication>
#include <mitkStandaloneDataStorage.h>
#include "QmitkRegisterClasses.h"
#include "QmitkRenderWindow.h"
#include "vtkRenderLargeImage.h"
#include "vtkPNGWriter.h"



typedef itk::Image< double, 3 >                 FloatImageType;
typedef itk::Image< unsigned short, 3 >          MaskImageType;

template <class charT>
class punct_facet : public std::numpunct<charT> {
public:
  punct_facet(charT sep) :
    m_Sep(sep)
  {

  }
protected:
  charT do_decimal_point() const { return m_Sep; }
private:
  charT m_Sep;
};

template<typename TPixel, unsigned int VImageDimension>
void
ResampleImage(itk::Image<TPixel, VImageDimension>* itkImage, float resolution, mitk::Image::Pointer& newImage)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::ResampleImageFilter<ImageType, ImageType> ResampleFilterType;

  typename ResampleFilterType::Pointer resampler = ResampleFilterType::New();
  auto spacing = itkImage->GetSpacing();
  auto size = itkImage->GetLargestPossibleRegion().GetSize();

  for (unsigned int i = 0; i < VImageDimension; ++i)
  {
    size[i] = size[i] / (1.0*resolution)*(1.0*spacing[i])+1.0;
  }
  spacing.Fill(resolution);

  resampler->SetInput(itkImage);
  resampler->SetSize(size);
  resampler->SetOutputSpacing(spacing);
  resampler->SetOutputOrigin(itkImage->GetOrigin());
  resampler->SetOutputDirection(itkImage->GetDirection());
  resampler->Update();

  newImage->InitializeByItk(resampler->GetOutput());
  mitk::GrabItkImageMemory(resampler->GetOutput(), newImage);
}


template<typename TPixel, unsigned int VImageDimension>
static void
CreateNoNaNMask(itk::Image<TPixel, VImageDimension>* itkValue, mitk::Image::Pointer mask, mitk::Image::Pointer& newMask)
{
  typedef itk::Image< TPixel, VImageDimension>                 LFloatImageType;
  typedef itk::Image< unsigned short, VImageDimension>          LMaskImageType;
  typename LMaskImageType::Pointer itkMask = LMaskImageType::New();

  mitk::CastToItkImage(mask, itkMask);

  typedef itk::ImageDuplicator< LMaskImageType > DuplicatorType;
  typename DuplicatorType::Pointer duplicator = DuplicatorType::New();
  duplicator->SetInputImage(itkMask);
  duplicator->Update();

  auto tmpMask = duplicator->GetOutput();

  itk::ImageRegionIterator<LMaskImageType> mask1Iter(itkMask, itkMask->GetLargestPossibleRegion());
  itk::ImageRegionIterator<LMaskImageType> mask2Iter(tmpMask, tmpMask->GetLargestPossibleRegion());
  itk::ImageRegionIterator<LFloatImageType> imageIter(itkValue, itkValue->GetLargestPossibleRegion());
  while (!mask1Iter.IsAtEnd())
  {
    mask2Iter.Set(0);
    if (mask1Iter.Value() > 0)
    {
      // Is not NaN
      if (imageIter.Value() == imageIter.Value())
      {
        mask2Iter.Set(1);
      }
    }
    ++mask1Iter;
    ++mask2Iter;
    ++imageIter;
  }

  newMask->InitializeByItk(tmpMask);
  mitk::GrabItkImageMemory(tmpMask, newMask);
}

template<typename TPixel, unsigned int VImageDimension>
static void
ResampleMask(itk::Image<TPixel, VImageDimension>* itkMoving, mitk::Image::Pointer ref, mitk::Image::Pointer& newMask)
{
  typedef itk::Image< TPixel, VImageDimension>          LMaskImageType;
  typedef itk::NearestNeighborInterpolateImageFunction< LMaskImageType> NearestNeighborInterpolateImageFunctionType;
  typedef itk::ResampleImageFilter<LMaskImageType, LMaskImageType> ResampleFilterType;

  typename NearestNeighborInterpolateImageFunctionType::Pointer nn_interpolator = NearestNeighborInterpolateImageFunctionType::New();
  typename LMaskImageType::Pointer itkRef = LMaskImageType::New();
  mitk::CastToItkImage(ref, itkRef);


  typename ResampleFilterType::Pointer resampler = ResampleFilterType::New();
  resampler->SetInput(itkMoving);
  resampler->SetReferenceImage(itkRef);
  resampler->UseReferenceImageOn();
  resampler->SetInterpolator(nn_interpolator);
  resampler->Update();

  newMask->InitializeByItk(resampler->GetOutput());
  mitk::GrabItkImageMemory(resampler->GetOutput(), newMask);
}

static void
ExtractSlicesFromImages(mitk::Image::Pointer image, mitk::Image::Pointer mask,
                        mitk::Image::Pointer maskNoNaN, mitk::Image::Pointer morphMask,
                        int direction,
                        std::vector<mitk::Image::Pointer> &imageVector,
                        std::vector<mitk::Image::Pointer> &maskVector,
                        std::vector<mitk::Image::Pointer> &maskNoNaNVector,
                        std::vector<mitk::Image::Pointer> &morphMaskVector)
{
  typedef itk::Image< double, 2 >                 FloatImage2DType;
  typedef itk::Image< unsigned short, 2 >          MaskImage2DType;

  FloatImageType::Pointer itkFloat = FloatImageType::New();
  MaskImageType::Pointer itkMask = MaskImageType::New();
  MaskImageType::Pointer itkMaskNoNaN = MaskImageType::New();
  MaskImageType::Pointer itkMorphMask = MaskImageType::New();
  mitk::CastToItkImage(mask, itkMask);
  mitk::CastToItkImage(maskNoNaN, itkMaskNoNaN);
  mitk::CastToItkImage(image, itkFloat);
  mitk::CastToItkImage(morphMask, itkMorphMask);

  int idxA, idxB, idxC;
  switch (direction)
  {
  case 0:
    idxA = 1; idxB = 2; idxC = 0;
    break;
  case 1:
    idxA = 0; idxB = 2; idxC = 1;
    break;
  case 2:
    idxA = 0; idxB = 1; idxC = 2;
    break;
  default:
    idxA = 1; idxB = 2; idxC = 0;
    break;
  }

  auto imageSize = image->GetLargestPossibleRegion().GetSize();
  FloatImageType::IndexType index3D;
  FloatImage2DType::IndexType index2D;
  FloatImage2DType::SpacingType spacing2D;
  spacing2D[0] = itkFloat->GetSpacing()[idxA];
  spacing2D[1] = itkFloat->GetSpacing()[idxB];

  for (unsigned int i = 0; i < imageSize[idxC]; ++i)
  {
    FloatImage2DType::RegionType region;
    FloatImage2DType::IndexType start;
    FloatImage2DType::SizeType size;
    start[0] = 0; start[1] = 0;
    size[0] = imageSize[idxA];
    size[1] = imageSize[idxB];
    region.SetIndex(start);
    region.SetSize(size);

    FloatImage2DType::Pointer image2D = FloatImage2DType::New();
    image2D->SetRegions(region);
    image2D->Allocate();

    MaskImage2DType::Pointer mask2D = MaskImage2DType::New();
    mask2D->SetRegions(region);
    mask2D->Allocate();

    MaskImage2DType::Pointer masnNoNaN2D = MaskImage2DType::New();
    masnNoNaN2D->SetRegions(region);
    masnNoNaN2D->Allocate();

    MaskImage2DType::Pointer morph2D = MaskImage2DType::New();
    morph2D->SetRegions(region);
    morph2D->Allocate();


    unsigned long voxelsInMask = 0;

    for (unsigned int a = 0; a < imageSize[idxA]; ++a)
    {
      for (unsigned int b = 0; b < imageSize[idxB]; ++b)
      {
        index3D[idxA] = a;
        index3D[idxB] = b;
        index3D[idxC] = i;
        index2D[0] = a;
        index2D[1] = b;
        image2D->SetPixel(index2D, itkFloat->GetPixel(index3D));
        mask2D->SetPixel(index2D, itkMask->GetPixel(index3D));
        masnNoNaN2D->SetPixel(index2D, itkMaskNoNaN->GetPixel(index3D));
        morph2D->SetPixel(index2D, itkMorphMask->GetPixel(index3D));
        voxelsInMask += (itkMask->GetPixel(index3D) > 0) ? 1 : 0;

      }
    }

    image2D->SetSpacing(spacing2D);
    mask2D->SetSpacing(spacing2D);
    masnNoNaN2D->SetSpacing(spacing2D);
    morph2D->SetSpacing(spacing2D);

    mitk::Image::Pointer tmpFloatImage = mitk::Image::New();
    tmpFloatImage->InitializeByItk(image2D.GetPointer());
    mitk::GrabItkImageMemory(image2D, tmpFloatImage);

    mitk::Image::Pointer tmpMaskImage = mitk::Image::New();
    tmpMaskImage->InitializeByItk(mask2D.GetPointer());
    mitk::GrabItkImageMemory(mask2D, tmpMaskImage);

    mitk::Image::Pointer tmpMaskNoNaNImage = mitk::Image::New();
    tmpMaskNoNaNImage->InitializeByItk(masnNoNaN2D.GetPointer());
    mitk::GrabItkImageMemory(masnNoNaN2D, tmpMaskNoNaNImage);

    mitk::Image::Pointer tmpMorphMaskImage = mitk::Image::New();
    tmpMorphMaskImage->InitializeByItk(morph2D.GetPointer());
    mitk::GrabItkImageMemory(morph2D, tmpMorphMaskImage);

    if (voxelsInMask > 0)
    {
      imageVector.push_back(tmpFloatImage);
      maskVector.push_back(tmpMaskImage);
      maskNoNaNVector.push_back(tmpMaskNoNaNImage);
      morphMaskVector.push_back(tmpMorphMaskImage);
    }
  }
}

static
void SaveSliceOrImageAsPNG(mitk::Image::Pointer image, mitk::Image::Pointer mask, std::string path, int index)
{
  // Create a Standalone Datastorage for the single purpose of saving screenshots..
  mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
  QmitkRenderWindow renderWindow;
  renderWindow.GetRenderer()->SetDataStorage(ds);

  auto nodeI = mitk::DataNode::New();
  nodeI->SetData(image);
  auto nodeM = mitk::DataNode::New();
  nodeM->SetData(mask);
  ds->Add(nodeI);
  ds->Add(nodeM);

  auto geo = ds->ComputeBoundingGeometry3D(ds->GetAll());
  mitk::RenderingManager::GetInstance()->InitializeViews(geo);

  mitk::SliceNavigationController::Pointer sliceNaviController = renderWindow.GetSliceNavigationController();
  unsigned int numberOfSteps = 1;
  if (sliceNaviController)
  {
    numberOfSteps = sliceNaviController->GetSlice()->GetSteps();
    sliceNaviController->GetSlice()->SetPos(0);
  }

  renderWindow.show();
  renderWindow.resize(256, 256);

  for (unsigned int currentStep = 0; currentStep < numberOfSteps; ++currentStep)
  {
    if (sliceNaviController)
    {
      sliceNaviController->GetSlice()->SetPos(currentStep);
    }

    renderWindow.GetRenderer()->PrepareRender();

    vtkRenderWindow* renderWindow2 = renderWindow.GetVtkRenderWindow();
    mitk::BaseRenderer* baserenderer = mitk::BaseRenderer::GetInstance(renderWindow2);
    auto vtkRender = baserenderer->GetVtkRenderer();
    vtkRender->GetRenderWindow()->WaitForCompletion();

    vtkRenderLargeImage* magnifier = vtkRenderLargeImage::New();
    magnifier->SetInput(vtkRender);
    magnifier->SetMagnification(3.0);

    std::stringstream ss;
    ss << path << "_Idx-" << index << "_Step-"<<currentStep<<".png";
    std::string tmpImageName;
    ss >> tmpImageName;
    auto fileWriter = vtkPNGWriter::New();
    fileWriter->SetInputConnection(magnifier->GetOutputPort());
    fileWriter->SetFileName(tmpImageName.c_str());
    fileWriter->Write();
    fileWriter->Delete();
  }
}

int main(int argc, char* argv[])
{
  // Commented : Updated to a common interface, include, if possible, mask is type unsigned short, uses Quantification, Comments
  //                                 Name follows standard scheme with Class Name::Feature Name
  // Commented 2: Updated to use automatic inclusion of list of parameters if required.
  mitk::GIFImageDescriptionFeatures::Pointer ipCalculator = mitk::GIFImageDescriptionFeatures::New(); // Commented 2, Tested
  mitk::GIFFirstOrderStatistics::Pointer firstOrderCalculator = mitk::GIFFirstOrderStatistics::New(); //Commented 2
  mitk::GIFFirstOrderHistogramStatistics::Pointer firstOrderHistoCalculator = mitk::GIFFirstOrderHistogramStatistics::New(); // Commented 2, Tested
  mitk::GIFFirstOrderNumericStatistics::Pointer firstOrderNumericCalculator = mitk::GIFFirstOrderNumericStatistics::New(); // Commented 2, Tested
  mitk::GIFVolumetricStatistics::Pointer volCalculator = mitk::GIFVolumetricStatistics::New();   // Commented 2, Tested
  mitk::GIFVolumetricDensityStatistics::Pointer voldenCalculator = mitk::GIFVolumetricDensityStatistics::New(); // Commented 2, Tested
  mitk::GIFCooccurenceMatrix::Pointer coocCalculator = mitk::GIFCooccurenceMatrix::New(); // Commented 2, Will not be tested
  mitk::GIFCooccurenceMatrix2::Pointer cooc2Calculator = mitk::GIFCooccurenceMatrix2::New(); //Commented 2
  mitk::GIFNeighbouringGreyLevelDependenceFeature::Pointer ngldCalculator = mitk::GIFNeighbouringGreyLevelDependenceFeature::New(); //Commented 2, Tested
  mitk::GIFGreyLevelRunLength::Pointer rlCalculator = mitk::GIFGreyLevelRunLength::New(); // Commented 2
  mitk::GIFGreyLevelSizeZone::Pointer glszCalculator = mitk::GIFGreyLevelSizeZone::New(); // Commented 2, Tested
  mitk::GIFGreyLevelDistanceZone::Pointer gldzCalculator = mitk::GIFGreyLevelDistanceZone::New(); //Commented 2, Tested
  mitk::GIFLocalIntensity::Pointer lociCalculator = mitk::GIFLocalIntensity::New(); //Commented 2, Tested
  mitk::GIFIntensityVolumeHistogramFeatures::Pointer ivohCalculator = mitk::GIFIntensityVolumeHistogramFeatures::New(); // Commented 2
  mitk::GIFNeighbourhoodGreyToneDifferenceFeatures::Pointer ngtdCalculator = mitk::GIFNeighbourhoodGreyToneDifferenceFeatures::New(); //Commented 2, Tested
  mitk::GIFCurvatureStatistic::Pointer curvCalculator = mitk::GIFCurvatureStatistic::New(); //Commented 2, Tested

  std::vector<mitk::AbstractGlobalImageFeature::Pointer> features;
  features.push_back(volCalculator.GetPointer());
  features.push_back(voldenCalculator.GetPointer());
  features.push_back(curvCalculator.GetPointer());
  features.push_back(firstOrderCalculator.GetPointer());
  features.push_back(firstOrderNumericCalculator.GetPointer());
  features.push_back(firstOrderHistoCalculator.GetPointer());
  features.push_back(ivohCalculator.GetPointer());
  features.push_back(lociCalculator.GetPointer());
  features.push_back(coocCalculator.GetPointer());
  features.push_back(cooc2Calculator.GetPointer());
  features.push_back(ngldCalculator.GetPointer());
  features.push_back(rlCalculator.GetPointer());
  features.push_back(glszCalculator.GetPointer());
  features.push_back(gldzCalculator.GetPointer());
  features.push_back(ipCalculator.GetPointer());
  features.push_back(ngtdCalculator.GetPointer());

  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  mitk::cl::GlobalImageFeaturesParameter param;
  param.AddParameter(parser);

  parser.addArgument("--","-", mitkCommandLineParser::String, "---", "---", us::Any(),true);
  for (auto cFeature : features)
  {
    cFeature->AddArguments(parser);
  }

  parser.addArgument("--", "-", mitkCommandLineParser::String, "---", "---", us::Any(), true);
  parser.addArgument("description","d",mitkCommandLineParser::String,"Text","Description that is added to the output",us::Any());
  parser.addArgument("direction", "dir", mitkCommandLineParser::String, "Int", "Allows to specify the direction for Cooc and RL. 0: All directions, 1: Only single direction (Test purpose), 2,3,4... Without dimension 0,1,2... ", us::Any());
  parser.addArgument("slice-wise", "slice", mitkCommandLineParser::String, "Int", "Allows to specify if the image is processed slice-wise (number giving direction) ", us::Any());
  parser.addArgument("output-mode", "omode", mitkCommandLineParser::Int, "Int", "Defines if the results of an image / slice are written in a single row (0 , default) or column (1).");

  // Miniapp Infos
  parser.setCategory("Classification Tools");
  parser.setTitle("Global Image Feature calculator");
  parser.setDescription("Calculates different global statistics for a given segmentation / image combination");
  parser.setContributor("MBI");

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  param.ParseParameter(parsedArgs);

  if (parsedArgs.size()==0)
  {
    return EXIT_FAILURE;
  }
  if ( parsedArgs.count("help") || parsedArgs.count("h"))
  {
    return EXIT_SUCCESS;
  }

  //bool savePNGofSlices = true;
  //std::string folderForPNGOfSlices = "E:\\tmp\\bonekamp\\fig\\";

  std::string version = "Version: 1.22";
  MITK_INFO << version;

  std::ofstream log;
  if (param.useLogfile)
  {
    log.open(param.logfilePath, std::ios::app);
    log << std::endl;
    log << version;
    log << "Image: " << param.imagePath;
    log << "Mask: " << param.maskPath;
  }


  if (param.useDecimalPoint)
  {
    std::cout.imbue(std::locale(std::cout.getloc(), new punct_facet<char>(param.decimalPoint)));
  }

  mitk::Image::Pointer image;
  mitk::Image::Pointer mask;

  mitk::Image::Pointer tmpImage = mitk::IOUtil::Load<mitk::Image>(param.imagePath);
  mitk::Image::Pointer tmpMask = mitk::IOUtil::Load<mitk::Image>(param.maskPath);
  image = tmpImage;
  mask = tmpMask;

  mitk::Image::Pointer morphMask = mask;
  if (param.useMorphMask)
  {
    morphMask = mitk::IOUtil::Load<mitk::Image>(param.morphPath);
  }

  log << " Check for Dimensions -";
  if ((image->GetDimension() != mask->GetDimension()))
  {
    MITK_INFO << "Dimension of image does not match. ";
    MITK_INFO << "Correct one image, may affect the result";
    if (image->GetDimension() == 2)
    {
      mitk::Convert2Dto3DImageFilter::Pointer multiFilter2 = mitk::Convert2Dto3DImageFilter::New();
      multiFilter2->SetInput(tmpImage);
      multiFilter2->Update();
      image = multiFilter2->GetOutput();
    }
    if (mask->GetDimension() == 2)
    {
      mitk::Convert2Dto3DImageFilter::Pointer multiFilter3 = mitk::Convert2Dto3DImageFilter::New();
      multiFilter3->SetInput(tmpMask);
      multiFilter3->Update();
      mask = multiFilter3->GetOutput();
    }
  }

  int writeDirection = 0;
  if (parsedArgs.count("output-mode"))
  {
    writeDirection = us::any_cast<int>(parsedArgs["output-mode"]);
  }

  log << " Check for Resolution -";
  if (param.resampleToFixIsotropic)
  {
    mitk::Image::Pointer newImage = mitk::Image::New();
    AccessByItk_2(image, ResampleImage, param.resampleResolution, newImage);
    image = newImage;
  }
  if ( ! mitk::Equal(mask->GetGeometry(0)->GetOrigin(), image->GetGeometry(0)->GetOrigin()))
  {
    MITK_INFO << "Not equal Origins";
    if (param.ensureSameSpace)
    {
      MITK_INFO << "Warning!";
      MITK_INFO << "The origin of the input image and the mask do not match. They are";
      MITK_INFO << "now corrected. Please check to make sure that the images still match";
      image->GetGeometry(0)->SetOrigin(mask->GetGeometry(0)->GetOrigin());
    } else
    {
      return -1;
    }
  }

  log << " Resample if required -";
  if (param.resampleMask)
  {
    mitk::Image::Pointer newMaskImage = mitk::Image::New();
    AccessByItk_2(mask, ResampleMask, image, newMaskImage);
    mask = newMaskImage;
  }

  log << " Check for Equality -";
  if ( ! mitk::Equal(mask->GetGeometry(0)->GetSpacing(), image->GetGeometry(0)->GetSpacing()))
  {
    MITK_INFO << "Not equal Spacing";
    if (param.ensureSameSpace)
    {
      MITK_INFO << "Warning!";
      MITK_INFO << "The spacing of the mask was set to match the spacing of the input image.";
      MITK_INFO << "This might cause unintended spacing of the mask image";
      image->GetGeometry(0)->SetSpacing(mask->GetGeometry(0)->GetSpacing());
    } else
    {
      MITK_INFO << "The spacing of the mask and the input images is not equal.";
      MITK_INFO << "Terminating the programm. You may use the '-fi' option";
      return -1;
    }
  }

  int direction = 0;
  if (parsedArgs.count("direction"))
  {
    direction = mitk::cl::splitDouble(parsedArgs["direction"].ToString(), ';')[0];
  }

  MITK_INFO << "Start creating Mask without NaN";

  mitk::Image::Pointer maskNoNaN = mitk::Image::New();
  AccessByItk_2(image, CreateNoNaNMask,  mask, maskNoNaN);
  //CreateNoNaNMask(mask, image, maskNoNaN);


  bool sliceWise = false;
  int sliceDirection = 0;
  unsigned int currentSlice = 0;
  bool imageToProcess = true;

  std::vector<mitk::Image::Pointer> floatVector;
  std::vector<mitk::Image::Pointer> maskVector;
  std::vector<mitk::Image::Pointer> maskNoNaNVector;
  std::vector<mitk::Image::Pointer> morphMaskVector;

  if ((parsedArgs.count("slice-wise")) && image->GetDimension() > 2)
  {
    MITK_INFO << "Enabled slice-wise";
    sliceWise = true;
    sliceDirection = mitk::cl::splitDouble(parsedArgs["slice-wise"].ToString(), ';')[0];
    MITK_INFO << sliceDirection;
    ExtractSlicesFromImages(image, mask, maskNoNaN, morphMask, sliceDirection, floatVector, maskVector, maskNoNaNVector, morphMaskVector);
    MITK_INFO << "Slice";
  }

  log << " Configure features -";
  for (auto cFeature : features)
  {
    if (param.defineGlobalMinimumIntensity)
    {
      cFeature->SetMinimumIntensity(param.globalMinimumIntensity);
      cFeature->SetUseMinimumIntensity(true);
    }
    if (param.defineGlobalMaximumIntensity)
    {
      cFeature->SetMaximumIntensity(param.globalMaximumIntensity);
      cFeature->SetUseMaximumIntensity(true);
    }
    if (param.defineGlobalNumberOfBins)
    {
      cFeature->SetBins(param.globalNumberOfBins);
      MITK_INFO << param.globalNumberOfBins;
    }
    cFeature->SetParameter(parsedArgs);
    cFeature->SetDirection(direction);
    cFeature->SetEncodeParameters(param.encodeParameter);
  }

  bool addDescription = parsedArgs.count("description");
  mitk::cl::FeatureResultWritter writer(param.outputPath, writeDirection);

  if (param.useDecimalPoint)
  {
    writer.SetDecimalPoint(param.decimalPoint);
  }

  std::string description = "";
  if (addDescription)
  {
    description = parsedArgs["description"].ToString();
  }

  mitk::Image::Pointer cImage = image;
  mitk::Image::Pointer cMask = mask;
  mitk::Image::Pointer cMaskNoNaN = maskNoNaN;
  mitk::Image::Pointer cMorphMask = morphMask;

  if (param.useHeader)
  {
    writer.AddColumn("SoftwareVersion");
    writer.AddColumn("Patient");
    writer.AddColumn("Image");
    writer.AddColumn("Segmentation");
  }

  // Create a QTApplication and a Datastorage
  // This is necessary in order to save screenshots of
  // each image / slice.
  QApplication qtapplication(argc, argv);
  QmitkRegisterClasses();

  std::vector<mitk::AbstractGlobalImageFeature::FeatureListType> allStats;

  log << " Begin Processing -";
  while (imageToProcess)
  {
    if (sliceWise)
    {
      cImage = floatVector[currentSlice];
      cMask = maskVector[currentSlice];
      cMaskNoNaN = maskNoNaNVector[currentSlice];
      cMorphMask = morphMaskVector[currentSlice];
      imageToProcess = (floatVector.size()-1 > (currentSlice)) ? true : false ;
    }
    else
    {
      imageToProcess = false;
    }

    if (param.writePNGScreenshots)
    {
      SaveSliceOrImageAsPNG(cImage, cMask, param.pngScreenshotsPath, currentSlice);
    }
    if (param.writeAnalysisImage)
    {
      mitk::IOUtil::Save(cImage, param.anaylsisImagePath);
    }
    if (param.writeAnalysisMask)
    {
      mitk::IOUtil::Save(cMask, param.analysisMaskPath);
    }

    mitk::AbstractGlobalImageFeature::FeatureListType stats;

    for (auto cFeature : features)
    {
      log << " Calculating " << cFeature->GetFeatureClassName() << " -";
      cFeature->SetMorphMask(cMorphMask);
      cFeature->CalculateFeaturesUsingParameters(cImage, cMask, cMaskNoNaN, stats);
    }

    for (std::size_t i = 0; i < stats.size(); ++i)
    {
      std::cout << stats[i].first << " - " << stats[i].second << std::endl;
    }

    writer.AddHeader(description, currentSlice, stats, param.useHeader, addDescription);
    if (true)
    {
      writer.AddSubjectInformation(MITK_REVISION);
      writer.AddSubjectInformation(param.imageFolder);
      writer.AddSubjectInformation(param.imageName);
      writer.AddSubjectInformation(param.maskName);
    }
    writer.AddResult(description, currentSlice, stats, param.useHeader, addDescription);

    allStats.push_back(stats);
    ++currentSlice;
  }

  log << " Process Slicewise -";
  if (sliceWise)
  {
    mitk::AbstractGlobalImageFeature::FeatureListType statMean, statStd;
    for (std::size_t i = 0; i < allStats[0].size(); ++i)
    {
      auto cElement1 = allStats[0][i];
      cElement1.first = "SliceWise Mean " + cElement1.first;
      cElement1.second = 0.0;
      auto cElement2 = allStats[0][i];
      cElement2.first = "SliceWise Var. " + cElement2.first;
      cElement2.second = 0.0;
      statMean.push_back(cElement1);
      statStd.push_back(cElement2);
    }

    for (auto cStat : allStats)
    {
      for (std::size_t i = 0; i < cStat.size(); ++i)
      {
        statMean[i].second += cStat[i].second / (1.0*allStats.size());
      }
    }

    for (auto cStat : allStats)
    {
      for (std::size_t i = 0; i < cStat.size(); ++i)
      {
        statStd[i].second += (cStat[i].second - statMean[i].second)*(cStat[i].second - statMean[i].second) / (1.0*allStats.size());
      }
    }

    for (std::size_t i = 0; i < statMean.size(); ++i)
    {
      std::cout << statMean[i].first << " - " << statMean[i].second << std::endl;
      std::cout << statStd[i].first << " - " << statStd[i].second << std::endl;
    }
    if (true)
    {
      writer.AddSubjectInformation(MITK_REVISION);
      writer.AddSubjectInformation(param.imageFolder);
      writer.AddSubjectInformation(param.imageName);
      writer.AddSubjectInformation(param.maskName + " - Mean");
    }
    writer.AddResult(description, currentSlice, statMean, param.useHeader, addDescription);
    if (true)
    {
      writer.AddSubjectInformation(MITK_REVISION);
      writer.AddSubjectInformation(param.imageFolder);
      writer.AddSubjectInformation(param.imageName);
      writer.AddSubjectInformation(param.maskName + " - Var.");
    }
    writer.AddResult(description, currentSlice, statStd, param.useHeader, addDescription);
  }

  if (param.useLogfile)
  {
    log << "Finished calculation" << std::endl;
    log.close();
  }
  return 0;
}

#endif
