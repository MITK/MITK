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

#include <itkImageFileWriter.h>
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


// itk includes
#include <itkImage.h>

// mitk includes
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

mitk::Image::Pointer DoReduceGradientDirections(mitk::Image::Pointer image, double BValue, unsigned int numOfGradientsToKeep)
{

  bool isDiffusionImage( mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(image) );
  if ( !isDiffusionImage ) {
    std::cout << "Image is not a Diffusion Weighted Image" << std::endl;
    //return;
  }

  typedef itk::ElectrostaticRepulsionDiffusionGradientReductionFilter<DiffusionPixelType, DiffusionPixelType> FilterType;
  typedef mitk::BValueMapProperty::BValueMap BValueMap;

  BValueMap shellSlectionMap;
  BValueMap originalShellMap = static_cast<mitk::BValueMapProperty*>
    (image->GetProperty(mitk::DiffusionPropertyHelper::BVALUEMAPPROPERTYNAME.c_str()).GetPointer() )
      ->GetBValueMap();

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

  itk::DwiGradientLengthCorrectionFilter::GradientDirectionContainerType::Pointer gradientContainer
      = static_cast<mitk::GradientDirectionsProperty*>
        ( image->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )
          ->GetGradientDirectionsContainer();


  ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
  mitk::CastToItkImage(image, itkVectorImagePointer);
  std::cout << "1" << std::endl;
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput( itkVectorImagePointer );
  filter->SetOriginalGradientDirections(gradientContainer);
  filter->SetNumGradientDirections(newNumGradientDirections);
  filter->SetOriginalBValueMap(originalShellMap);
  filter->SetShellSelectionBValueMap(shellSlectionMap);
  filter->Update();
  std::cout << "2" << std::endl;

  if( filter->GetOutput() == nullptr) {
    std::cout << "filter get output is nullptr" << std::endl;
  }

  mitk::Image::Pointer newImage = mitk::GrabItkImageMemory( filter->GetOutput() );
  mitk::DiffusionPropertyHelper::CopyProperties(image, newImage, true);

  newImage->GetPropertyList()->ReplaceProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(),
                         mitk::GradientDirectionsProperty::New( filter->GetGradientDirections() ) );

  mitk::DiffusionPropertyHelper propertyHelper( newImage );
  propertyHelper.InitializeImage(); //needed?

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
    parser.addArgument("input", "i", mitkCommandLineParser::String, "Input:", "input image", us::Any(), false);
    parser.addArgument("output", "o", mitkCommandLineParser::String, "Output:", "output image", us::Any(), false);
    parser.addArgument("bValue", "b", mitkCommandLineParser::Float, "b-value:", "float", 1000, false);
    parser.addArgument("nrOfGradients", "n", mitkCommandLineParser::Int, "Nr of gradients:", "integer", 32, false);


    std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;


    std::string inFileName = us::any_cast<std::string>(parsedArgs["input"]);
    std::string outFileName = us::any_cast<std::string>(parsedArgs["output"]);
    double bValue = us::any_cast<float>(parsedArgs["bValue"]);
    unsigned int nrOfGradients = us::any_cast<int>(parsedArgs["nrOfGradients"]);

    try
    {
        mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({ "Diffusion Weighted Images" }, {});
        mitk::Image::Pointer mitkImage = mitk::IOUtil::Load<mitk::Image>(inFileName, &functor);
        mitk::Image::Pointer newImage = DoReduceGradientDirections(mitkImage, bValue, nrOfGradients);
        //mitk::IOUtil::Save(newImage, outFileName); //save as dwi image
        mitk::IOUtil::Save(newImage, "application/vnd.mitk.nii.gz", outFileName);  //save as nifti image

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
