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

#include <mitkIOUtil.h>

#include "mitkImage.h"
#include <mitkImageCast.h>
#include "mitkITKImageImport.h"
#include <mitkTensorImage.h>
#include <itkDiffusionTensor3D.h>
#include "mitkCommandLineParser.h"

#include <itkConnectednessFilter.h>

// ITK
#include "itkBinaryErodeImageFilter.h"
#include "itkFlatStructuringElement.h"

using namespace std;

typedef itk::Image<unsigned char,3> BinaryType;
typedef itk::Image<mitk::ScalarType,3> ResultType;
typedef itk::Image< itk::DiffusionTensor3D<float>, 3 > ItkTensorImage;

int main(int argc, char* argv[])
{
    // Setup CLI Module parsable interface
    mitkCommandLineParser parser;
    parser.setTitle("Connectedness Maps");
    parser.setCategory("Features");
    parser.setDescription("Computes connectedness maps");
    parser.setContributor("MBI");

    parser.setArgumentPrefix("--","-");
    parser.addArgument("input", "i", mitkCommandLineParser::InputImage, "input file");
    parser.addArgument("seed", "s", mitkCommandLineParser::InputImage, "seed file");
    parser.addArgument("mask", "m", mitkCommandLineParser::InputImage, "mask file");
    parser.addArgument("mode", "t", mitkCommandLineParser::String, "Mode Feature |  Vector | FeatureVector");
    parser.addArgument("vector", "v", mitkCommandLineParser::InputImage, "Tensor Image (.dti)");
    parser.addArgument("confidence", "c", mitkCommandLineParser::InputImage, "confidence map (only when Tensor Images are used)");
    parser.addArgument("valueImage", "x", mitkCommandLineParser::InputImage, "image of values that are propagated");

    parser.addArgument("erodeSeed", "a", mitkCommandLineParser::Bool, "apply erosion of seed region");

    parser.addArgument("rankFilter", "r", mitkCommandLineParser::Bool, "median filter for propagation");

    parser.addArgument("propMap", "p", mitkCommandLineParser::OutputFile, "[out] propagated map");
    parser.addArgument("distanceMap", "d", mitkCommandLineParser::OutputFile, "[out] connectedness map");
    parser.addArgument("euclidDistanceMap", "e", mitkCommandLineParser::OutputFile, "[out] euclid distance map");

    // Parse input parameters
    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

    // Show a help message
    if ( parsedArgs.size()==0 || parsedArgs.count("help") || parsedArgs.count("h"))
    {
      std::cout << parser.helpText();
      return EXIT_SUCCESS;
    }

    bool useRank = false;
    bool applyErosion = false;
    bool useValueImage = false;
    if (parsedArgs.count("rankFilter") || parsedArgs.count("r"))
        useRank = true;

    if (parsedArgs.count("valueImage") || parsedArgs.count("x"))
        useValueImage = true;

    if (parsedArgs.count("erodeSeed") || parsedArgs.count("a"))
        applyErosion = true;


    std::string inputFile = us::any_cast<string>(parsedArgs["input"]);
    std::string propMap = us::any_cast<string>(parsedArgs["propMap"]);
    std::string conMap = us::any_cast<string>(parsedArgs["distanceMap"]);
    std::string tensImageFile = us::any_cast<string>(parsedArgs["vector"]);
    std::string maskFile = us::any_cast<string>(parsedArgs["mask"]);
    std::string mode = us::any_cast<string>(parsedArgs["mode"]);
    std::string seedFile = us::any_cast<string>(parsedArgs["seed"]);
    std::string confFile = us::any_cast<string>(parsedArgs["confidence"]);
    std::string euclidFile = us::any_cast<string>(parsedArgs["euclidDistanceMap"]);

    std::string valueImageFile = "";
    if (useValueImage)
        valueImageFile = us::any_cast<string>(parsedArgs["valueImage"]);

    // Read-in image data
    mitk::Image::Pointer tmpImage;
    mitk::Image::Pointer inputImage =  mitk::IOUtil::LoadImage(inputFile);
    mitk::Image::Pointer maskImage = mitk::IOUtil::LoadImage(maskFile);
    mitk::Image::Pointer seedImage = mitk::IOUtil::LoadImage(seedFile);

    mitk::Image::Pointer valueImage;
    if (useValueImage)
        valueImage = mitk::IOUtil::LoadImage(valueImageFile);

    mitk::Image::Pointer confImage;
    if (mode == "Vector" || mode == "FeatureVector")
    {
        MITK_INFO << "Load Tensor/Confidence";
        tmpImage= mitk::IOUtil::LoadImage(tensImageFile);
        confImage= mitk::IOUtil::LoadImage(confFile);
    }

    mitk::TensorImage* diffusionImage =   static_cast<mitk::TensorImage*>(tmpImage.GetPointer());

    // Convert all input data to ITK
    BinaryType::Pointer itkSeed = BinaryType::New();
    BinaryType::Pointer itkMask = BinaryType::New();
    ResultType::Pointer itkImage = ResultType::New();
    ItkTensorImage::Pointer itkTensor = ItkTensorImage::New();
    ResultType::Pointer itkWeight = ResultType::New();
    ResultType::Pointer itkValueImage = ResultType::New();

    mitk::CastToItkImage(inputImage, itkImage);
    mitk::CastToItkImage(maskImage, itkMask);
    mitk::CastToItkImage(seedImage, itkSeed);
    if (useValueImage)
        mitk::CastToItkImage(valueImage, itkValueImage);



    if (applyErosion)
    {

      typedef itk::FlatStructuringElement<3> StructuringElementType;
      StructuringElementType::RadiusType elementRadius;
      elementRadius.Fill(2);
      elementRadius[2] = 0;
      StructuringElementType structuringElement = StructuringElementType::Box(elementRadius);

      typedef itk::BinaryErodeImageFilter <BinaryType, BinaryType, StructuringElementType> BinaryErodeImageFilterType;

      BinaryErodeImageFilterType::Pointer erodeFilter = BinaryErodeImageFilterType::New();
      erodeFilter->SetInput(itkSeed);
      erodeFilter->SetKernel(structuringElement);
      erodeFilter->SetForegroundValue(1);
      erodeFilter->Update();
      itkSeed = erodeFilter->GetOutput();
    }


    if (mode == "Vector" || mode == "FeatureVector")
    {
        mitk::CastToItkImage(diffusionImage, itkTensor);
        mitk::CastToItkImage(confImage, itkWeight);
    }

    // Setup filter
    itk::ConnectednessFilter<ResultType, BinaryType,float>::Pointer filter
            = itk::ConnectednessFilter<ResultType, BinaryType, float>::New();

    filter->SetInputImage(itkImage);
    filter->SetInputSeed(itkSeed);
    filter->SetInputMask(itkMask);
    if (mode == "Vector")
    {
        filter->SetInputVectorField(itkTensor);
        filter->SetInputVectorFieldConfidenceMap(itkWeight);
        filter->SetMode(itk::ConnectednessFilter<ResultType, BinaryType,float>::VectorAgreement);
    }
    else if (mode == "FeatureVector")
    {
        filter->SetInputVectorField(itkTensor);
        filter->SetInputVectorFieldConfidenceMap(itkWeight);
        filter->SetMode(itk::ConnectednessFilter<ResultType, BinaryType,float>::FeatureVectorAgreement);
    }
    else
        filter->SetMode(itk::ConnectednessFilter<ResultType, BinaryType,float>::FeatureSimilarity);

    if (useValueImage)
        filter->SetPropagationImage(itkValueImage);

    filter->SetApplyRankFilter(useRank);
    filter->Update();

    // Grab output and write results
    mitk::Image::Pointer result = mitk::Image::New();
    mitk::GrabItkImageMemory(filter->GetOutput(), result);

    mitk::IOUtil::Save(result, propMap);

    mitk::Image::Pointer distance = mitk::Image::New();
    mitk::GrabItkImageMemory(filter->GetDistanceImage().GetPointer(), distance);
    mitk::IOUtil::Save(distance, conMap);

    mitk::Image::Pointer euclidDistance = mitk::Image::New();
    mitk::GrabItkImageMemory(filter->GetEuclideanDistanceImage().GetPointer(), euclidDistance);
    mitk::IOUtil::Save(euclidDistance, euclidFile);

    return EXIT_SUCCESS;
}
