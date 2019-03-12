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

/*!
\brief Copies transformation matrix of one image to another
*/
int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;

  parser.setTitle("Copy Geometry");
  parser.setCategory("Preprocessing Tools");
  parser.setDescription("Copies transformation matrix of one image to another");
  parser.setContributor("MIC");

  parser.setArgumentPrefix("--", "-");
  parser.addArgument("", "i", mitkCommandLineParser::String, "Input:", "input image", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("", "o", mitkCommandLineParser::String, "Output:", "output image", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("ref", "r", mitkCommandLineParser::String, "Reference:", "reference image", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("alignCentroid", "a", mitkCommandLineParser::Bool, "align centroids", "align centroids", us::Any(), true);

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
  if (parsedArgs.size()==0)
    return EXIT_FAILURE;

  // mandatory arguments
  std::string imageName = us::any_cast<std::string>(parsedArgs["i"]);
  std::string refImage = us::any_cast<std::string>(parsedArgs["ref"]);
  std::string outImage = us::any_cast<std::string>(parsedArgs["o"]);

  bool originOnly = false;

  // Show a help message
  if ( parsedArgs.count("alignCentroid") || parsedArgs.count("a"))
  {
    originOnly = true;
  }

  try
  {
    mitk::Image::Pointer source = mitk::IOUtil::Load<mitk::Image>(refImage);
    mitk::Image::Pointer target = mitk::IOUtil::Load<mitk::Image>(imageName);
    if (originOnly)
    {
      // Calculate correction to align centroids
      double c[3];
      c[0] = source->GetGeometry()->GetOrigin()[0]
             + source->GetGeometry()->GetExtent(0)/2.0
             - target->GetGeometry()->GetOrigin()[0]
             - target->GetGeometry()->GetExtent(0)/2.0;

      c[1] = source->GetGeometry()->GetOrigin()[1]
             + source->GetGeometry()->GetExtent(1)/2.0
             - target->GetGeometry()->GetOrigin()[1]
             - target->GetGeometry()->GetExtent(1)/2.0;


      c[2] = source->GetGeometry()->GetOrigin()[2]
             + source->GetGeometry()->GetExtent(2)/2.0
             - target->GetGeometry()->GetOrigin()[2]
             - target->GetGeometry()->GetExtent(2)/2.0;

      double newOrigin[3];
      newOrigin[0] = target->GetGeometry()->GetOrigin()[0] +c[0];
      newOrigin[1] = target->GetGeometry()->GetOrigin()[1] +c[1];
      newOrigin[2] = target->GetGeometry()->GetOrigin()[2] +c[2];

      target->GetGeometry()->SetOrigin(newOrigin);
    }
    else
    {
      mitk::BaseGeometry* s_geom = source->GetGeometry();
      mitk::BaseGeometry* t_geom = target->GetGeometry();

      t_geom->SetIndexToWorldTransform(s_geom->GetIndexToWorldTransform());
      target->SetGeometry(t_geom);
    }
    mitk::IOUtil::Save(target, outImage);
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
