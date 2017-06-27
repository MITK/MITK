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

#include <mitkSplitParameterToVector.h>

#include <QApplication>
#include <mitkStandaloneDataStorage.h>
#include "QmitkRegisterClasses.h"
#include "QmitkRenderWindow.h"
#include "vtkRenderLargeImage.h"
#include "vtkPNGWriter.h"


static
void SaveSliceOrImageAsPNG(std::vector<std::string> listOfOutputs, std::string path)
{
  // Create a Standalone Datastorage for the single purpose of saving screenshots..
  mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
  QmitkRenderWindow renderWindow;
  renderWindow.GetRenderer()->SetDataStorage(ds);

  for (auto name : listOfOutputs)
  {
    mitk::Image::Pointer tmpImage = mitk::IOUtil::LoadImage(name);
    auto nodeI = mitk::DataNode::New();
    nodeI->SetData(tmpImage);
    ds->Add(nodeI);
  }

  mitk::TimeGeometry::Pointer geo = ds->ComputeBoundingGeometry3D(ds->GetAll());
  mitk::RenderingManager::GetInstance()->InitializeViews(geo);

  mitk::SliceNavigationController::Pointer sliceNaviController = renderWindow.GetSliceNavigationController();
  unsigned int numberOfSteps = 1;
  if (sliceNaviController)
  {
    numberOfSteps = sliceNaviController->GetSlice()->GetSteps();
    sliceNaviController->GetSlice()->SetPos(0);
  }

  renderWindow.show();
  renderWindow.resize(512, 512);

  for (unsigned int currentStep = 0; currentStep < numberOfSteps; ++currentStep)
  {
    if (sliceNaviController)
    {
      sliceNaviController->GetSlice()->SetPos(currentStep);
    }

    renderWindow.GetRenderer()->PrepareRender();

    vtkRenderWindow* renderWindow2 = renderWindow.GetVtkRenderWindow();
    mitk::BaseRenderer* baserenderer = mitk::BaseRenderer::GetInstance(renderWindow2);
    auto vtkRender = baserenderer->GetVtkRenderer();
    vtkRender->GetRenderWindow()->WaitForCompletion();

    vtkRenderLargeImage* magnifier = vtkRenderLargeImage::New();
    magnifier->SetInput(vtkRender);
    magnifier->SetMagnification(3.0);

    std::stringstream ss;
    ss << path << "screenshot_step-"<<currentStep<<".png";
    std::string tmpImageName;
    ss >> tmpImageName;
    auto fileWriter = vtkPNGWriter::New();
    fileWriter->SetInputConnection(magnifier->GetOutputPort());
    fileWriter->SetFileName(tmpImageName.c_str());
    fileWriter->Write();
    fileWriter->Delete();
  }
}

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");

  // Required Parameter
  parser.addArgument("image", "i", mitkCommandLineParser::InputImage, "Input Image", "Path to the input image files (Separated with semicolons)", us::Any(), false);
  parser.addArgument("output", "o", mitkCommandLineParser::OutputFile, "Output text file", "Path to output file. The output statistic is appended to this file.", us::Any(), false);
  parser.addArgument("direction", "dir", mitkCommandLineParser::String, "Int", "Allows to specify the direction for Cooc and RL. 0: All directions, 1: Only single direction (Test purpose), 2,3,4... Without dimension 0,1,2... ", us::Any());

  // Miniapp Infos
  parser.setCategory("Classification Tools");
  parser.setTitle("Screenshot of a single image");
  parser.setDescription("");
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

  std::string version = "Version: 1.0";
  MITK_INFO << version;

  int direction = 0;
  if (parsedArgs.count("direction"))
  {
    direction = mitk::cl::splitDouble(parsedArgs["direction"].ToString(), ';')[0];
  }

  auto listOfFiles = mitk::cl::splitString(parsedArgs["image"].ToString(), ';');

  // Create a QTApplication and a Datastorage
  // This is necessary in order to save screenshots of
  // each image / slice.
  QApplication qtapplication(argc, argv);
  QmitkRegisterClasses();

  SaveSliceOrImageAsPNG(listOfFiles, parsedArgs["output"].ToString());

  return 0;
}

#endif
