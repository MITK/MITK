/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkWatershedTool.h"

#include "mitkIOUtil.h"
#include "mitkITKImageImport.h"
#include "mitkImage.h"
#include "mitkLabelSetImage.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"
#include "mitkImageStatisticsHolder.h"
#include "mitkLevelWindowManager.h"
#include "mitkLookupTable.h"
#include "mitkLookupTableProperty.h"
#include "mitkProgressBar.h"
#include "mitkRenderingManager.h"
#include "mitkRenderingModeProperty.h"
#include "mitkToolCommand.h"
#include "mitkToolManager.h"
#include <mitkSliceNavigationController.h>

#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

#include <vtkLookupTable.h>

#include <itkExceptionObject.h>
#include <itkGradientMagnitudeRecursiveGaussianImageFilter.h>
#include <itkWatershedImageFilter.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, WatershedTool, "Watershed tool");
}


void mitk::WatershedTool::Activated()
{
  Superclass::Activated();

  m_Level = 0.0;
  m_Threshold = 0.0;

  m_MagFilter = nullptr;
  m_WatershedFilter = nullptr;
  m_LastFilterInput = nullptr;
}

us::ModuleResource mitk::WatershedTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Watershed_48x48.png");
  return resource;
}

const char **mitk::WatershedTool::GetXPM() const
{
  return nullptr;
}

const char *mitk::WatershedTool::GetName() const
{
  return "Watershed";
}

mitk::LabelSetImage::Pointer mitk::WatershedTool::ComputeMLPreview(const Image* inputAtTimeStep, TimeStepType /*timeStep*/)
{
  mitk::LabelSetImage::Pointer labelSetOutput;

  try
  {
    mitk::Image::Pointer output;
    bool inputChanged = inputAtTimeStep != m_LastFilterInput;
    // create and run itk filter pipeline
    AccessByItk_2(inputAtTimeStep, ITKWatershed, output, inputChanged);

    labelSetOutput = mitk::LabelSetImage::New();
    labelSetOutput->InitializeByLabeledImage(output);
  }
  catch (itk::ExceptionObject & e)
  {
    //force reset of filters as they might be in an invalid state now.
    m_MagFilter = nullptr;
    m_WatershedFilter = nullptr;
    m_LastFilterInput = nullptr;

    MITK_ERROR << "Watershed Filter Error: " << e.GetDescription();
  }
  
  m_LastFilterInput = inputAtTimeStep;

  return labelSetOutput;
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::WatershedTool::ITKWatershed(const itk::Image<TPixel, VImageDimension>* originalImage,
  mitk::Image::Pointer& segmentation, bool inputChanged)
{
  typedef itk::WatershedImageFilter<itk::Image<float, VImageDimension>> WatershedFilter;
  typedef itk::GradientMagnitudeRecursiveGaussianImageFilter<itk::Image<TPixel, VImageDimension>,
    itk::Image<float, VImageDimension>>
    MagnitudeFilter;

  // We create the filter pipeline only once (if needed) and not everytime we
  // generate the ml image preview.
  // Reason: If only the levels are changed the update of the pipe line is very
  // fast and we want to profit from this feature.

  // at first add a gradient magnitude filter
  typename MagnitudeFilter::Pointer magnitude = dynamic_cast<MagnitudeFilter*>(m_MagFilter.GetPointer());
  if (magnitude.IsNull())
  {
    magnitude = MagnitudeFilter::New();
    magnitude->SetSigma(1.0);
    magnitude->AddObserver(itk::ProgressEvent(), m_ProgressCommand);
    m_MagFilter = magnitude.GetPointer();
  }

  if (inputChanged)
  {
    magnitude->SetInput(originalImage);
  }

  // then add the watershed filter to the pipeline
  typename WatershedFilter::Pointer watershed = dynamic_cast<WatershedFilter*>(m_WatershedFilter.GetPointer());
  if (watershed.IsNull())
  {
    watershed = WatershedFilter::New();
    watershed->SetInput(magnitude->GetOutput());
    watershed->AddObserver(itk::ProgressEvent(), m_ProgressCommand);
    m_WatershedFilter = watershed.GetPointer();
  }

  watershed->SetThreshold(m_Threshold);
  watershed->SetLevel(m_Level);
  watershed->Update();

  // then make sure, that the output has the desired pixel type
  typedef itk::CastImageFilter<typename WatershedFilter::OutputImageType,
    itk::Image<Tool::DefaultSegmentationDataType, VImageDimension>>
    CastFilter;
  typename CastFilter::Pointer cast = CastFilter::New();
  cast->SetInput(watershed->GetOutput());

  // start the whole pipeline
  cast->Update();

  // since we obtain a new image from our pipeline, we have to make sure, that our mitk::Image::Pointer
  // is responsible for the memory management of the output image
  segmentation = mitk::GrabItkImageMemory(cast->GetOutput());
}
