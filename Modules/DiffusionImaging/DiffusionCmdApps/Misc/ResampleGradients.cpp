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

#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <istream>
#include <itkMetaDataObject.h>
#include <itkVectorImage.h>
#include <mitkImageCast.h>
#include <mitkBaseData.h>
#include "mitkCommandLineParser.h"
#include <mitkLexicalCast.h>
#include <mitkCoreObjectFactory.h>
#include <mitkIOUtil.h>
#include <mitkBValueMapProperty.h>
#include <mitkGradientDirectionsProperty.h>
#include <itkElectrostaticRepulsionDiffusionGradientReductionFilter.h>
#include <itkImage.h>
#include <mitkImage.h>
#include "itkDWIVoxelFunctor.h"
#include <mitkDiffusionPropertyHelper.h>

typedef short DiffusionPixelType;
typedef itk::VectorImage< short, 3 > ItkDwiType;

// itk includes
#include "itkTimeProbe.h"
#include "itkB0ImageExtractionImageFilter.h"
#include "itkB0ImageExtractionToSeparateImageFilter.h"
#include "itkBrainMaskExtractionImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkVectorContainer.h"
#include <itkElectrostaticRepulsionDiffusionGradientReductionFilter.h>
#include <itkMergeDiffusionImagesFilter.h>
#include <itkDwiGradientLengthCorrectionFilter.h>
#include <itkDwiNormilzationFilter.h>
#include <mitkTensorImage.h>
#include <mitkOdfImage.h>

// Multishell includes
#include <itkRadialMultishellToSingleshellImageFilter.h>

// Multishell Functors
#include <itkADCAverageFunctor.h>
#include <itkKurtosisFitFunctor.h>
#include <itkBiExpFitFunctor.h>
#include <itkADCFitFunctor.h>

// mitk includes
#include "mitkProgressBar.h"
#include "mitkStatusBar.h"
#include "mitkNodePredicateDataType.h"
#include "mitkProperties.h"
#include "mitkVtkResliceInterpolationProperty.h"
#include "mitkLookupTable.h"
#include "mitkLookupTableProperty.h"
#include "mitkTransferFunction.h"
#include "mitkTransferFunctionProperty.h"
//#include "mitkDataNodeObject.h"
#include "mitkOdfNormalizationMethodProperty.h"
#include "mitkOdfScaleByProperty.h"
#include <mitkPointSet.h>
#include <itkAdcImageFilter.h>
#include <itkBrainMaskExtractionImageFilter.h>
#include <mitkImageCast.h>
#include <mitkRotationOperation.h>
//#include <QTableWidgetItem>
//#include <QTableWidget>
#include <mitkInteractionConst.h>
#include <mitkImageAccessByItk.h>
#include <itkResampleDwiImageFilter.h>
#include <itkResampleImageFilter.h>
#include <itkImageDuplicator.h>
#include <itkMaskImageFilter.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <itkCropImageFilter.h>
#include <itkDiffusionTensor3DReconstructionImageFilter.h>
#include <itkRemoveDwiChannelFilter.h>
#include <itkExtractDwiChannelFilter.h>
#include <mitkITKImageImport.h>
#include <mitkBValueMapProperty.h>
#include <mitkGradientDirectionsProperty.h>
#include <mitkMeasurementFrameProperty.h>
#include <itkImageDuplicator.h>
#include <itkFlipImageFilter.h>
#include <mitkITKImageImport.h>
#include "mitkPreferenceListReaderOptionsFunctor.h"

mitk::Image::Pointer DoReduceGradientDirections(mitk::Image::Pointer image, double BValue, unsigned int numOfGradientsToKeep, bool use_first_n)
{

  bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image) );
  if ( !isDiffusionImage ) {
    std::cout << "Image is not a Diffusion Weighted Image" << std::endl;
    //return;
  }

  typedef itk::ElectrostaticRepulsionDiffusionGradientReductionFilter<DiffusionPixelType, DiffusionPixelType> FilterType;
  typedef mitk::BValueMapProperty::BValueMap BValueMap;

  BValueMap shellSlectionMap;
  BValueMap originalShellMap = mitk::DiffusionPropertyHelper::GetBValueMap(image);

  std::vector<unsigned int> newNumGradientDirections;

  //Keeps 1 b0 gradient
  double B0Value = 0;
  shellSlectionMap[B0Value] = originalShellMap[B0Value];
  unsigned int num = 1;
  newNumGradientDirections.push_back(num);

  //BValue = 1000;
  shellSlectionMap[BValue] = originalShellMap[BValue];
  //numOfGradientsToKeep = 32;
  newNumGradientDirections.push_back(numOfGradientsToKeep);


  if (newNumGradientDirections.empty()) {
    std::cout << "newNumGradientDirections is empty" << std::endl;
    //return;
  }

  auto gradientContainer = mitk::DiffusionPropertyHelper::GetGradientContainer(image);


  ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
  mitk::CastToItkImage(image, itkVectorImagePointer);
  std::cout << "1" << std::endl;
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput( itkVectorImagePointer );
  filter->SetOriginalGradientDirections(gradientContainer);
  filter->SetNumGradientDirections(newNumGradientDirections);
  filter->SetOriginalBValueMap(originalShellMap);
  filter->SetShellSelectionBValueMap(shellSlectionMap);
  filter->SetUseFirstN(use_first_n);
  filter->Update();
  std::cout << "2" << std::endl;

  if( filter->GetOutput() == nullptr) {
    std::cout << "filter get output is nullptr" << std::endl;
  }

  mitk::Image::Pointer newImage = mitk::GrabItkImageMemory( filter->GetOutput() );
  mitk::DiffusionPropertyHelper::CopyProperties(image, newImage, true);
  mitk::DiffusionPropertyHelper::SetGradientContainer(newImage, filter->GetGradientDirections());
  mitk::DiffusionPropertyHelper::InitializeImage( newImage );

  return newImage;
}



/*!
\brief Resample gradients of input DWI image.
*/
int main(int argc, char* argv[])
{
    mitkCommandLineParser parser;

    parser.setTitle("Resample Gradients");
    parser.setCategory("Preprocessing Tools");
    parser.setDescription("Resample gradients of input DWI image. You can select one b-value shell and the number of gradients within this shell you want to have. It will also keep one b0 image.");
    parser.setContributor("MIC");

    parser.setArgumentPrefix("--", "-");
    parser.addArgument("", "i", mitkCommandLineParser::String, "Input:", "input image", us::Any(), false);
    parser.addArgument("", "o", mitkCommandLineParser::String, "Output:", "output image", us::Any(), false);
    parser.addArgument("b_value", "", mitkCommandLineParser::Float, "b-value:", "float", 1000, false);
    parser.addArgument("num_gradients", "", mitkCommandLineParser::Int, "Nr of gradients:", "integer", 32, false);
    parser.addArgument("use_first_n", "", mitkCommandLineParser::Bool, "Use first N:", "no optimization, simply use first n gradients", 0);


    std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;


    std::string inFileName = us::any_cast<std::string>(parsedArgs["i"]);
    std::string outFileName = us::any_cast<std::string>(parsedArgs["o"]);
    double bValue = us::any_cast<float>(parsedArgs["b_value"]);
    unsigned int nrOfGradients = us::any_cast<int>(parsedArgs["num_gradients"]);
    bool use_first_n = false;
    if (parsedArgs.count("use_first_n"))
      use_first_n = true;

    try
    {
        mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({ "Diffusion Weighted Images" }, {});
        mitk::Image::Pointer mitkImage = mitk::IOUtil::Load<mitk::Image>(inFileName, &functor);
        mitk::Image::Pointer newImage = DoReduceGradientDirections(mitkImage, bValue, nrOfGradients, use_first_n);
        //mitk::IOUtil::Save(newImage, outFileName); //save as dwi image
        mitk::IOUtil::Save(newImage, "DWI_NIFTI", outFileName);  //save as nifti image

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
