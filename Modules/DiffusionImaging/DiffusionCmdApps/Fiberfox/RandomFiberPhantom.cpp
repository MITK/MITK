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

#include <metaCommand.h>
#include "mitkCommandLineParser.h"
#include <usAny.h>
#include <mitkCoreObjectFactory.h>
#include <mitkFiberBundle.h>
#include <mitkDiffusionDataIOHelper.h>
#include <itkRandomPhantomFilter.h>

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Random Fiber Phantom");
  parser.setCategory("Diffusion Simulation Tools");
  parser.setContributor("MIC");
  parser.setDescription("Create Random Fiber Configurations");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("num_bundles", "", mitkCommandLineParser::Int, "", "", 50);
  parser.addArgument("min_density", "", mitkCommandLineParser::Int, "", "", 50);
  parser.addArgument("max_density", "", mitkCommandLineParser::Int, "", "", 200);
  parser.addArgument("size_x", "", mitkCommandLineParser::Int, "", "", 250);
  parser.addArgument("size_y", "", mitkCommandLineParser::Int, "", "", 250);
  parser.addArgument("size_z", "", mitkCommandLineParser::Int, "", "", 250);
  parser.addArgument("min_stepsize", "", mitkCommandLineParser::Int, "", "", 15);
  parser.addArgument("max_stepsize", "", mitkCommandLineParser::Int, "", "", 30);
  parser.addArgument("min_curve", "", mitkCommandLineParser::Int, "", "", 5);
  parser.addArgument("max_curve", "", mitkCommandLineParser::Int, "", "", 45);
  parser.addArgument("min_radius", "", mitkCommandLineParser::Int, "", "", 5);
  parser.addArgument("max_radius", "", mitkCommandLineParser::Int, "", "", 25);
  parser.addArgument("min_twist", "", mitkCommandLineParser::Int, "", "", 15);
  parser.addArgument("max_twist", "", mitkCommandLineParser::Int, "", "", 30);
  parser.addArgument("compress", "", mitkCommandLineParser::Float, "Compress:", "compress fiber using the given error threshold (in mm)", 0.1);
  parser.addArgument("", "o", mitkCommandLineParser::String, "Output folder:", "output folder", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("fix_seed", "", mitkCommandLineParser::Int, "Fix random seed:", "if >= 0, produce same random values on each run using this seed.", -1);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  std::string out_folder = us::any_cast<std::string>(parsedArgs["o"]);

  float compress=0.1;
  if (parsedArgs.count("compress"))
    compress = us::any_cast<float>(parsedArgs["compress"]);

  int num_bundles=50;
  if (parsedArgs.count("num_bundles"))
    num_bundles = us::any_cast<int>(parsedArgs["num_bundles"]);

  int min_density=50;
  if (parsedArgs.count("min_density"))
    min_density = us::any_cast<int>(parsedArgs["min_density"]);

  int max_density=200;
  if (parsedArgs.count("max_density"))
    max_density = us::any_cast<int>(parsedArgs["max_density"]);

  int size_x=250;
  if (parsedArgs.count("size_x"))
    size_x = us::any_cast<int>(parsedArgs["size_x"]);

  int size_y=250;
  if (parsedArgs.count("size_y"))
    size_y = us::any_cast<int>(parsedArgs["size_y"]);

  int size_z=250;
  if (parsedArgs.count("size_z"))
    size_z = us::any_cast<int>(parsedArgs["size_z"]);

  int min_stepsize=15;
  if (parsedArgs.count("min_stepsize"))
    min_stepsize = us::any_cast<int>(parsedArgs["min_stepsize"]);

  int max_stepsize=30;
  if (parsedArgs.count("max_stepsize"))
    max_stepsize = us::any_cast<int>(parsedArgs["max_stepsize"]);

  int min_curve=5;
  if (parsedArgs.count("min_curve"))
    min_curve = us::any_cast<int>(parsedArgs["min_curve"]);

  int max_curve=45;
  if (parsedArgs.count("max_curve"))
    max_curve = us::any_cast<int>(parsedArgs["max_curve"]);

  int min_radius=5;
  if (parsedArgs.count("min_radius"))
    min_radius = us::any_cast<int>(parsedArgs["min_radius"]);

  int max_radius=25;
  if (parsedArgs.count("max_radius"))
    max_radius = us::any_cast<int>(parsedArgs["max_radius"]);

  int min_twist=15;
  if (parsedArgs.count("min_twist"))
    min_twist = us::any_cast<int>(parsedArgs["min_twist"]);

  int max_twist=30;
  if (parsedArgs.count("max_twist"))
    max_twist = us::any_cast<int>(parsedArgs["max_twist"]);

  int fix_seed = -1;
  if (parsedArgs.count("fix_seed"))
    fix_seed = us::any_cast<int>(parsedArgs["fix_seed"]);

  try
  {
    itk::RandomPhantomFilter::Pointer filter = itk::RandomPhantomFilter::New();
    filter->SetNumTracts(static_cast<unsigned int>(num_bundles));
    filter->SetMinStreamlineDensity(static_cast<unsigned int>(min_density));
    filter->SetMaxStreamlineDensity(static_cast<unsigned int>(max_density));
    mitk::Vector3D vol;
    vol[0] = size_x;
    vol[1] = size_y;
    vol[2] = size_z;
    filter->SetVolumeSize(vol);
    filter->SetStepSizeMin(static_cast<unsigned int>(min_stepsize));
    filter->SetStepSizeMax(static_cast<unsigned int>(max_stepsize));
    filter->SetCurvynessMin(static_cast<unsigned int>(min_curve));
    filter->SetCurvynessMax(static_cast<unsigned int>(max_curve));
    filter->SetStartRadiusMin(static_cast<unsigned int>(min_radius));
    filter->SetStartRadiusMax(static_cast<unsigned int>(max_radius));
    filter->SetMinTwist(static_cast<unsigned int>(min_twist));
    filter->SetMaxTwist(static_cast<unsigned int>(max_twist));
    filter->SetFixSeed(fix_seed);
    filter->Update();
    auto fibs = filter->GetFiberBundles();

    std::vector< mitk::DataNode::Pointer > fiber_nodes;
    int c = 1;
    for (auto fib : fibs)
    {
      if (compress>0)
        fib->Compress(compress);
      mitk::IOUtil::Save(fib, out_folder + "Bundle_" + boost::lexical_cast<std::string>(c) + ".fib");
      ++c;
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
