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

#include <mitkGIFCooccurenceMatrix.h>
#include <mitkGIFCooccurenceMatrix2.h>
#include <mitkGIFGrayLevelRunLength.h>
#include <mitkGIFFirstOrderStatistics.h>
#include <mitkGIFVolumetricStatistics.h>
#include <mitkGIFNeighbouringGreyLevelDependenceFeatures.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>
#include <mitkITKImageImport.h>
#include <mitkConvert2Dto3DImageFilter.h>


#include <itkImageDuplicator.h>
#include <itkImageRegionIterator.h>


#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkResampleImageFilter.h"



typedef itk::Image< double, 3 >                 FloatImageType;
typedef itk::Image< unsigned char, 3 >          MaskImageType;

static std::vector<double> splitDouble(std::string str, char delimiter) {
  std::vector<double> internal;
  std::stringstream ss(str); // Turn the string into a stream.
  std::string tok;
  double val;
  while (std::getline(ss, tok, delimiter)) {
    std::stringstream s2(tok);
    s2 >> val;
    internal.push_back(val);
  }

  return internal;
}

template<typename TPixel, unsigned int VImageDimension>
void
ResampleImage(itk::Image<TPixel, VImageDimension>* itkImage, float resolution, mitk::Image::Pointer& newImage)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::ResampleImageFilter<ImageType, ImageType> ResampleFilterType;

  typename ResampleFilterType::Pointer resampler = ResampleFilterType::New();
  auto spacing = itkImage->GetSpacing();
  auto size = itkImage->GetLargestPossibleRegion().GetSize();

  for (int i = 0; i < VImageDimension; ++i)
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
  typedef itk::Image< unsigned char, VImageDimension>          LMaskImageType;
  LMaskImageType::Pointer itkMask = LMaskImageType::New();

  mitk::CastToItkImage(mask, itkMask);

  typedef itk::ImageDuplicator< LMaskImageType > DuplicatorType;
  DuplicatorType::Pointer duplicator = DuplicatorType::New();
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

  NearestNeighborInterpolateImageFunctionType::Pointer nn_interpolator = NearestNeighborInterpolateImageFunctionType::New();
  LMaskImageType::Pointer itkRef = LMaskImageType::New();
  mitk::CastToItkImage(ref, itkRef);


  ResampleFilterType::Pointer resampler = ResampleFilterType::New();
  resampler->SetInput(itkMoving);
  resampler->SetReferenceImage(itkRef);
  resampler->UseReferenceImageOn();
  resampler->SetInterpolator(nn_interpolator);
  resampler->Update();

  newMask->InitializeByItk(resampler->GetOutput());
  mitk::GrabItkImageMemory(resampler->GetOutput(), newMask);
}


static void
ExtractSlicesFromImages(mitk::Image::Pointer image, mitk::Image::Pointer mask, mitk::Image::Pointer maskNoNaN,
                        int direction,
                        std::vector<mitk::Image::Pointer> &imageVector,
                        std::vector<mitk::Image::Pointer> &maskVector,
                        std::vector<mitk::Image::Pointer> &maskNoNaNVector)
{
  typedef itk::Image< double, 2 >                 FloatImage2DType;
  typedef itk::Image< unsigned char, 2 >          MaskImage2DType;

  FloatImageType::Pointer itkFloat = FloatImageType::New();
  MaskImageType::Pointer itkMask = MaskImageType::New();
  MaskImageType::Pointer itkMaskNoNaN = MaskImageType::New();
  mitk::CastToItkImage(mask, itkMask);
  mitk::CastToItkImage(maskNoNaN, itkMaskNoNaN);
  mitk::CastToItkImage(image, itkFloat);

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

  for (int i = 0; i < imageSize[idxC]; ++i)
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

    for (int a = 0; a < imageSize[idxA]; ++a)
    {
      for (int b = 0; b < imageSize[idxB]; ++b)
      {
        index3D[idxA] = a;
        index3D[idxB] = b;
        index3D[idxC] = i;
        index2D[0] = a;
        index2D[1] = b;
        image2D->SetPixel(index2D, itkFloat->GetPixel(index3D));
        mask2D->SetPixel(index2D, itkMask->GetPixel(index3D));
        masnNoNaN2D->SetPixel(index2D, itkMaskNoNaN->GetPixel(index3D));

      }
    }

    image2D->SetSpacing(spacing2D);
    mask2D->SetSpacing(spacing2D);
    masnNoNaN2D->SetSpacing(spacing2D);

    mitk::Image::Pointer tmpFloatImage = mitk::Image::New();
    tmpFloatImage->InitializeByItk(image2D.GetPointer());
    mitk::GrabItkImageMemory(image2D, tmpFloatImage);

    mitk::Image::Pointer tmpMaskImage = mitk::Image::New();
    tmpMaskImage->InitializeByItk(mask2D.GetPointer());
    mitk::GrabItkImageMemory(mask2D, tmpMaskImage);

    mitk::Image::Pointer tmpMaskNoNaNImage = mitk::Image::New();
    tmpMaskNoNaNImage->InitializeByItk(masnNoNaN2D.GetPointer());
    mitk::GrabItkImageMemory(masnNoNaN2D, tmpMaskNoNaNImage);

    imageVector.push_back(tmpFloatImage);
    maskVector.push_back(tmpMaskImage);
    maskNoNaNVector.push_back(tmpMaskNoNaNImage);
  }
}



int main(int argc, char* argv[])
{

  mitk::GIFFirstOrderStatistics::Pointer firstOrderCalculator = mitk::GIFFirstOrderStatistics::New();
  mitk::GIFVolumetricStatistics::Pointer volCalculator = mitk::GIFVolumetricStatistics::New();
  mitk::GIFCooccurenceMatrix::Pointer coocCalculator = mitk::GIFCooccurenceMatrix::New();
  mitk::GIFCooccurenceMatrix2::Pointer cooc2Calculator = mitk::GIFCooccurenceMatrix2::New();
  mitk::GIFNeighbouringGreyLevelDependenceFeature::Pointer ngldCalculator = mitk::GIFNeighbouringGreyLevelDependenceFeature::New();
  mitk::GIFGrayLevelRunLength::Pointer rlCalculator = mitk::GIFGrayLevelRunLength::New();




  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  // required params
  parser.addArgument("image", "i", mitkCommandLineParser::InputImage, "Input Image", "Path to the input image file", us::Any(), false);
  parser.addArgument("mask", "m", mitkCommandLineParser::InputImage, "Input Mask", "Path to the mask Image that specifies the area over for the statistic (Values = 1)", us::Any(), false);
  parser.addArgument("output", "o", mitkCommandLineParser::OutputFile, "Output text file", "Path to output file. The output statistic is appended to this file.", us::Any(), false);
  parser.addArgument("--","-", mitkCommandLineParser::String, "---", "---", us::Any(),true);
  firstOrderCalculator->AddArguments(parser);   // Does not support single direction
  volCalculator->AddArguments(parser);          // Does not support single direction
  coocCalculator->AddArguments(parser);         //
  cooc2Calculator->AddArguments(parser);        // Needs parameter fixing
  ngldCalculator->AddArguments(parser);         // Needs parameter fixing
  rlCalculator->AddArguments(parser);           // Does not support single direction

  parser.addArgument("--", "-", mitkCommandLineParser::String, "---", "---", us::Any(), true);


  //parser.addArgument("cooccurence", "cooc", mitkCommandLineParser::String, "Use Co-occurence matrix", "calculates Co-occurence based features", us::Any());
  //parser.addArgument("cooccurence2", "cooc2", mitkCommandLineParser::String, "Use Co-occurence matrix", "calculates Co-occurence based features (new implementation)", us::Any());
  //parser.addArgument("ngldm", "ngldm", mitkCommandLineParser::String, "Calculate Neighbouring grey level dependence based features", "Calculate Neighbouring grey level dependence based features", us::Any());
  //parser.addArgument("volume","vol",mitkCommandLineParser::String, "Use Volume-Statistic", "calculates volume based features",us::Any());
  //parser.addArgument("run-length","rl",mitkCommandLineParser::String, "Use Co-occurence matrix", "calculates Co-occurence based features",us::Any());
  //parser.addArgument("first-order","fo",mitkCommandLineParser::String, "Use First Order Features", "calculates First order based features",us::Any());
  parser.addArgument("header","head",mitkCommandLineParser::String,"Add Header (Labels) to output","",us::Any());
  parser.addArgument("description","d",mitkCommandLineParser::String,"Text","Description that is added to the output",us::Any());
  parser.addArgument("same-space", "sp", mitkCommandLineParser::String, "Bool", "Set the spacing of all images to equal. Otherwise an error will be thrown. ", us::Any());
  parser.addArgument("resample-mask", "rm", mitkCommandLineParser::Bool, "Bool", "Resamples the mask to the resolution of the input image ", us::Any());
  parser.addArgument("save-resample-mask", "srm", mitkCommandLineParser::String, "String", "If specified the resampled mask is saved to this path (if -rm is 1)", us::Any());
  parser.addArgument("fixed-isotropic", "fi", mitkCommandLineParser::Float, "Float", "Input image resampled to fixed isotropic resolution given in mm. Should be used with resample-mask ", us::Any());
  parser.addArgument("direction", "dir", mitkCommandLineParser::String, "Int", "Allows to specify the direction for Cooc and RL. 0: All directions, 1: Only single direction (Test purpose), 2,3,4... Without dimension 0,1,2... ", us::Any());
  parser.addArgument("slice-wise", "slice", mitkCommandLineParser::String, "Int", "Allows to specify if the image is processed slice-wise (number giving direction) ", us::Any());
  parser.addArgument("minimum-intensity", "minimum", mitkCommandLineParser::String, "Float", "Minimum intensity. If set, it is overwritten by more specific intensity minima", us::Any());
  parser.addArgument("maximum-intensity", "maximum", mitkCommandLineParser::String, "Float", "Maximum intensity. If set, it is overwritten by more specific intensity maxima", us::Any());
  parser.addArgument("bins", "bins", mitkCommandLineParser::String, "Int", "Number of bins if bins are used. If set, it is overwritten by more specific bin count", us::Any());



  // Miniapp Infos
  parser.setCategory("Classification Tools");
  parser.setTitle("Global Image Feature calculator");
  parser.setDescription("Calculates different global statistics for a given segmentation / image combination");
  parser.setContributor("MBI");

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.size()==0)
  {
    return EXIT_FAILURE;
  }
  if ( parsedArgs.count("help") || parsedArgs.count("h"))
  {
    return EXIT_SUCCESS;
  }

  MITK_INFO << "Version: "<< 1.8;

  //bool useCooc = parsedArgs.count("cooccurence");

  bool resampleMask = false;
  if (parsedArgs.count("resample-mask"))
  {
    resampleMask = us::any_cast<bool>(parsedArgs["resample-mask"]);
  }

  mitk::Image::Pointer image;
  mitk::Image::Pointer mask;

  mitk::Image::Pointer tmpImage = mitk::IOUtil::LoadImage(parsedArgs["image"].ToString());
  mitk::Image::Pointer tmpMask = mitk::IOUtil::LoadImage(parsedArgs["mask"].ToString());
  image = tmpImage;
  mask = tmpMask;

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



  if (parsedArgs.count("fixed-isotropic"))
  {
    mitk::Image::Pointer newImage = mitk::Image::New();
    float resolution = us::any_cast<float>(parsedArgs["fixed-isotropic"]);
    AccessByItk_2(image, ResampleImage, resolution, newImage);
    image = newImage;
  }

  if (resampleMask)
  {
    mitk::Image::Pointer newMaskImage = mitk::Image::New();
    AccessByItk_2(mask, ResampleMask, image, newMaskImage);
    mask = newMaskImage;
    if (parsedArgs.count("save-resample-mask"))
    {
      mitk::IOUtil::SaveImage(mask, parsedArgs["save-resample-mask"].ToString());
    }
  }


  bool fixDifferentSpaces = parsedArgs.count("same-space");
  if ( ! mitk::Equal(mask->GetGeometry(0)->GetOrigin(), image->GetGeometry(0)->GetOrigin()))
  {
    MITK_INFO << "Not equal Origins";
    if (fixDifferentSpaces)
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
  if ( ! mitk::Equal(mask->GetGeometry(0)->GetSpacing(), image->GetGeometry(0)->GetSpacing()))
  {
    MITK_INFO << "Not equal Sapcings";
    if (fixDifferentSpaces)
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
    direction = splitDouble(parsedArgs["direction"].ToString(), ';')[0];
  }

  MITK_INFO << "Start creating Mask without NaN";

  mitk::Image::Pointer maskNoNaN = mitk::Image::New();
  AccessByItk_2(image, CreateNoNaNMask,  mask, maskNoNaN);
  //CreateNoNaNMask(mask, image, maskNoNaN);


  bool sliceWise = false;
  int sliceDirection = 0;
  int currentSlice = 0;
  bool imageToProcess = true;

  std::vector<mitk::Image::Pointer> floatVector;
  std::vector<mitk::Image::Pointer> maskVector;
  std::vector<mitk::Image::Pointer> maskNoNaNVector;

  if (parsedArgs.count("slice-wise"))
  {
    MITK_INFO << "Enabled slice-wise";
    sliceWise = true;
    sliceDirection = splitDouble(parsedArgs["slice-wise"].ToString(), ';')[0];
    MITK_INFO << sliceDirection;
    ExtractSlicesFromImages(image, mask, maskNoNaN, sliceDirection, floatVector, maskVector, maskNoNaNVector);
    MITK_INFO << "Slice";
  }

  if (parsedArgs.count("minimum-intensity"))
  {
    float minimum = splitDouble(parsedArgs["minimum-intensity"].ToString(), ';')[0];
    firstOrderCalculator->SetMinimumIntensity(minimum);
    firstOrderCalculator->SetUseMinimumIntensity(true);
    volCalculator->SetMinimumIntensity(minimum);
    volCalculator->SetUseMinimumIntensity(true);
    coocCalculator->SetMinimumIntensity(minimum);
    coocCalculator->SetUseMinimumIntensity(true);
    cooc2Calculator->SetMinimumIntensity(minimum);
    cooc2Calculator->SetUseMinimumIntensity(true);
    ngldCalculator->SetMinimumIntensity(minimum);
    ngldCalculator->SetUseMinimumIntensity(true);
    rlCalculator->SetMinimumIntensity(minimum);
    rlCalculator->SetUseMinimumIntensity(true);
  }

  if (parsedArgs.count("maximum-intensity"))
  {
    float minimum = splitDouble(parsedArgs["maximum-intensity"].ToString(), ';')[0];
    firstOrderCalculator->SetMaximumIntensity(minimum);
    firstOrderCalculator->SetUseMaximumIntensity(true);
    volCalculator->SetMaximumIntensity(minimum);
    volCalculator->SetUseMaximumIntensity(true);
    coocCalculator->SetMaximumIntensity(minimum);
    coocCalculator->SetUseMaximumIntensity(true);
    cooc2Calculator->SetMaximumIntensity(minimum);
    cooc2Calculator->SetUseMaximumIntensity(true);
    ngldCalculator->SetMaximumIntensity(minimum);
    ngldCalculator->SetUseMaximumIntensity(true);
    rlCalculator->SetMaximumIntensity(minimum);
    rlCalculator->SetUseMaximumIntensity(true);
  }

  if (parsedArgs.count("bins"))
  {
    int minimum = splitDouble(parsedArgs["bins"].ToString(), ';')[0];
    firstOrderCalculator->SetBins(minimum);
    volCalculator->SetBins(minimum);
    coocCalculator->SetBins(minimum);
    cooc2Calculator->SetBins(minimum);
    ngldCalculator->SetBins(minimum);
    rlCalculator->SetBins(minimum);
  }





  firstOrderCalculator->SetParameter(parsedArgs);
  firstOrderCalculator->SetDirection(direction);
  volCalculator->SetParameter(parsedArgs);
  volCalculator->SetDirection(direction);
  coocCalculator->SetParameter(parsedArgs);
  coocCalculator->SetDirection(direction);
  cooc2Calculator->SetParameter(parsedArgs);
  cooc2Calculator->SetDirection(direction);
  ngldCalculator->SetParameter(parsedArgs);
  ngldCalculator->SetDirection(direction);
  rlCalculator->SetParameter(parsedArgs);
  rlCalculator->SetDirection(direction);

  std::ofstream output(parsedArgs["output"].ToString(), std::ios::app);

  mitk::Image::Pointer cImage = image;
  mitk::Image::Pointer cMask = mask;
  mitk::Image::Pointer cMaskNoNaN = maskNoNaN;

  while (imageToProcess)
  {
    if (sliceWise)
    {
      cImage = floatVector[currentSlice];
      cMask = maskVector[currentSlice];
      cMaskNoNaN = maskNoNaNVector[currentSlice];
      imageToProcess = (floatVector.size()-1 > (currentSlice)) ? true : false ;
    }
    else
    {
      imageToProcess = false;
    }


    mitk::AbstractGlobalImageFeature::FeatureListType stats;
    firstOrderCalculator->CalculateFeaturesUsingParameters(cImage, cMask, cMaskNoNaN, stats);
    volCalculator->CalculateFeaturesUsingParameters(cImage, cMask, cMaskNoNaN, stats);
    coocCalculator->CalculateFeaturesUsingParameters(cImage, cMask, cMaskNoNaN, stats);
    cooc2Calculator->CalculateFeaturesUsingParameters(cImage, cMask, cMaskNoNaN, stats);
    ngldCalculator->CalculateFeaturesUsingParameters(cImage, cMask, cMaskNoNaN, stats);
    rlCalculator->CalculateFeaturesUsingParameters(cImage, cMask, cMaskNoNaN, stats);

    for (std::size_t i = 0; i < stats.size(); ++i)
    {
      std::cout << stats[i].first << " - " << stats[i].second << std::endl;
    }

    if (parsedArgs.count("header") && (currentSlice == 0))
    {
      if (parsedArgs.count("description"))
      {
        output << "Description" << ";";
      }
      if (sliceWise)
      {
        output << "SliceNumber" << ";";
      }
      for (std::size_t i = 0; i < stats.size(); ++i)
      {
        output << stats[i].first << ";";
      }
      output << "EndOfMeasurement;";
      output << std::endl;
    }

    if (parsedArgs.count("description"))
    {
      output << parsedArgs["description"].ToString() << ";";
    }
    if (sliceWise)
    {
      output << currentSlice << ";";
    }
    for (std::size_t i = 0; i < stats.size(); ++i)
    {
      output << stats[i].second << ";";
    }
    output << "EndOfMeasurement;";
    output << std::endl;

    ++currentSlice;
  }
  if (sliceWise)
  {
    output << "EndOfFile" << std::endl;
  }
  output.close();

  return 0;
}

#endif
