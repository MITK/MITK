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
#include <mitkLexicalCast.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <itksys/SystemTools.hxx>
#include <mitkFiberfoxParameters.h>

using namespace mitk;

/*!
* \brief Command line interface to Fiberfox.
* Simulate a diffusion-weighted image from a tractogram using the specified parameter file.
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setTitle("Fiberfox");
  parser.setCategory("Diffusion Simulation Tools");
  parser.setContributor("MIC");
  parser.setDescription("Command line interface to Fiberfox." " Simulate a diffusion-weighted image from a tractogram using the specified parameter file.");
  parser.setArgumentPrefix("--", "-");
  parser.addArgument("", "o", mitkCommandLineParser::String, "Output root:", "output folder and file prefix", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("", "i", mitkCommandLineParser::String, "Input:", "input tractogram or diffusion-weighted image", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("parameters", "p", mitkCommandLineParser::String, "Parameter file:", "fiberfox parameter file (.ffp)", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("template", "t", mitkCommandLineParser::String, "Template image:", "use parameters of the template image", us::Any(), true, false, false, mitkCommandLineParser::Input);
  parser.addArgument("verbose", "v", mitkCommandLineParser::Bool, "Output additional images:", "output volume fraction images etc.", us::Any());
  parser.addArgument("dont_apply_direction_matrix", "", mitkCommandLineParser::Bool, "Don't apply direction matrix:", "don't rotate gradients by image direction matrix", us::Any());
  parser.addArgument("fix_seed", "", mitkCommandLineParser::Bool, "Use fix random seed:", "always use same sequence of random numbers", us::Any());

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
  {
    return EXIT_FAILURE;
  }
  std::string outName = us::any_cast<std::string>(parsedArgs["o"]);
  std::string paramName = us::any_cast<std::string>(parsedArgs["parameters"]);

  std::string input="";
  if (parsedArgs.count("i"))
    input = us::any_cast<std::string>(parsedArgs["i"]);

  bool fix_seed = false;
  if (parsedArgs.count("fix_seed"))
    fix_seed = us::any_cast<bool>(parsedArgs["fix_seed"]);

  bool verbose = false;
  if (parsedArgs.count("verbose"))
    verbose = us::any_cast<bool>(parsedArgs["verbose"]);

  bool apply_direction_matrix = true;
  if (parsedArgs.count("dont_apply_direction_matrix"))
    apply_direction_matrix = false;

  FiberfoxParameters parameters;
  parameters.LoadParameters(paramName, fix_seed);

  // Test if /path/dir is an existing directory:
  std::string file_extension = "";
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
      parameters.m_Misc.m_OutputPath = itksys::SystemTools::GetCurrentWorkingDirectory() + '/';
    }

    file_extension = itksys::SystemTools::GetFilenameExtension(outName);
    if( ! itksys::SystemTools::GetFilenameWithoutExtension( outName ).empty() )
    {
      parameters.m_Misc.m_OutputPrefix = itksys::SystemTools::GetFilenameWithoutExtension( outName );
    }
    else
    {
      parameters.m_Misc.m_OutputPrefix = "fiberfox";
    }

    outName = parameters.m_Misc.m_OutputPath + parameters.m_Misc.m_OutputPrefix;
  }

  mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"Diffusion Weighted Images", "Fiberbundles"}, {});
  mitk::BaseData::Pointer inputData = mitk::IOUtil::Load(input, &functor)[0];

  itk::TractsToDWIImageFilter< short >::Pointer tractsToDwiFilter = itk::TractsToDWIImageFilter< short >::New();

  if ( dynamic_cast<mitk::FiberBundle*>(inputData.GetPointer()) )   // simulate dataset from fibers
  {
    tractsToDwiFilter->SetFiberBundle(dynamic_cast<mitk::FiberBundle*>(inputData.GetPointer()));

    if (parsedArgs.count("template"))
    {
      MITK_INFO << "Loading template image";
      typedef itk::VectorImage< short, 3 >    ItkDwiType;
      typedef itk::Image< short, 3 >    ItkImageType;
      mitk::BaseData::Pointer templateData = mitk::IOUtil::Load(us::any_cast<std::string>(parsedArgs["template"]), &functor)[0];
      mitk::Image::Pointer template_image = dynamic_cast<mitk::Image*>(templateData.GetPointer());

      if (mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(template_image))
      {
        ItkDwiType::Pointer itkVectorImagePointer = mitk::DiffusionPropertyHelper::GetItkVectorImage(template_image);
        parameters.m_SignalGen.m_ImageRegion = itkVectorImagePointer->GetLargestPossibleRegion();
        parameters.m_SignalGen.m_ImageSpacing = itkVectorImagePointer->GetSpacing();
        parameters.m_SignalGen.m_ImageOrigin = itkVectorImagePointer->GetOrigin();
        parameters.m_SignalGen.m_ImageDirection = itkVectorImagePointer->GetDirection();
        parameters.SetBvalue(mitk::DiffusionPropertyHelper::GetReferenceBValue(template_image));
        parameters.SetGradienDirections(mitk::DiffusionPropertyHelper::GetOriginalGradientContainer(template_image));
      }
      else
      {
        ItkImageType::Pointer itkImagePointer = ItkImageType::New();
        mitk::CastToItkImage(template_image, itkImagePointer);
        parameters.m_SignalGen.m_ImageRegion = itkImagePointer->GetLargestPossibleRegion();
        parameters.m_SignalGen.m_ImageSpacing = itkImagePointer->GetSpacing();
        parameters.m_SignalGen.m_ImageOrigin = itkImagePointer->GetOrigin();
        parameters.m_SignalGen.m_ImageDirection = itkImagePointer->GetDirection();
      }
    }
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
    parameters.SetBvalue(mitk::DiffusionPropertyHelper::GetReferenceBValue(diffImg));
    parameters.SetGradienDirections(mitk::DiffusionPropertyHelper::GetOriginalGradientContainer(diffImg));

    tractsToDwiFilter->SetInputImage(itkVectorImagePointer);
  }

  if (verbose)
  {
    MITK_DEBUG << outName << ".ffp";
    parameters.m_Misc.m_OutputAdditionalImages = true;
    parameters.SaveParameters(outName+".ffp");
  }
  else
    parameters.m_Misc.m_OutputAdditionalImages = false;
  
  if (apply_direction_matrix)
  {
    MITK_INFO << "Applying direction matrix to gradient directions.";
    parameters.ApplyDirectionMatrix();
  }
  tractsToDwiFilter->SetParameters(parameters);
  tractsToDwiFilter->SetUseConstantRandSeed(fix_seed);
  tractsToDwiFilter->Update();

  mitk::Image::Pointer image = mitk::GrabItkImageMemory(tractsToDwiFilter->GetOutput());

  if (parameters.m_SignalGen.GetNumWeightedVolumes()>0)
  {
    if (apply_direction_matrix)
      mitk::DiffusionPropertyHelper::SetGradientContainer(image, parameters.m_SignalGen.GetItkGradientContainer());
    else
      mitk::DiffusionPropertyHelper::SetOriginalGradientContainer(image, parameters.m_SignalGen.GetItkGradientContainer());
    mitk::DiffusionPropertyHelper::SetReferenceBValue(image, parameters.m_SignalGen.GetBvalue());
    mitk::DiffusionPropertyHelper::InitializeImage(image);

    if (file_extension=="")
      mitk::IOUtil::Save(image, "DWI_NIFTI", outName+".nii.gz");
    else if (file_extension==".nii" || file_extension==".nii.gz")
      mitk::IOUtil::Save(image, "DWI_NIFTI", outName+file_extension);
    else
      mitk::IOUtil::Save(image, outName+file_extension);
  }
  else
    mitk::IOUtil::Save(image, outName+".nii.gz");

  if (verbose)
  {
    if (tractsToDwiFilter->GetTickImage().IsNotNull())
    {
      mitk::Image::Pointer mitkImage = mitk::Image::New();
      itk::TractsToDWIImageFilter< short >::Float2DImageType::Pointer itkImage = tractsToDwiFilter->GetTickImage();
      mitkImage = mitk::GrabItkImageMemory( itkImage.GetPointer() );
      mitk::IOUtil::Save(mitkImage, outName+"_Ticks.nii.gz");
    }

    if (tractsToDwiFilter->GetRfImage().IsNotNull())
    {
      mitk::Image::Pointer mitkImage = mitk::Image::New();
      itk::TractsToDWIImageFilter< short >::Float2DImageType::Pointer itkImage = tractsToDwiFilter->GetRfImage();
      mitkImage = mitk::GrabItkImageMemory( itkImage.GetPointer() );
      mitk::IOUtil::Save(mitkImage, outName+"_TimeFromRf.nii.gz");
    }

    std::vector< itk::TractsToDWIImageFilter< short >::ItkDoubleImgType::Pointer > volumeFractions = tractsToDwiFilter->GetVolumeFractions();
    for (unsigned int k=0; k<volumeFractions.size(); k++)
    {
      mitk::Image::Pointer image = mitk::Image::New();
      image->InitializeByItk(volumeFractions.at(k).GetPointer());
      image->SetVolume(volumeFractions.at(k)->GetBufferPointer());
      mitk::IOUtil::Save(image, outName+"_Compartment"+boost::lexical_cast<std::string>(k+1)+".nii.gz");
    }

    if (tractsToDwiFilter->GetPhaseImage().IsNotNull())
    {
      mitk::Image::Pointer image = mitk::Image::New();
      itk::TractsToDWIImageFilter< short >::DoubleDwiType::Pointer itkPhase = tractsToDwiFilter->GetPhaseImage();
      image = mitk::GrabItkImageMemory( itkPhase.GetPointer() );
      mitk::IOUtil::Save(image, outName+"_Phase.nii.gz");
    }

    if (tractsToDwiFilter->GetKspaceImage().IsNotNull())
    {
      mitk::Image::Pointer image = mitk::Image::New();
      itk::TractsToDWIImageFilter< short >::DoubleDwiType::Pointer itkImage = tractsToDwiFilter->GetKspaceImage();
      image = mitk::GrabItkImageMemory( itkImage.GetPointer() );
      mitk::IOUtil::Save(image, outName+"_kSpace.nii.gz");
    }

    int c = 1;
    std::vector< itk::TractsToDWIImageFilter< short >::DoubleDwiType::Pointer > output_real = tractsToDwiFilter->GetOutputImagesReal();
    for (auto real : output_real)
    {
      mitk::Image::Pointer image = mitk::Image::New();
      image->InitializeByItk(real.GetPointer());
      image->SetVolume(real->GetBufferPointer());
      mitk::IOUtil::Save(image, outName+"_Coil-"+boost::lexical_cast<std::string>(c)+"-real.nii.gz");
      ++c;
    }

    c = 1;
    std::vector< itk::TractsToDWIImageFilter< short >::DoubleDwiType::Pointer > output_imag = tractsToDwiFilter->GetOutputImagesImag();
    for (auto imag : output_imag)
    {
      mitk::Image::Pointer image = mitk::Image::New();
      image->InitializeByItk(imag.GetPointer());
      image->SetVolume(imag->GetBufferPointer());
      mitk::IOUtil::Save(image, outName+"_Coil-"+boost::lexical_cast<std::string>(c)+"-imag.nii.gz");
      ++c;
    }
  }

  return EXIT_SUCCESS;
}

