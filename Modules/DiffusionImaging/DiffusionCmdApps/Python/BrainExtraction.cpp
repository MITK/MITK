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
#include <boost/algorithm/string.hpp>

typedef mitk::DiffusionPropertyHelper DPH;
typedef itksys::SystemTools ist;

std::string GetPythonFile(std::string filename, std::string exec_dir)
{
  std::string out = "";
  for (auto dir : mitk::bet::relative_search_dirs)
  {
    if ( ist::FileExists( exec_dir + dir + filename) )
    {
      out = exec_dir + dir + filename;
      return out;
    }
    if ( ist::FileExists( ist::GetCurrentWorkingDirectory() + dir + filename) )
    {
      out = ist::GetCurrentWorkingDirectory() + dir + filename;
      return out;
    }
  }
  for (auto dir : mitk::bet::absolute_search_dirs)
  {
    if ( ist::FileExists( dir + filename) )
    {
      out = dir + filename;
      return out;
    }
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
  parser.addArgument("", "i", mitkCommandLineParser::String, "Input:", "input image", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("", "o", mitkCommandLineParser::String, "Output:", "output root", us::Any(), false, false, false, mitkCommandLineParser::Output);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  // mandatory arguments
  std::string i = us::any_cast<std::string>(parsedArgs["i"]);
  std::string o = us::any_cast<std::string>(parsedArgs["o"]);

  std::string exec_dir = ist::GetFilenamePath(argv[0]);
  mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"Diffusion Weighted Images"}, {});
  mitk::Image::Pointer mitk_image = mitk::IOUtil::Load<mitk::Image>(i, &functor);

  bool missing_file = false;
  std::string missing_file_string = "";
  if ( GetPythonFile("run_mitk.py", exec_dir).empty() )
  {
    missing_file_string += "Brain extraction script file missing: run_mitk.py\n\n";
    missing_file = true;
  }

  if ( GetPythonFile("model_final.model", exec_dir).empty() )
  {
    missing_file_string += "Brain extraction model file missing: model_final.model\n\n";
    missing_file = true;
  }

  if ( GetPythonFile("basic_config_just_like_braintumor.py", exec_dir).empty() )
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

  m_PythonService->AddAbsoluteSearchDirs(mitk::bet::absolute_search_dirs);
  m_PythonService->AddRelativeSearchDirs(mitk::bet::relative_search_dirs);
  m_PythonService->Execute("paths=[]");

  // set input files (model and config)
  m_PythonService->Execute("model_file=\""+GetPythonFile("model_final.model", exec_dir)+"\"");
  m_PythonService->Execute("config_file=\""+GetPythonFile("basic_config_just_like_braintumor.py", exec_dir)+"\"");

  // copy input  image to python
  m_PythonService->CopyToPythonAsSimpleItkImage( mitk_image, "in_image");

  // run segmentation script
  m_PythonService->ExecuteScript( GetPythonFile("run_mitk.py", exec_dir) );

  // clean up after running script (better way than deleting individual variables?)
  if(m_PythonService->DoesVariableExist("in_image"))
    m_PythonService->Execute("del in_image");

  // check for errors
  if(!m_PythonService->GetVariable("error_string").empty())
    mitkThrow() << m_PythonService->GetVariable("error_string");

  // get output images and add to datastorage
  std::string output_variables = m_PythonService->GetVariable("output_variables");
  std::vector<std::string> outputs;
  boost::split(outputs, output_variables, boost::is_any_of(","));

  std::string output_types = m_PythonService->GetVariable("output_types");
  std::vector<std::string> types;
  boost::split(types, output_types, boost::is_any_of(","));

  for (unsigned int i=0; i<outputs.size(); ++i)
  {
    if (m_PythonService->DoesVariableExist(outputs.at(i)))
    {
      mitk::Image::Pointer image = m_PythonService->CopySimpleItkImageFromPython(outputs.at(i));

      if(types.at(i)=="input" && mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(mitk_image))
      {
        mitk::DiffusionPropertyHelper::CopyProperties(mitk_image, image, true);
        mitk::DiffusionPropertyHelper::InitializeImage(image);
      }

      mitk::DataNode::Pointer corrected_node = mitk::DataNode::New();
      corrected_node->SetData( image );
      std::string name = o + "_";
      name += outputs.at(i);

      if(types.at(i)=="input" && mitk::DiffusionPropertyHelper::IsDiffusionWeightedImage(mitk_image))
        mitk::IOUtil::Save(image, "DWI_NIFTI", name+".nii.gz");
      else
        mitk::IOUtil::Save(image, name+".nii.gz");
    }
  }

  MITK_INFO << "Finished brain extraction";
  return EXIT_SUCCESS;
}
