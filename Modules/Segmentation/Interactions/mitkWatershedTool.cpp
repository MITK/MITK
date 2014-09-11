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

#include "mitkWatershedTool.h"

#include "mitkBinaryThresholdTool.xpm"
#include "mitkToolManager.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"
#include "mitkITKImageImport.h"
#include "mitkRenderingManager.h"
#include <mitkSliceNavigationController.h>
#include "mitkRenderingModeProperty.h"
#include "mitkLookupTable.h"
#include "mitkLookupTableProperty.h"
#include "mitkIOUtil.h"
#include "mitkLevelWindowManager.h"
#include "mitkImageStatisticsHolder.h"
#include "mitkToolCommand.h"
#include "mitkProgressBar.h"
#include "mitkImage.h"

#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>

#include <vtkLookupTable.h>

#include <itkWatershedImageFilter.h>
#include <itkGradientMagnitudeRecursiveGaussianImageFilter.h>
#include <itkExceptionObject.h>

namespace mitk {
  MITK_TOOL_MACRO(MitkSegmentation_EXPORT, WatershedTool, "Watershed tool");
}


mitk::WatershedTool::WatershedTool() :
  m_Level(0.),
  m_Threshold(0.)
{
}

mitk::WatershedTool::~WatershedTool()
{
}

void mitk::WatershedTool::Activated()
{
  Superclass::Activated();
}

void mitk::WatershedTool::Deactivated()
{
  Superclass::Deactivated();
}

us::ModuleResource mitk::WatershedTool::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Watershed_48x48.png");
  return resource;
}

const char** mitk::WatershedTool::GetXPM() const
{
  return NULL;
}

const char* mitk::WatershedTool::GetName() const
{
  return "Watershed";
}

void mitk::WatershedTool::DoIt()
{

  // get image from tool manager
  mitk::DataNode::Pointer referenceData = m_ToolManager->GetReferenceData(0);
  mitk::Image::Pointer input = dynamic_cast<mitk::Image*>(referenceData->GetData());
  if (input.IsNull())
    return;

  unsigned int timestep = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetTime()->GetPos();
  input = Get3DImage(input, timestep);

  mitk::Image::Pointer output;

  try {
    // create and run itk filter pipeline
    AccessByItk_1(input.GetPointer(),ITKWatershed,output);

    // create a new datanode for output
    mitk::DataNode::Pointer dataNode = mitk::DataNode::New();
    dataNode->SetData(output);

    // set properties of datanode
    dataNode->SetProperty("binary", mitk::BoolProperty::New(false));
    dataNode->SetProperty("name", mitk::StringProperty::New("Watershed Result"));
    mitk::RenderingModeProperty::Pointer renderingMode = mitk::RenderingModeProperty::New();
    renderingMode->SetValue( mitk::RenderingModeProperty::LOOKUPTABLE_LEVELWINDOW_COLOR );
    dataNode->SetProperty("Image Rendering.Mode", renderingMode);

    // since we create a multi label image, define a vtk lookup table
    mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
    mitk::LookupTableProperty::Pointer prop = mitk::LookupTableProperty::New(lut);
    vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
    lookupTable->SetHueRange(1.0, 0.0);
    lookupTable->SetSaturationRange(1.0, 1.0);
    lookupTable->SetValueRange(1.0, 1.0);
    lookupTable->SetTableRange(-1.0, 1.0);
    lookupTable->Build();
    lookupTable->SetTableValue(1,0,0,0);
    lut->SetVtkLookupTable(lookupTable);
    prop->SetLookupTable(lut);
    dataNode->SetProperty("LookupTable",prop);

    // make the levelwindow fit to right values
    mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
    mitk::LevelWindow levelwindow;
    levelwindow.SetRangeMinMax(0, output->GetStatistics()->GetScalarValueMax());
    levWinProp->SetLevelWindow( levelwindow );
    dataNode->SetProperty( "levelwindow", levWinProp );
    dataNode->SetProperty( "opacity", mitk::FloatProperty::New(0.5));

    // set name of data node
    std::string name = referenceData->GetName() + "_Watershed";
    dataNode->SetName( name );

    // look, if there is already a node with this name
    mitk::DataStorage::SetOfObjects::ConstPointer children = m_ToolManager->GetDataStorage()->GetDerivations(referenceData);
    mitk::DataStorage::SetOfObjects::ConstIterator currentNode = children->Begin();
    mitk::DataNode::Pointer removeNode;
    while(currentNode != children->End())
    {
      if(dataNode->GetName().compare(currentNode->Value()->GetName()) == 0)
      {
        removeNode = currentNode->Value();
      }
      currentNode++;
    }
    // remove node with same name
    if(removeNode.IsNotNull())
      m_ToolManager->GetDataStorage()->Remove(removeNode);

    // add output to the data storage
    m_ToolManager->GetDataStorage()->Add(dataNode,referenceData);
  }
  catch(itk::ExceptionObject& e)
  {
    MITK_ERROR<<"Watershed Filter Error: " << e.GetDescription();
  }

  RenderingManager::GetInstance()->RequestUpdateAll();

}

template <typename TPixel, unsigned int VImageDimension>
void mitk::WatershedTool::ITKWatershed( itk::Image<TPixel, VImageDimension>* originalImage, mitk::Image::Pointer& segmentation )
{
  typedef itk::WatershedImageFilter< itk::Image<float, VImageDimension> > WatershedFilter;
  typedef itk::GradientMagnitudeRecursiveGaussianImageFilter< itk::Image<TPixel, VImageDimension >, itk::Image<float, VImageDimension> > MagnitudeFilter;

  // at first add a gradient magnitude filter
  typename MagnitudeFilter::Pointer magnitude = MagnitudeFilter::New();
  magnitude->SetInput(originalImage);
  magnitude->SetSigma(1.0);

  // use the progress bar
  mitk::ToolCommand::Pointer command = mitk::ToolCommand::New();
  command->AddStepsToDo(60);

  // then add the watershed filter to the pipeline
  typename WatershedFilter::Pointer watershed = WatershedFilter::New();
  watershed->SetInput(magnitude->GetOutput());
  watershed->SetThreshold(m_Threshold);
  watershed->SetLevel(m_Level);
  watershed->AddObserver(itk::ProgressEvent(),command);
  watershed->Update();

  // then make sure, that the output has the desired pixel type
  typedef itk::CastImageFilter<typename WatershedFilter::OutputImageType, itk::Image<Tool::DefaultSegmentationDataType,VImageDimension> > CastFilter;
  typename CastFilter::Pointer cast = CastFilter::New();
  cast->SetInput(watershed->GetOutput());

  // start the whole pipeline
  cast->Update();

  // reset the progress bar by setting progress
  command->SetProgress(10);

  // since we obtain a new image from our pipeline, we have to make sure, that our mitk::Image::Pointer
  // is responsible for the memory management of the output image
  segmentation = mitk::GrabItkImageMemory(cast->GetOutput());

}
