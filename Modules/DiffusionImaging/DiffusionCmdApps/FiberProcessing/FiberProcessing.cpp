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

#include <itkImageFileWriter.h>
#include <itkMetaDataObject.h>
#include <itkVectorImage.h>

#include <mitkBaseData.h>
#include <mitkFiberBundle.h>
#include "mitkCommandLineParser.h"
#include <mitkLexicalCast.h>
#include <mitkCoreObjectFactory.h>
#include <mitkIOUtil.h>
#include <itkFiberCurvatureFilter.h>


mitk::FiberBundle::Pointer LoadFib(std::string filename)
{
  std::vector<mitk::BaseData::Pointer> fibInfile = mitk::IOUtil::Load(filename);
  if( fibInfile.empty() )
    std::cout << "File " << filename << " could not be read!";
  mitk::BaseData::Pointer baseData = fibInfile.at(0);
  return dynamic_cast<mitk::FiberBundle*>(baseData.GetPointer());
}

/*!
\brief Modify input tractogram: fiber resampling, compression, pruning and transformation.
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Fiber Processing");
  parser.setCategory("Fiber Tracking and Processing Methods");
  parser.setDescription("Modify input tractogram: fiber resampling, compression, pruning and transformation.");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");

  parser.beginGroup("1. Mandatory arguments:");
  parser.addArgument("", "i", mitkCommandLineParser::String, "Input:", "Input fiber bundle (.fib, .trk, .tck)", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("", "o", mitkCommandLineParser::String, "Output:", "Output fiber bundle (.fib, .trk)", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.endGroup();

  parser.beginGroup("2. Resampling:");
  parser.addArgument("spline_resampling", "", mitkCommandLineParser::Float, "Spline resampling:", "Resample fiber using splines with the given point distance (in mm)");
  parser.addArgument("linear_resampling", "", mitkCommandLineParser::Float, "Linear resampling:", "Resample fiber linearly with the given point distance (in mm)");
  parser.addArgument("num_resampling", "", mitkCommandLineParser::Int, "Num. fiber points resampling:", "Resample all fibers to the given number of points");
  parser.addArgument("compress", "", mitkCommandLineParser::Float, "Compress:", "Compress fiber using the given error threshold (in mm)");
  parser.endGroup();

  parser.beginGroup("3. Filtering:");
  parser.addArgument("min_length", "", mitkCommandLineParser::Float, "Minimum length:", "Minimum fiber length (in mm)");
  parser.addArgument("max_length", "", mitkCommandLineParser::Float, "Maximum length:", "Maximum fiber length (in mm)");
  parser.addArgument("max_angle", "", mitkCommandLineParser::Float, "Maximum angle:", "Maximum angular STDEV (in degree) over given distance");
  parser.addArgument("max_angle_dist", "", mitkCommandLineParser::Float, "Distance:", "Distance in mm", 10);
  parser.addArgument("remove", "", mitkCommandLineParser::Bool, "Remove fibers exceeding curvature threshold:", "If false, only the high curvature parts are removed");
  parser.addArgument("subsample", "", mitkCommandLineParser::Float, "Randomly select fraction of streamlines:", "Randomly select the specified fraction of streamlines from the input tractogram");
  parser.addArgument("random_subsample", "", mitkCommandLineParser::Bool, "Randomly seed subsampling:", "Randomly seed subsampling. Else, use seed 0.");
  parser.endGroup();

  parser.beginGroup("4. Transformation:");
  parser.addArgument("mirror", "", mitkCommandLineParser::Int, "Invert coordinates:", "Invert fiber coordinates XYZ (e.g. 010 to invert y-coordinate of each fiber point)");

  parser.addArgument("rotate_x", "", mitkCommandLineParser::Float, "Rotate x-axis:", "Rotate around x-axis (in deg)");
  parser.addArgument("rotate_y", "", mitkCommandLineParser::Float, "Rotate y-axis:", "Rotate around y-axis (in deg)");
  parser.addArgument("rotate_z", "", mitkCommandLineParser::Float, "Rotate z-axis:", "Rotate around z-axis (in deg)");

  parser.addArgument("scale_x", "", mitkCommandLineParser::Float, "Scale x-axis:", "Scale in direction of x-axis");
  parser.addArgument("scale_y", "", mitkCommandLineParser::Float, "Scale y-axis:", "Scale in direction of y-axis");
  parser.addArgument("scale_z", "", mitkCommandLineParser::Float, "Scale z-axis", "Scale in direction of z-axis");

  parser.addArgument("translate_x", "", mitkCommandLineParser::Float, "Translate x-axis:", "Translate in direction of x-axis (in mm)");
  parser.addArgument("translate_y", "", mitkCommandLineParser::Float, "Translate y-axis:", "Translate in direction of y-axis (in mm)");
  parser.addArgument("translate_z", "", mitkCommandLineParser::Float, "Translate z-axis:", "Translate in direction of z-axis (in mm)");
  parser.endGroup();


  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  bool remove = false;
  if (parsedArgs.count("remove"))
    remove = us::any_cast<bool>(parsedArgs["remove"]);

  bool random_subsample = false;
  if (parsedArgs.count("random_subsample"))
    random_subsample = us::any_cast<bool>(parsedArgs["random_subsample"]);

  float spline_resampling = -1;
  if (parsedArgs.count("spline_resampling"))
    spline_resampling = us::any_cast<float>(parsedArgs["spline_resampling"]);

  float linear_resampling = -1;
  if (parsedArgs.count("linear_resampling"))
    linear_resampling = us::any_cast<float>(parsedArgs["linear_resampling"]);

  int num_resampling = -1;
  if (parsedArgs.count("num_resampling"))
    num_resampling = us::any_cast<int>(parsedArgs["num_resampling"]);

  float subsample = -1;
  if (parsedArgs.count("subsample"))
    subsample = us::any_cast<float>(parsedArgs["subsample"]);

  float compress = -1;
  if (parsedArgs.count("compress"))
    compress = us::any_cast<float>(parsedArgs["compress"]);

  float minFiberLength = -1;
  if (parsedArgs.count("min_length"))
    minFiberLength = us::any_cast<float>(parsedArgs["min_length"]);

  float maxFiberLength = -1;
  if (parsedArgs.count("max_length"))
    maxFiberLength = us::any_cast<float>(parsedArgs["max_length"]);

  float max_angle_dist = 10;
  if (parsedArgs.count("max_angle_dist"))
    max_angle_dist = us::any_cast<float>(parsedArgs["max_angle_dist"]);

  float maxAngularDev = -1;
  if (parsedArgs.count("max_angle"))
    maxAngularDev = us::any_cast<float>(parsedArgs["max_angle"]);

  int axis = 0;
  if (parsedArgs.count("mirror"))
    axis = us::any_cast<int>(parsedArgs["mirror"]);

  float rotateX = 0;
  if (parsedArgs.count("rotate_x"))
    rotateX = us::any_cast<float>(parsedArgs["rotate_x"]);

  float rotateY = 0;
  if (parsedArgs.count("rotate_y"))
    rotateY = us::any_cast<float>(parsedArgs["rotate_y"]);

  float rotateZ = 0;
  if (parsedArgs.count("rotate_z"))
    rotateZ = us::any_cast<float>(parsedArgs["rotate_z"]);

  float scaleX = 0;
  if (parsedArgs.count("scale_x"))
    scaleX = us::any_cast<float>(parsedArgs["scale_x"]);

  float scaleY = 0;
  if (parsedArgs.count("scale_y"))
    scaleY = us::any_cast<float>(parsedArgs["scale_y"]);

  float scaleZ = 0;
  if (parsedArgs.count("scale_z"))
    scaleZ = us::any_cast<float>(parsedArgs["scale_z"]);

  float translateX = 0;
  if (parsedArgs.count("translate_x"))
    translateX = us::any_cast<float>(parsedArgs["translate_x"]);

  float translateY = 0;
  if (parsedArgs.count("translate_y"))
    translateY = us::any_cast<float>(parsedArgs["translate_y"]);

  float translateZ = 0;
  if (parsedArgs.count("translate_z"))
    translateZ = us::any_cast<float>(parsedArgs["translate_z"]);


  std::string inFileName = us::any_cast<std::string>(parsedArgs["i"]);
  std::string outFileName = us::any_cast<std::string>(parsedArgs["o"]);

  try
  {
    mitk::FiberBundle::Pointer fib = LoadFib(inFileName);

    if (subsample>0)
      fib = fib->SubsampleFibers(subsample, random_subsample);

    if (maxAngularDev>0)
    {
      auto filter = itk::FiberCurvatureFilter::New();
      filter->SetInputFiberBundle(fib);
      filter->SetAngularDeviation(maxAngularDev);
      filter->SetDistance(max_angle_dist);
      filter->SetRemoveFibers(remove);
      filter->Update();
      fib = filter->GetOutputFiberBundle();
    }

    if (minFiberLength>0)
      fib->RemoveShortFibers(minFiberLength);

    if (maxFiberLength>0)
      fib->RemoveLongFibers(maxFiberLength);

    if (spline_resampling>0)
      fib->ResampleSpline(spline_resampling);

    if (linear_resampling>0)
      fib->ResampleLinear(linear_resampling);

    if (num_resampling>0)
      fib->ResampleToNumPoints(num_resampling);

    if (compress>0)
      fib->Compress(compress);

    if (axis/100==1)
      fib->MirrorFibers(0);

    if ((axis%100)/10==1)
      fib->MirrorFibers(1);

    if (axis%10==1)
      fib->MirrorFibers(2);


    if (rotateX > 0 || rotateY > 0 || rotateZ > 0){
      std::cout << "Rotate " << rotateX << " " << rotateY << " " << rotateZ;
      fib->RotateAroundAxis(rotateX, rotateY, rotateZ);
    }
    if (translateX > 0 || translateY > 0 || translateZ > 0){
      fib->TranslateFibers(translateX, translateY, translateZ);
    }
    if (scaleX > 0 || scaleY > 0 || scaleZ > 0)
      fib->ScaleFibers(scaleX, scaleY, scaleZ);

    mitk::IOUtil::Save(fib.GetPointer(), outFileName );

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
