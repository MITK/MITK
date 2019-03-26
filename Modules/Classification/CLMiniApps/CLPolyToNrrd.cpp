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

#include <mitkIOUtil.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <mitkSurface.h>

#include "mitkCommandLineParser.h"

#include <mitkSurfaceToImageFilter.h>
#include <vtkSmartPointer.h>
#include <vtkLinearExtrusionFilter.h>
#include <mitkSurfaceToImageFilter.h>
#include <mitkConvert2Dto3DImageFilter.h>



typedef itk::Image< double, 3 >                 FloatImageType;
typedef itk::Image< unsigned char, 3 >          MaskImageType;

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");
  // required params
  parser.addArgument("polydata", "p", mitkCommandLineParser::Directory, "Input Polydata", "Path to the input VTK polydata", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("image", "i", mitkCommandLineParser::Directory, "Input Image", "Image which defines the dimensions of the Segmentation", us::Any(), false, false, false, mitkCommandLineParser::Output);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output file", "Output files. Two files are create, a .nrrd image and a 3d-vtk.", us::Any(), false, false, false, mitkCommandLineParser::Input);
  // Miniapp Infos
  parser.setCategory("Classification Tools");
  parser.setTitle("2D-Polydata to Nrrd Segmentation");
  parser.setDescription("Creates a Nrrd segmentation based on a 2d-vtk polydata.");
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

  mitk::BaseData::Pointer data = mitk::IOUtil::Load(parsedArgs["polydata"].ToString())[0];
  mitk::Image::Pointer image = mitk::IOUtil::Load<mitk::Image>(parsedArgs["image"].ToString());

  //MITK_INFO << data;
  mitk::Surface::Pointer surf = dynamic_cast<mitk::Surface*>(data.GetPointer());
  vtkSmartPointer<vtkPolyData> circle = surf->GetVtkPolyData();

  vtkSmartPointer<vtkLinearExtrusionFilter> extruder =
      vtkSmartPointer<vtkLinearExtrusionFilter>::New();

  extruder->SetInputData(circle);
  image->GetGeometry()->GetSpacing()[2];
  extruder->SetScaleFactor(1.);
  extruder->SetExtrusionTypeToNormalExtrusion();
  extruder->SetVector(0, 0,   image->GetGeometry()->GetSpacing()[2]);
  extruder->Update();
  surf->SetVtkPolyData(extruder->GetOutput());

  mitk::SurfaceToImageFilter::Pointer surfaceToImageFilter = mitk::SurfaceToImageFilter::New();
  surfaceToImageFilter->MakeOutputBinaryOn();
  surfaceToImageFilter->SetInput(surf);
  surfaceToImageFilter->SetImage(image);
  surfaceToImageFilter->Update();
  mitk::Image::Pointer resultImage = surfaceToImageFilter->GetOutput();

  mitk::Convert2Dto3DImageFilter::Pointer multiFilter = mitk::Convert2Dto3DImageFilter::New();
  multiFilter->SetInput(resultImage);
  multiFilter->Update();
  resultImage = multiFilter->GetOutput();

  std::string saveAs = parsedArgs["output"].ToString();
  MITK_INFO << "Save as: " << saveAs;
  saveAs = saveAs + ".vtk";
  mitk::IOUtil::Save(surf.GetPointer(),saveAs);

  saveAs = parsedArgs["output"].ToString();
  MITK_INFO << "Save as: " << saveAs;
  saveAs = saveAs + ".nrrd";
  mitk::IOUtil::Save(resultImage,saveAs);

  return 0;
}

#endif
