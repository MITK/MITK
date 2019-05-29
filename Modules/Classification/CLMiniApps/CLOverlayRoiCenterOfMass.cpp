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
#include <mitkGlobalImageFeaturesParameter.h>
#include <itkImageRegionIteratorWithIndex.h>

#include <QApplication>
#include <mitkStandaloneDataStorage.h>
#include "QmitkRegisterClasses.h"
#include "QmitkRenderWindow.h"
#include "vtkRenderLargeImage.h"
#include "vtkPNGWriter.h"

#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>

typedef itk::Image< double, 3 >                 FloatImageType;
typedef itk::Image< unsigned char, 3 >          MaskImageType;


template<typename TPixel, unsigned int VImageDimension>
static void
FindMostSampleSlice(itk::Image<TPixel, VImageDimension>* mask, int & selectedSlice)
{
  int idx = VImageDimension - 1;

  int size = mask->GetLargestPossibleRegion().GetSize()[idx];
  std::vector<int> numberOfSamples;
  numberOfSamples.resize(size,0);

  itk::ImageRegionIteratorWithIndex<itk::Image<TPixel, VImageDimension> > mask1Iter(mask, mask->GetLargestPossibleRegion());
  while (!mask1Iter.IsAtEnd())
  {
    if (mask1Iter.Value() > 0)
    {
      numberOfSamples[mask1Iter.GetIndex()[idx]]+=1;
    }
    ++mask1Iter;
  }
  selectedSlice = 0;
  for (std::size_t i = 0; i < numberOfSamples.size(); ++i)
  {
    if (numberOfSamples[selectedSlice] < numberOfSamples[i])
      selectedSlice = i;
  }
}

static
void SaveSliceOrImageAsPNG(mitk::Image::Pointer image, mitk::Image::Pointer mask, std::string path, int index)
{
  // Create a Standalone Datastorage for the single purpose of saving screenshots..
  mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
  QmitkRenderWindow renderWindow;
  renderWindow.GetRenderer()->SetDataStorage(ds);

  auto nodeI = mitk::DataNode::New();
  nodeI->SetData(image);
  auto nodeM = mitk::DataNode::New();
  nodeM->SetData(mask);
  ds->Add(nodeI);
  ds->Add(nodeM);

  auto geo = ds->ComputeBoundingGeometry3D(ds->GetAll());
  mitk::RenderingManager::GetInstance()->InitializeViews(
    mask->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);

  mitk::SliceNavigationController::Pointer sliceNaviController = renderWindow.GetSliceNavigationController();
  sliceNaviController->SetViewDirection(mitk::SliceNavigationController::Axial);
  unsigned int numberOfSteps = 1;
  if (sliceNaviController)
  {
    numberOfSteps = sliceNaviController->GetSlice()->GetSteps();
    sliceNaviController->GetSlice()->SetPos(numberOfSteps-index);
  }

  renderWindow.show();
  renderWindow.resize(256, 256);

  //if (sliceNaviController)
  //{
  //  sliceNaviController->GetSlice()->SetPos(index);
  //}
  renderWindow.GetRenderer()->PrepareRender();

  vtkRenderWindow* renderWindow2 = renderWindow.GetVtkRenderWindow();
  mitk::BaseRenderer* baserenderer = mitk::BaseRenderer::GetInstance(renderWindow2);
  auto vtkRender = baserenderer->GetVtkRenderer();
  vtkRender->GetRenderWindow()->WaitForCompletion();

  vtkRenderLargeImage* magnifier = vtkRenderLargeImage::New();
  magnifier->SetInput(vtkRender);
  magnifier->SetMagnification(3.0);

  std::stringstream ss;
  ss << path <<".png";
  std::string tmpImageName;
  ss >> tmpImageName;
  auto fileWriter = vtkPNGWriter::New();
  fileWriter->SetInputConnection(magnifier->GetOutputPort());
  fileWriter->SetFileName(tmpImageName.c_str());
  fileWriter->Write();
  fileWriter->Delete();
}

int main(int argc, char* argv[])
{
  mitkCommandLineParser parser;
  parser.setArgumentPrefix("--", "-");

  parser.addArgument("image", "i", mitkCommandLineParser::Image, "Input Image", "", us::Any(),false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("mask", "m", mitkCommandLineParser::Image, "Input Image", "", us::Any(),false, false, false, mitkCommandLineParser::Input);
  parser.addArgument("output", "o", mitkCommandLineParser::Image, "Output Image", "", us::Any(),false, false, false, mitkCommandLineParser::Input);

  // Miniapp Infos
  parser.setCategory("Classification Tools");
  parser.setTitle("Image with Overlay Plotter");
  parser.setDescription("Plots ");
  parser.setContributor("MBI");

  std::map<std::string, us::Any> parsedArgs = parser.parseArguments(argc, argv);

  std::string imagePath = us::any_cast<std::string>(parsedArgs["image"]);
  std::string maskPath = us::any_cast<std::string>(parsedArgs["mask"]);
  std::string outputPath = us::any_cast<std::string>(parsedArgs["output"]);

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

  mitk::Image::Pointer image = mitk::IOUtil::Load<mitk::Image>(imagePath);
  mitk::Image::Pointer mask = mitk::IOUtil::Load<mitk::Image>(maskPath);

  // Create a QTApplication and a Datastorage
  // This is necessary in order to save screenshots of
  // each image / slice.
  QApplication qtapplication(argc, argv);
  QmitkRegisterClasses();

  int currentSlice = 0;
  AccessByItk_1(mask, FindMostSampleSlice, currentSlice);

  SaveSliceOrImageAsPNG(image, mask, outputPath, currentSlice);

  return 0;
}

#endif
