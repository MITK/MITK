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

#include <mitkImageCast.h>
#include <mitkITKImageImport.h>
#include <mitkProperties.h>
#include <mitkImage.h>
#include <mitkIOUtil.h>
#include <mitkFiberBundle.h>
#include <mitkFiberfoxParameters.h>
#include "mitkCommandLineParser.h"

#include <itkTractsToDWIImageFilter.h>
#include <boost/lexical_cast.hpp>

using namespace mitk;

/*!
* \brief Command line interface to Fiberfox.
* Simulate a diffusion-weighted image from a tractogram using the specified parameter file.
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setTitle("Fiberfox");
  parser.setCategory("Fiber Tracking and Processing Methods");
  parser.setContributor("MBI");
  parser.setDescription("Command line interface to Fiberfox."
                        " Simulate a diffusion-weighted image from a tractogram using the specified parameter file.");
  parser.setArgumentPrefix("--", "-");
  parser.addArgument("out", "o", mitkCommandLineParser::OutputFile, "Output root:",
                     "output root", us::Any(), false);
  parser.addArgument("parameters", "p", mitkCommandLineParser::InputFile, "Parameter file:",
                     "fiberfox parameter file (.ffp)", us::Any(), false);
  parser.addArgument("input", "i", mitkCommandLineParser::String, "Input:",
                     "Input tractogram or diffusion-weighted image.", us::Any(), false);
  parser.addArgument("verbose", "v", mitkCommandLineParser::Bool, "Output additional images:",
                     "output volume fraction images etc.", us::Any());

  map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
  {
    return EXIT_FAILURE;
  }
  string outName = us::any_cast<string>(parsedArgs["out"]);
  string paramName = us::any_cast<string>(parsedArgs["parameters"]);

  string input="";
  if (parsedArgs.count("input"))
  {
    input = us::any_cast<string>(parsedArgs["input"]);
  }
  bool verbose = false;
  if (parsedArgs.count("verbose"))
  {
    verbose = us::any_cast<bool>(parsedArgs["verbose"]);
  }
  FiberfoxParameters<double> parameters;
  parameters.LoadParameters(paramName);

  // Test if /path/dir is an existing directory:
  if( itksys::SystemTools::FileIsDirectory( outName ) )
  {
    while( *(--(outName.cend())) == '/')
    {
      outName.pop_back();
    }
    outName = outName + '/';
    parameters.m_Misc.m_OutputPath = outName;
    outName = outName + parameters.m_Misc.m_OutputPrefix; // using default m_OutputPrefix as initialized.
  }
  else
  {
    // outName is NOT an existing directory, so we need to remove all trailing slashes:
    while( *(--(outName.cend())) == '/')
    {
      outName.pop_back();
    }

    // now split up the given outName into directory and (prefix of) filename:
    if( ! itksys::SystemTools::GetFilenamePath( outName ).empty()
        && itksys::SystemTools::FileIsDirectory(itksys::SystemTools::GetFilenamePath( outName ) ) )
    {
      parameters.m_Misc.m_OutputPath = itksys::SystemTools::GetFilenamePath( outName ) + '/';
    }
    else
    {
      parameters.m_Misc.m_OutputPath = mitk::IOUtil::GetTempPath() + '/';
    }

    if( ! itksys::SystemTools::GetFilenameName( outName ).empty() )
    {
      parameters.m_Misc.m_OutputPrefix = itksys::SystemTools::GetFilenameName( outName );
    }
    else
    {
      parameters.m_Misc.m_OutputPrefix = "fiberfox";
    }

    outName = parameters.m_Misc.m_OutputPath + parameters.m_Misc.m_OutputPrefix;
  }

  // check if log file already exists and avoid overwriting existing files:
  std::string NameTest = outName;
  int c = 0;
  while( itksys::SystemTools::FileExists( outName + ".log" )
         && c <= std::numeric_limits<int>::max() )
  {
    outName = NameTest + "_" + boost::lexical_cast<std::string>(c);
    ++c;
  }

  if (verbose)
  {
    MITK_DEBUG << outName << ".ffp";
    parameters.SaveParameters(outName+".ffp");
  }

  mitk::BaseData::Pointer inputData = mitk::IOUtil::Load(input)[0];

  itk::TractsToDWIImageFilter< short >::Pointer tractsToDwiFilter = itk::TractsToDWIImageFilter< short >::New();

  if ( dynamic_cast<mitk::FiberBundle*>(inputData.GetPointer()) )   // simulate dataset from fibers
  {
    tractsToDwiFilter->SetFiberBundle(dynamic_cast<mitk::FiberBundle*>(inputData.GetPointer()));
  }
  else if ( dynamic_cast<mitk::Image*>(inputData.GetPointer()) )  // add artifacts to existing image
  {
    typedef itk::VectorImage< short, 3 >    ItkDwiType;
    mitk::Image::Pointer diffImg = dynamic_cast<mitk::Image*>(inputData.GetPointer());
    ItkDwiType::Pointer itkVectorImagePointer = ItkDwiType::New();
    mitk::CastToItkImage(diffImg, itkVectorImagePointer);

    parameters.m_SignalGen.m_SignalScale = 1;
    parameters.m_SignalGen.m_ImageRegion = itkVectorImagePointer->GetLargestPossibleRegion();
    parameters.m_SignalGen.m_ImageSpacing = itkVectorImagePointer->GetSpacing();
    parameters.m_SignalGen.m_ImageOrigin = itkVectorImagePointer->GetOrigin();
    parameters.m_SignalGen.m_ImageDirection = itkVectorImagePointer->GetDirection();
    parameters.m_SignalGen.m_Bvalue = static_cast<mitk::FloatProperty*>
        (diffImg->GetProperty(mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str()).GetPointer() )
        ->GetValue();
    parameters.m_SignalGen.SetGradienDirections( static_cast<mitk::GradientDirectionsProperty*>
        ( diffImg->GetProperty(mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str()).GetPointer() )
        ->GetGradientDirectionsContainer() );

    tractsToDwiFilter->SetInputImage(itkVectorImagePointer);
  }
  tractsToDwiFilter->SetParameters(parameters);
  tractsToDwiFilter->Update();

  mitk::Image::Pointer image = mitk::GrabItkImageMemory( tractsToDwiFilter->GetOutput() );
  image->SetProperty( mitk::DiffusionPropertyHelper::GRADIENTCONTAINERPROPERTYNAME.c_str(),
                      mitk::GradientDirectionsProperty::New( parameters.m_SignalGen.GetGradientDirections() ) );
  image->SetProperty( mitk::DiffusionPropertyHelper::REFERENCEBVALUEPROPERTYNAME.c_str(),
                      mitk::FloatProperty::New( parameters.m_SignalGen.m_Bvalue ) );
  mitk::DiffusionPropertyHelper propertyHelper( image );
  propertyHelper.InitializeImage();

  mitk::IOUtil::Save(image, outName+".dwi");

  if (verbose)
  {
    std::vector< itk::TractsToDWIImageFilter< short >::ItkDoubleImgType::Pointer > volumeFractions = tractsToDwiFilter->GetVolumeFractions();
    for (unsigned int k=0; k<volumeFractions.size(); k++)
    {
      mitk::Image::Pointer image = mitk::Image::New();
      image->InitializeByItk(volumeFractions.at(k).GetPointer());
      image->SetVolume(volumeFractions.at(k)->GetBufferPointer());
      mitk::IOUtil::Save(image, outName+"_Compartment"+boost::lexical_cast<string>(k)+".nrrd");
    }

    if (tractsToDwiFilter->GetPhaseImage().IsNotNull())
    {
      mitk::Image::Pointer image = mitk::Image::New();
      itk::TractsToDWIImageFilter< short >::DoubleDwiType::Pointer itkPhase = tractsToDwiFilter->GetPhaseImage();
      image = mitk::GrabItkImageMemory( itkPhase.GetPointer() );
      mitk::IOUtil::Save(image, outName+"_Phase.nrrd");
    }

    if (tractsToDwiFilter->GetKspaceImage().IsNotNull())
    {
      mitk::Image::Pointer image = mitk::Image::New();
      itk::TractsToDWIImageFilter< short >::DoubleDwiType::Pointer itkImage = tractsToDwiFilter->GetKspaceImage();
      image = mitk::GrabItkImageMemory( itkImage.GetPointer() );
      mitk::IOUtil::Save(image, outName+"_kSpace.nrrd");
    }
  }

  return EXIT_SUCCESS;
}

