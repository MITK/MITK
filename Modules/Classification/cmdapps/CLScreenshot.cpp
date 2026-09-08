/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <sstream>

#include <mitkIOUtil.h>
#include <mitkCommandLineParser.h>

#include <mitkSplitParameterToVector.h>
#include <mitkProperties.h>

#include <QApplication>
#include <mitkStandaloneDataStorage.h>
#include <QmitkRegisterClasses.h>
#include <QmitkRenderWindow.h>
#include <vtkRenderLargeImage.h>
#include <vtkPNGWriter.h>


static
void SaveSliceOrImageAsPNG(std::vector<std::string> listOfOutputs, std::string path)
{
  std::vector<mitk::ColorProperty::Pointer> colorList;
  colorList.push_back(mitk::ColorProperty::New(0.9569, 0.16471, 0.25490));  // Red
  colorList.push_back(mitk::ColorProperty::New(1, 0.839, 0));               // Yellow
  colorList.push_back(mitk::ColorProperty::New(0, 0.6, 0.2));               // Green
  colorList.push_back(mitk::ColorProperty::New(0, 0.2784, 0.7255));         // BLue
  colorList.push_back(mitk::ColorProperty::New(1,0.3608,0));                // Orange
  colorList.push_back(mitk::ColorProperty::New(0.839215,0.141176,0.80784)); // Violett
  colorList.push_back(mitk::ColorProperty::New(0.1372,0.81568,0.7647));     // Turkis
  colorList.push_back(mitk::ColorProperty::New(0.61176,0.9568,0.16078));    // Bright Green
  colorList.push_back(mitk::ColorProperty::New(1,0.4274,0.16862));          // Dark Orange
  colorList.push_back(mitk::ColorProperty::New(0.88633,0.14901,0.64705));   // Pink

  // Create a Standalone Datastorage for the single purpose of saving screenshots..
  mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
  QmitkRenderWindow renderWindow;
  renderWindow.GetRenderer()->SetDataStorage(ds);

  int numberOfSegmentations = 0;
  for (auto name : listOfOutputs)
  {
    mitk::Image::Pointer tmpImage = mitk::IOUtil::Load<mitk::Image>(name);
    auto nodeI = mitk::DataNode::New();
    nodeI->SetData(tmpImage);
    bool isSegmentation = false;
    nodeI->GetPropertyValue("binary",isSegmentation);
    if (isSegmentation)
    {
      nodeI->SetProperty("color", colorList[numberOfSegmentations % colorList.size()]);
      nodeI->SetProperty("binaryimage.hoveringannotationcolor", colorList[numberOfSegmentations % colorList.size()]);
      nodeI->SetProperty("binaryimage.hoveringcolor", colorList[numberOfSegmentations % colorList.size()]);
      nodeI->SetProperty("binaryimage.selectedannotationcolor", colorList[numberOfSegmentations % colorList.size()]);
      nodeI->SetProperty("binaryimage.selectedcolor", colorList[numberOfSegmentations % colorList.size()]);
      numberOfSegmentations++;
    }
    ds->Add(nodeI);
  }

  auto geo = ds->ComputeBoundingGeometry3D(ds->GetAll());
  mitk::RenderingManager::GetInstance()->InitializeViews(geo);

  mitk::SliceNavigationController::Pointer sliceNaviController = renderWindow.GetSliceNavigationController();
  unsigned int numberOfSteps = 1;
  if (sliceNaviController)
  {
    numberOfSteps = sliceNaviController->GetStepper()->GetSteps();
    sliceNaviController->GetStepper()->SetPos(0);
  }

  renderWindow.show();
  renderWindow.resize(512, 512);

  for (unsigned int currentStep = 0; currentStep < numberOfSteps; ++currentStep)
  {
    if (sliceNaviController)
    {
      sliceNaviController->GetStepper()->SetPos(currentStep);
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
    const std::string tmpImageName = ss.str();
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
  parser.addArgument("image", "i", mitkCommandLineParser::Image, "Input Image", "Path to the input image files (Separated with semicolons)", us::Any(), false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::File, "Output prefix", "Prefix of the PNG files that are written. One file <prefix>screenshot_step-<n>.png is written per slice.", us::Any(), false, false, false, mitkCommandLineParser::Output);

  // General information about the app
  parser.setCategory("Classification Tools");
  parser.setTitle("Screenshot of a single image");
  parser.setDescription("Renders the given images together in a 2D view and saves every slice as a PNG file. Binary images are drawn as colored overlays.");
  parser.setContributor("German Cancer Research Center (DKFZ)");

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

  auto listOfFiles = mitk::cl::splitString(parsedArgs["image"].ToString(), ';');

  // Create a QTApplication and a Datastorage
  // This is necessary in order to save screenshots of
  // each image / slice.
  QApplication qtapplication(argc, argv);
  QmitkRegisterClasses();

  SaveSliceOrImageAsPNG(listOfFiles, parsedArgs["output"].ToString());

  return EXIT_SUCCESS;
}
