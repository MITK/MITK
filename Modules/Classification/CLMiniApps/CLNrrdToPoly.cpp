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
#ifndef mitkCLPolyToNrrd_cpp
#define mitkCLPolyToNrrd_cpp

#include "time.h"
#include <sstream>
#include <fstream>

#include <mitkIOUtil.h>
#include "mitkCommandLineParser.h"

// VTK
#include <vtkSmartPointer.h>
#include <vtkImageMarchingCubes.h>
#include <vtkXMLPolyDataWriter.h>

typedef itk::Image< double, 3 >                 FloatImageType;
typedef itk::Image< unsigned char, 3 >          MaskImageType;


int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  // required params
  parser.addArgument("mask", "m", mitkCommandLineParser::Image, "Input Mask", "Mask Image that specifies the area over for the statistic, (Values = 1)", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output text file", "Target file. The output statistic is appended to this file.", us::Any(), false, false, false, mitkCommandLineParser::Output);

  // Miniapp Infos
  parser.setCategory("Classification Tools");
  parser.setTitle("Segmentation to Mask");
  parser.setDescription("Estimates a Mesh from a segmentation");
  parser.setContributor("MBI");

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  if (parsedArgs.size()==0)
  {
    return EXIT_FAILURE;
  }
  if ( parsedArgs.count("help") || parsedArgs.count("h"))
  {
    return EXIT_SUCCESS;
  }

  MITK_INFO << "Version: "<< 1.0;

  mitk::Image::Pointer mask = mitk::IOUtil::Load<mitk::Image>(parsedArgs["mask"].ToString());


  vtkSmartPointer<vtkImageData> image = mask->GetVtkImageData();
  image->SetOrigin(mask->GetGeometry()->GetOrigin()[0], mask->GetGeometry()->GetOrigin()[1], mask->GetGeometry()->GetOrigin()[2]);
  vtkSmartPointer<vtkImageMarchingCubes> mesher = vtkSmartPointer<vtkImageMarchingCubes>::New();
  mesher->SetInputData(image);
  mitk::Surface::Pointer surf = mitk::Surface::New();
  mesher->SetValue(0,0.5);
  mesher->Update();
  surf->SetVtkPolyData(mesher->GetOutput());
  mitk::IOUtil::Save(surf, parsedArgs["output"].ToString());

  return 0;
}

#endif
