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

#include <itkImageFileWriter.h>
#include <itkResampleImageFilter.h>
#include <itkFiniteDiffOdfMaximaExtractionFilter.h>

#include <mitkImage.h>
#include <mitkOdfImage.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <mitkTensorImage.h>

#include <mitkCoreObjectFactory.h>
#include "mitkCommandLineParser.h"
#include <itkShCoefficientImageImporter.h>
#include <itkFlipImageFilter.h>
#include <mitkLexicalCast.h>
#include <boost/algorithm/string.hpp>

#include <mitkIOUtil.h>

template<int shOrder>
int StartPeakExtraction(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  parser.addArgument("image", "i", mitkCommandLineParser::InputFile, "Input image", "sh coefficient image", us::Any(), false);
  parser.addArgument("outroot", "o", mitkCommandLineParser::OutputDirectory, "Output directory", "output root", us::Any(), false);
  parser.addArgument("mask", "m", mitkCommandLineParser::InputFile, "Mask", "mask image");
  parser.addArgument("normalization", "n", mitkCommandLineParser::Int, "Normalization", "0=no norm, 1=max norm, 2=single vec norm", 1, true);
  parser.addArgument("numpeaks", "p", mitkCommandLineParser::Int, "Max. number of peaks", "maximum number of extracted peaks", 2, true);
  parser.addArgument("peakthres", "r", mitkCommandLineParser::Float, "Peak threshold", "peak threshold relative to largest peak", 0.4, true);
  parser.addArgument("abspeakthres", "a", mitkCommandLineParser::Float, "Absolute peak threshold", "absolute peak threshold weighted with local GFA value", 0.06, true);
  parser.addArgument("shConvention", "s", mitkCommandLineParser::String, "Use specified SH-basis", "use specified SH-basis (MITK, FSL, MRtrix)", std::string("MITK"), true);
  parser.addArgument("noFlip", "f", mitkCommandLineParser::Bool, "No flip", "do not flip input image to match MITK coordinate convention");
  parser.addArgument("clusterThres", "c", mitkCommandLineParser::Float, "Clustering threshold", "directions closer together than the specified angular threshold will be clustered (in rad)", 0.9);
  parser.addArgument("flipX", "fx", mitkCommandLineParser::Bool, "Flip X", "Flip peaks in x direction");
  parser.addArgument("flipY", "fy", mitkCommandLineParser::Bool, "Flip Y", "Flip peaks in y direction");
  parser.addArgument("flipZ", "fz", mitkCommandLineParser::Bool, "Flip Z", "Flip peaks in z direction");


  parser.setCategory("Preprocessing Tools");
  parser.setTitle("Peak Extraction");
  parser.setDescription("");
  parser.setContributor("MIC");

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  // mandatory arguments
  std::string imageName = us::any_cast<std::string>(parsedArgs["image"]);
  std::string outRoot = us::any_cast<std::string>(parsedArgs["outroot"]);

  // optional arguments
  std::string maskImageName("");
  if (parsedArgs.count("mask"))
    maskImageName = us::any_cast<std::string>(parsedArgs["mask"]);

  int normalization = 1;
  if (parsedArgs.count("normalization"))
    normalization = us::any_cast<int>(parsedArgs["normalization"]);

  int numPeaks = 2;
  if (parsedArgs.count("numpeaks"))
    numPeaks = us::any_cast<int>(parsedArgs["numpeaks"]);

  float peakThres = 0.4;
  if (parsedArgs.count("peakthres"))
    peakThres = us::any_cast<float>(parsedArgs["peakthres"]);

  float absPeakThres = 0.06;
  if (parsedArgs.count("abspeakthres"))
    absPeakThres = us::any_cast<float>(parsedArgs["abspeakthres"]);

  float clusterThres = 0.9;
  if (parsedArgs.count("clusterThres"))
    clusterThres = us::any_cast<float>(parsedArgs["clusterThres"]);

  bool noFlip = false;
  if (parsedArgs.count("noFlip"))
    noFlip = us::any_cast<bool>(parsedArgs["noFlip"]);

  bool flipX = false;
  if (parsedArgs.count("flipX"))
    flipX = us::any_cast<bool>(parsedArgs["flipX"]);

  bool flipY = false;
  if (parsedArgs.count("flipY"))
    flipY = us::any_cast<bool>(parsedArgs["flipY"]);

  bool flipZ = false;
  if (parsedArgs.count("flipZ"))
    flipZ = us::any_cast<bool>(parsedArgs["flipZ"]);

  std::cout << "image: " << imageName;
  std::cout << "outroot: " << outRoot;
  if (!maskImageName.empty())
    std::cout << "mask: " << maskImageName;
  else
    std::cout << "no mask image selected";
  std::cout << "numpeaks: " << numPeaks;
  std::cout << "peakthres: " << peakThres;
  std::cout << "abspeakthres: " << absPeakThres;
  std::cout << "shOrder: " << shOrder;

  try
  {
    mitk::Image::Pointer image = mitk::IOUtil::Load<mitk::Image>(imageName);
    mitk::Image::Pointer mask = mitk::IOUtil::Load<mitk::Image>(maskImageName);

    typedef itk::Image<unsigned char, 3>  ItkUcharImgType;
    typedef itk::FiniteDiffOdfMaximaExtractionFilter< float, shOrder, 20242 > MaximaExtractionFilterType;
    typename MaximaExtractionFilterType::Pointer peak_extraction_filter = MaximaExtractionFilterType::New();

    int toolkitConvention = 0;

    if (parsedArgs.count("shConvention"))
    {
      std::string convention = us::any_cast<std::string>(parsedArgs["shConvention"]).c_str();
      if ( boost::algorithm::equals(convention, "FSL") )
      {
        toolkitConvention = 1;
        std::cout << "Using FSL SH-basis";
      }
      else if ( boost::algorithm::equals(convention, "MRtrix") )
      {
        toolkitConvention = 2;
        std::cout << "Using MRtrix SH-basis";
      }
      else
        std::cout << "Using MITK SH-basis";
    }
    else
      std::cout << "Using MITK SH-basis";

    ItkUcharImgType::Pointer itkMaskImage = nullptr;
    if (mask.IsNotNull())
    {
      try{
        itkMaskImage = ItkUcharImgType::New();
        mitk::CastToItkImage(mask, itkMaskImage);
        peak_extraction_filter->SetMaskImage(itkMaskImage);
      }
      catch(...)
      {

      }
    }

    if (toolkitConvention>0)
    {
      std::cout << "Converting coefficient image to MITK format";
      typedef itk::ShCoefficientImageImporter< float, shOrder > ConverterType;
      typedef mitk::ImageToItk< itk::Image< float, 4 > > CasterType;
      CasterType::Pointer caster = CasterType::New();
      caster->SetInput(image);
      caster->Update();
      itk::Image< float, 4 >::Pointer itkImage = caster->GetOutput();

      typename ConverterType::Pointer converter = ConverterType::New();

      if (noFlip)
      {
        converter->SetInputImage(itkImage);
      }
      else
      {
        std::cout << "Flipping image";
        itk::FixedArray<bool, 4> flipAxes;
        flipAxes[0] = true;
        flipAxes[1] = true;
        flipAxes[2] = false;
        flipAxes[3] = false;
        itk::FlipImageFilter< itk::Image< float, 4 > >::Pointer flipper = itk::FlipImageFilter< itk::Image< float, 4 > >::New();
        flipper->SetInput(itkImage);
        flipper->SetFlipAxes(flipAxes);
        flipper->Update();
        itk::Image< float, 4 >::Pointer flipped = flipper->GetOutput();
        itk::Matrix< double,4,4 > m = itkImage->GetDirection(); m[0][0] *= -1; m[1][1] *= -1;
        flipped->SetDirection(m);

        itk::Point< float, 4 > o = itkImage->GetOrigin();
        o[0] -= (flipped->GetLargestPossibleRegion().GetSize(0)-1);
        o[1] -= (flipped->GetLargestPossibleRegion().GetSize(1)-1);
        flipped->SetOrigin(o);
        converter->SetInputImage(flipped);
      }

      std::cout << "Starting conversion";
      switch (toolkitConvention)
      {
      case 1:
        peak_extraction_filter->SetToolkit(MaximaExtractionFilterType::FSL);
        break;
      case 2:
        peak_extraction_filter->SetToolkit(MaximaExtractionFilterType::MRTRIX);
        break;
      default:
        peak_extraction_filter->SetToolkit(MaximaExtractionFilterType::FSL);
        break;
      }
      converter->GenerateData();
      peak_extraction_filter->SetInput(converter->GetCoefficientImage());
    }
    else
    {
      try{
        typedef mitk::ImageToItk< typename MaximaExtractionFilterType::CoefficientImageType > CasterType;
        typename CasterType::Pointer caster = CasterType::New();
        caster->SetInput(image);
        caster->Update();
        peak_extraction_filter->SetInput(caster->GetOutput());
      }
      catch(...)
      {
        std::cout << "wrong image type";
        return EXIT_FAILURE;
      }
    }

    peak_extraction_filter->SetMaxNumPeaks(numPeaks);
    peak_extraction_filter->SetPeakThreshold(peakThres);
    peak_extraction_filter->SetAbsolutePeakThreshold(absPeakThres);
    peak_extraction_filter->SetAngularThreshold(1);
    peak_extraction_filter->SetClusteringThreshold(clusterThres);
    peak_extraction_filter->SetFlipX(flipX);
    peak_extraction_filter->SetFlipY(flipY);
    peak_extraction_filter->SetFlipZ(flipZ);

    switch (normalization)
    {
    case 0:
      peak_extraction_filter->SetNormalizationMethod(MaximaExtractionFilterType::NO_NORM);
      break;
    case 1:
      peak_extraction_filter->SetNormalizationMethod(MaximaExtractionFilterType::MAX_VEC_NORM);
      break;
    case 2:
      peak_extraction_filter->SetNormalizationMethod(MaximaExtractionFilterType::SINGLE_VEC_NORM);
      break;
    }

    std::cout << "Starting extraction";
    peak_extraction_filter->Update();

    // write direction image
    {
      typename MaximaExtractionFilterType::PeakImageType::Pointer itkImg = peak_extraction_filter->GetPeakImage();
      std::string outfilename = outRoot;
      outfilename.append("_PEAKS.nrrd");

      typedef itk::ImageFileWriter< typename MaximaExtractionFilterType::PeakImageType > WriterType;
      typename WriterType::Pointer writer = WriterType::New();
      writer->SetFileName(outfilename);
      writer->SetInput(itkImg);
      writer->Update();
    }

    // write num directions image
    {
      ItkUcharImgType::Pointer numDirImage = peak_extraction_filter->GetNumDirectionsImage();

      if (itkMaskImage.IsNotNull())
      {
        numDirImage->SetDirection(itkMaskImage->GetDirection());
        numDirImage->SetOrigin(itkMaskImage->GetOrigin());
      }

      std::string outfilename = outRoot.c_str();
      outfilename.append("_NUM_PEAKS.nrrd");
      typedef itk::ImageFileWriter< ItkUcharImgType > WriterType;
      WriterType::Pointer writer = WriterType::New();
      writer->SetFileName(outfilename);
      writer->SetInput(numDirImage);
      writer->Update();
    }
  }
  catch (itk::ExceptionObject e)
  {
    std::cout << e;
    return EXIT_FAILURE;
  }
  catch (std::exception e)
  {
    std::cout << e.what();
    return EXIT_FAILURE;
  }
  catch (...)
  {
    std::cout << "ERROR!?!";
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

/*!
\brief Extract maxima in the input spherical harmonics image.
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  parser.addArgument("image", "i", mitkCommandLineParser::InputFile, "Input image", "sh coefficient image", us::Any(), false);
  parser.addArgument("shOrder", "sh", mitkCommandLineParser::Int, "Spherical harmonics order", "spherical harmonics order");
  parser.addArgument("outroot", "o", mitkCommandLineParser::OutputDirectory, "Output directory", "output root", us::Any(), false);
  parser.addArgument("mask", "m", mitkCommandLineParser::InputFile, "Mask", "mask image");
  parser.addArgument("normalization", "n", mitkCommandLineParser::Int, "Normalization", "0=no norm, 1=max norm, 2=single vec norm", 1, true);
  parser.addArgument("numpeaks", "p", mitkCommandLineParser::Int, "Max. number of peaks", "maximum number of extracted peaks", 2, true);
  parser.addArgument("peakthres", "r", mitkCommandLineParser::Float, "Peak threshold", "peak threshold relative to largest peak", 0.4, true);
  parser.addArgument("abspeakthres", "a", mitkCommandLineParser::Float, "Absolute peak threshold", "absolute peak threshold weighted with local GFA value", 0.06, true);
  parser.addArgument("shConvention", "s", mitkCommandLineParser::String, "Use specified SH-basis", "use specified SH-basis (MITK, FSL, MRtrix)", std::string("MITK"), true);
  parser.addArgument("noFlip", "f", mitkCommandLineParser::Bool, "No flip", "do not flip input image to match MITK coordinate convention");

  parser.setCategory("Preprocessing Tools");
  parser.setTitle("Peak Extraction");
  parser.setDescription("Extract maxima in the input spherical harmonics image.");
  parser.setContributor("MIC");

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;


  int shOrder = -1;
  if (parsedArgs.count("shOrder"))
    shOrder = us::any_cast<int>(parsedArgs["shOrder"]);

  switch (shOrder)
  {
  case 4:
    return StartPeakExtraction<4>(argc, argv);
  case 6:
    return StartPeakExtraction<6>(argc, argv);
  case 8:
    return StartPeakExtraction<8>(argc, argv);
  case 10:
    return StartPeakExtraction<10>(argc, argv);
  case 12:
    return StartPeakExtraction<12>(argc, argv);
  }
  return EXIT_FAILURE;
}
