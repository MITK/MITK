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
#include <mitkImage.h>
#include <mitkIOUtil.h>
#include "mitkCommandLineParser.h"
#include <mitkDiffusionPropertyHelper.h>
#include <itksys/SystemTools.hxx>
#include <itkB0ImageExtractionToSeparateImageFilter.h>
#include <mitkIPythonService.h>
#include <mitkImageCast.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModule.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>
#include <BetData.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>

typedef mitk::DiffusionPropertyHelper DPH;
typedef itksys::SystemTools ist;

std::string GetPythonFile(std::string filename)
{
  std::string out = "";

  for (auto dir : mitk::bet::relative_search_dirs)
    if ( ist::FileExists( ist::GetCurrentWorkingDirectory() + dir + filename) )
    {
      out = ist::GetCurrentWorkingDirectory() + dir + filename;
      return out;
    }
  for (auto dir : mitk::bet::absolute_search_dirs)
    if ( ist::FileExists( dir + filename) )
    {
      out = dir + filename;
      return out;
    }

  return out;
}

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("BrainExtraction");
  parser.setCategory("Preprocessing Tools");
  parser.setDescription("Performs brain extraction using a deep learning model");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("", "i", mitkCommandLineParser::InputFile, "Input:", "input image", us::Any(), false);
  parser.addArgument("", "o", mitkCommandLineParser::OutputFile, "Output:", "output root", us::Any(), false);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  // mandatory arguments
  std::string i = us::any_cast<std::string>(parsedArgs["i"]);
  std::string o = us::any_cast<std::string>(parsedArgs["o"]);

  mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"Diffusion Weighted Images"}, {});
  mitk::Image::Pointer mitk_image = mitk::IOUtil::Load<mitk::Image>(i, &functor);

  bool missing_file = false;
  std::string missing_file_string = "";
  if ( GetPythonFile("run_mitk.py").empty() )
  {
    missing_file_string += "Brain extraction script file missing: run_mitk.py\n\n";
    missing_file = true;
  }

  if ( GetPythonFile("model_final.model").empty() )
  {
    missing_file_string += "Brain extraction model file missing: model_final.model\n\n";
    missing_file = true;
  }

  if ( GetPythonFile("basic_config_just_like_braintumor.py").empty() )
  {
    missing_file_string += "Config file missing: basic_config_just_like_braintumor.py\n\n";
    missing_file = true;
  }

  if (missing_file)
  {
    mitkThrow() << missing_file_string;
  }

  us::ModuleContext* context = us::GetModuleContext();
  us::ServiceReference<mitk::IPythonService> m_PythonServiceRef = context->GetServiceReference<mitk::IPythonService>();
  mitk::IPythonService* m_PythonService = dynamic_cast<mitk::IPythonService*> ( context->GetService<mitk::IPythonService>(m_PythonServiceRef) );
  mitk::IPythonService::ForceLoadModule();

  // load essential modules
  m_PythonService->Execute("import SimpleITK as sitk");
  m_PythonService->Execute("import SimpleITK._SimpleITK as _SimpleITK");
  m_PythonService->Execute("import numpy");

  // extend python search path
  std::string pythonpath = "";
  for (auto dir : mitk::bet::relative_search_dirs)
    pythonpath += "','" + ist::GetCurrentWorkingDirectory() + dir;
  for (auto dir : mitk::bet::absolute_search_dirs)
    pythonpath += "','" + dir;
  m_PythonService->Execute("paths=['"+pythonpath+"']");

  // set input files (model and config)
  m_PythonService->Execute("model_file=\""+GetPythonFile("model_final.model")+"\"");
  m_PythonService->Execute("config_file=\""+GetPythonFile("basic_config_just_like_braintumor.py")+"\"");

  // copy input  image to python
  m_PythonService->CopyToPythonAsSimpleItkImage( mitk_image, "in_image");

  // run segmentation script
  m_PythonService->ExecuteScript( GetPythonFile("run_mitk.py") );

  // clean up after running script (better way than deleting individual variables?)
  if(m_PythonService->DoesVariableExist("in_image"))
    m_PythonService->Execute("del in_image");

  // check for errors
  if(!m_PythonService->GetVariable("error_string").empty())
    mitkThrow() << m_PythonService->GetVariable("error_string");

  {
    mitk::Image::Pointer image = m_PythonService->CopySimpleItkImageFromPython("brain_mask");
    mitk::IOUtil::Save(image, o + "_BrainMask.nii.gz");
  }

  {
    mitk::Image::Pointer image = m_PythonService->CopySimpleItkImageFromPython("brain_extracted");

    if(mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(mitk_image))
    {
      mitk::DiffusionPropertyHelper::CopyProperties(mitk_image, image, true);
      mitk::DiffusionPropertyHelper::InitializeImage(image);
      mitk::IOUtil::Save(image, "application/vnd.mitk.nii.gz", o + "_SkullStripped.nii.gz");
    }
    else
      mitk::IOUtil::Save(image, o + "_SkullStripped.nii.gz");
  }

  return EXIT_SUCCESS;
}
