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

#include "mitkMedianTool3D.h"

#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"
#include "mitkInteractionConst.h"
#include "mitkApplyDiffImageOperation.h"
#include "mitkOperationEvent.h"
#include "mitkDiffImageApplier.h"
#include "mitkUndoController.h"
#include "mitkImageAccessByItk.h"
#include "mitkToolManager.h"
#include "mitkImageCast.h"
#include "mitkImageTimeSelector.h"

#include <itkBinaryThresholdImageFilter.h>
#include <itkMedianImageFilter.h>

// us
#include "mitkModule.h"
#include "mitkModuleResource.h"
#include <mitkGetModuleContext.h>

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, MedianTool3D, "MedianTool3D tool");
}


mitk::MedianTool3D::MedianTool3D()
{
}

mitk::MedianTool3D::~MedianTool3D()
{
}

const char** mitk::MedianTool3D::GetXPM() const
{
  return NULL;//mitkMedianTool3D_xpm;
}

mitk::ModuleResource mitk::MedianTool3D::GetIconResource() const
{
  Module* module = GetModuleContext()->GetModule();
  ModuleResource resource = module->GetResource("MedianTool3D_48x48.png");
  return resource;
}

const char* mitk::MedianTool3D::GetName() const
{
  return "MedianTool3D";
}

void mitk::MedianTool3D::Run()
{
  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  if (!workingNode) return;

  mitk::LabelSetImage* lsImage = dynamic_cast< mitk::LabelSetImage* >( workingNode->GetData() );
  if (!lsImage) return;

  m_ProgressCommand = mitk::ToolCommand::New();

  CurrentlyBusy.Send(true);

  try
  {
    AccessByItk(lsImage, ITKProcessing);
  }
  catch( itk::ExceptionObject & e )
  {
   MITK_ERROR << "Exception caught: " << e.GetDescription();
   m_ProgressCommand->Reset();
   CurrentlyBusy.Send(false);
   return;
  }

  CurrentlyBusy.Send(false);

  lsImage->Modified();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


template < typename ImageType >
void mitk::MedianTool3D::ITKProcessing( typename ImageType* input )
{
  typedef itk::MedianImageFilter< ImageType, ImageType > MedianFilterType;

  typename ImageType::SizeType radius;
  radius.Fill(1);

  typename MedianFilterType::Pointer medianFilter = MedianFilterType::New();
  medianFilter->SetInput( input );
  medianFilter->SetRadius( radius );

  if (m_ProgressCommand.IsNotNull())
  {
    medianFilter->AddObserver( itk::AnyEvent(), m_ProgressCommand);
    m_ProgressCommand->AddStepsToDo(100);
  }

  medianFilter->Update();

  if (m_ProgressCommand.IsNotNull())
  {
    m_ProgressCommand->Reset(); // todo: use m_ProgressCommand->Reset()
  }

  ImageType::Pointer result = medianFilter->GetOutput();
  result->DisconnectPipeline();

  typedef itk::ImageRegionConstIterator< ImageType > SourceIteratorType;
  typedef itk::ImageRegionIterator< ImageType > TargetIteratorType;

  SourceIteratorType sourceIter( result, result->GetLargestPossibleRegion() );
  sourceIter.GoToBegin();

  TargetIteratorType targetIter( input, input->GetLargestPossibleRegion() );
  targetIter.GoToBegin();

  while ( !sourceIter.IsAtEnd() )
  {
    targetIter.Set( sourceIter.Get() );
    ++sourceIter;
    ++targetIter;
  }
}
