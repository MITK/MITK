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

#include "mitkAutoSegmentationTool.h"
#include "mitkToolManager.h"
#include "mitkUndoController.h"
#include "mitkLabelSetImage.h"
#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"
#include "mitkRenderingManager.h"

mitk::AutoSegmentationTool::AutoSegmentationTool() : Tool("dummy")
{
}

mitk::AutoSegmentationTool::~AutoSegmentationTool()
{
}

const char* mitk::AutoSegmentationTool::GetGroup() const
{
  return "autoSegmentation";
}

//to be moved to mitkInteractionConst.h by StateMachineEditor
const mitk::OperationType mitk::AutoSegmentationTool::OP_EXCHANGE = 717;

// constructors for operation classes
mitk::AutoSegmentationTool::opExchangeNodes::opExchangeNodes(
    mitk::OperationType type, mitk::DataNode* node, mitk::BaseData* oldData, mitk::BaseData* newData )
:mitk::Operation(type),m_Node(node),m_OldData(oldData),m_NewData(newData)
{
}

void mitk::AutoSegmentationTool::ExecuteOperation (mitk::Operation *operation)
{
  if (!operation) return;

  switch (operation->GetOperationType())
  {
    case OP_EXCHANGE:
      {
        opExchangeNodes* op = static_cast<opExchangeNodes*>(operation);
        op->GetNode()->SetData(op->GetNewData());
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        break;
      }
    default:;
  }
}

void mitk::AutoSegmentationTool::PasteSegmentation( Image* targetImage, Image* sourceImage, int pixelvalue, int timestep )
{
  if ((!targetImage)|| (!sourceImage)) return;

  try
  {
    AccessFixedDimensionByItk_2( targetImage, ItkPasteSegmentation, 3, sourceImage, pixelvalue );
  }
  catch( itk::ExceptionObject & e )
  {
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    m_ToolManager->ActivateTool(-1);
    return;
  }
  catch (...)
  {
    MITK_ERROR << "Unkown exception caught!";
    m_ToolManager->ActivateTool(-1);
    return;
  }
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::AutoSegmentationTool::ItkPasteSegmentation( itk::Image<TPixel,VImageDimension>* targetImage,
                                                       const mitk::Image* sourceImage, int overwritevalue )
{
  typedef itk::Image<TPixel,VImageDimension> ImageType;

  typename ImageType::Pointer sourceImageITK;
  CastToItkImage( sourceImage, sourceImageITK );

  typedef itk::ImageRegionConstIterator< ImageType > SourceIteratorType;
  typedef itk::ImageRegionIterator< ImageType >      TargetIteratorType;

  typename SourceIteratorType sourceIterator( sourceImageITK, sourceImageITK->GetLargestPossibleRegion() );
  typename TargetIteratorType targetIterator( targetImage, targetImage->GetLargestPossibleRegion() );

  sourceIterator.GoToBegin();
  targetIterator.GoToBegin();

  int activePixelValue = m_ToolManager->GetActiveLabel()->GetIndex();

  while ( !targetIterator.IsAtEnd() )
  {
    int targetValue = static_cast< int >( targetIterator.Get() );
    if ( !m_ToolManager->GetLabelLocked(targetValue) && sourceIterator.Get() )
    {
      targetIterator.Set( overwritevalue );
    }

    ++targetIterator;
    ++sourceIterator;
  }
}

void mitk::AutoSegmentationTool::InitializeUndoController()
{
//  mitk::UndoController::GetCurrentUndoModel()->Clear();
//  mitk::UndoController::GetCurrentUndoModel()->ClearRedoList();

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  if (!workingNode)
    mitkThrow() << "Could not retrieve working node!";

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>( workingNode->GetData() );
  if (!workingNode)
    mitkThrow() << "Could not retrieve working image!";

  mitk::LabelSetImage::Pointer diffImage = mitk::LabelSetImage::New(workingImage);
  diffImage->SetVolume(workingImage->GetData());

  opExchangeNodes* undoOp = new opExchangeNodes(OP_EXCHANGE, workingNode,
  workingNode->GetData(), diffImage);

  opExchangeNodes* doOp  = new opExchangeNodes(OP_EXCHANGE, workingNode,
      diffImage, workingNode->GetData());

  // TODO: MITK doesn't recognize that a new event happens in the next line,
  //       because nothing happens in the render window.
  //       As a result the undo action will happen together with the last action
  //       recognized by MITK.
  std::string msg("auto-segmentation");
  mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent(
     new mitk::OperationEvent(this, doOp, undoOp, msg) ); // tell the undo controller about the action

  ExecuteOperation(doOp);
}
