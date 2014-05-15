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

#include "mitkSegTool3D.h"
#include "mitkToolManager.h"
#include "mitkBaseRenderer.h"
#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"
#include "mitkUndoController.h"
#include "mitkImageTimeSelector.h"
#include "mitkLabelSetImage.h"
#include "mitkPositionEvent.h"

mitk::SegTool3D::SegTool3D(const char* type) : Tool(type)
{
  m_ProgressCommand = mitk::ToolCommand::New();
}

mitk::SegTool3D::~SegTool3D()
{
}

const char* mitk::SegTool3D::GetGroup() const
{
  return "SegTool3D";
}

//to be moved to mitkInteractionConst.h by StateMachineEditor
//const mitk::OperationType mitk::SegTool3D::OP_EXCHANGE = 717;

// constructors for operation classes
mitk::SegTool3D::opExchangeNodes::opExchangeNodes(
    mitk::OperationType type, mitk::DataNode* node, mitk::BaseData* oldData, mitk::BaseData* newData )
:mitk::Operation(type),m_Node(node),m_OldData(oldData),m_NewData(newData)
{
}

void mitk::SegTool3D::ExecuteOperation (mitk::Operation *operation)
{
  if (!operation) return;
  /*
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
  */
}

void mitk::SegTool3D::AcceptPreview()
{
  if ( m_PreviewImage.IsNull() ) return;

  mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( m_WorkingNode->GetData() );
  assert(workingImage);

  //check if active label has changed in the meanwhile (between Run and AcceptPreview)
  if (m_PaintingPixelValue != workingImage->GetActiveLabelPixelValue())
  {
    m_PreviewNode->SetVisibility(false);
    m_PreviewImage = NULL;
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    return;
  }

  CurrentlyBusy.Send(true);

  try
  {
    AccessFixedDimensionByItk_1( workingImage, InternalAcceptPreview, 3, m_PreviewImage );
  }
  catch( itk::ExceptionObject & e )
  {
    CurrentlyBusy.Send(false);
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    m_ToolManager->ActivateTool(-1);
    return;
  }
  catch (...)
  {
    CurrentlyBusy.Send(false);
    MITK_ERROR << "Unkown exception caught!";
    m_ToolManager->ActivateTool(-1);
    return;
  }

  CurrentlyBusy.Send(false);

  workingImage->Modified();

  m_PreviewNode->SetVisibility(false);
  m_PreviewImage = NULL;

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::SegTool3D::InvertPreview()
{
  if ( m_PreviewImage.IsNull() ) return;

  mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( m_WorkingNode->GetData() );
  assert(workingImage);

  try
  {
    AccessFixedDimensionByItk( workingImage, InternalInvertPreview, 3 );
  }
  catch( itk::ExceptionObject & e )
  {
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    m_ToolManager->ActivateTool(-1);
    return;
  }
  catch (...)
  {
    MITK_ERROR << "Unknown exception caught!";
    m_ToolManager->ActivateTool(-1);
    return;
  }

  m_PreviewImage->Modified();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::SegTool3D::AddPreview()
{
  if ( m_PreviewImage.IsNull() ) return;

  mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( m_WorkingNode->GetData() );
  assert(workingImage);

  try
  {
    AccessFixedDimensionByItk( workingImage, InternalAddPreview, 3 );
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

  m_PreviewImage->Modified();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::SegTool3D::Cancel()
{
  m_ToolManager->ActivateTool(-1);
}

void mitk::SegTool3D::Activated()
{
  Superclass::Activated();

  // feedback node and its visualization properties
  m_PreviewNode = mitk::DataNode::New();
  m_PreviewNode->SetName("3D tool preview");

  m_PreviewNode->SetProperty("texture interpolation", BoolProperty::New(false) );
  m_PreviewNode->SetProperty("layer", IntProperty::New(100) );
  m_PreviewNode->SetProperty("binary", BoolProperty::New(true) );
  m_PreviewNode->SetProperty("outline binary", BoolProperty::New(true) );
  m_PreviewNode->SetProperty("outline binary shadow", BoolProperty::New(true) );
  m_PreviewNode->SetProperty("helper object", BoolProperty::New(true) );
  m_PreviewNode->SetOpacity(1.0);
  m_PreviewNode->SetColor(0.0, 1.0, 0.0);

  m_ToolManager->GetDataStorage()->Add(m_PreviewNode, m_WorkingNode);
}

void mitk::SegTool3D::Deactivated()
{
  Superclass::Deactivated();
  m_ToolManager->GetDataStorage()->Remove( m_PreviewNode );
  m_PreviewNode = NULL;
  m_PreviewImage = NULL;
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::SegTool3D::CreateNewLabel(const std::string& name, const mitk::Color& color)
{
  if ( m_PreviewImage.IsNull() ) return;

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(m_WorkingNode->GetData());
  assert(workingImage);

  workingImage->AddLabel(name, color);

  this->AcceptPreview();
}

template<typename ImageType>
void mitk::SegTool3D::InternalAcceptPreview( ImageType* targetImage, const mitk::Image* sourceImage )
{
  typename ImageType::Pointer sourceImageITK;
  CastToItkImage( sourceImage, sourceImageITK );

  typedef itk::ImageRegionConstIterator< ImageType > SourceIteratorType;
  typedef itk::ImageRegionIterator< ImageType >      TargetIteratorType;

  typename ImageType::IndexType cropIndex;
  cropIndex[0] = m_RequestedRegion.GetIndex(0);
  cropIndex[1] = m_RequestedRegion.GetIndex(1);
  cropIndex[2] = m_RequestedRegion.GetIndex(2);

  typename ImageType::SizeType cropSize;
  cropSize[0] = m_RequestedRegion.GetSize(0);
  cropSize[1] = m_RequestedRegion.GetSize(1);
  cropSize[2] = m_RequestedRegion.GetSize(2);

  typename ImageType::RegionType cropRegion(cropIndex, cropSize);

  SourceIteratorType sourceIter( sourceImageITK, sourceImageITK->GetLargestPossibleRegion() );
  TargetIteratorType targetIter( targetImage, cropRegion );

  sourceIter.GoToBegin();
  targetIter.GoToBegin();

  while ( !targetIter.IsAtEnd() )
  {
    int targetValue = static_cast< int >( targetIter.Get() );
    int sourceValue = static_cast< int >( sourceIter.Get() );

    if ( (targetValue == m_PaintingPixelValue) || sourceValue )
    {
      if (sourceValue)
        targetIter.Set( m_PaintingPixelValue );
      else
        targetIter.Set( 0 );
    }

    ++sourceIter;
    ++targetIter;
  }
}

template<typename ImageType>
void mitk::SegTool3D::InternalInvertPreview( ImageType* input )
{
  typedef itk::Image<LabelSetImage::PixelType, 3> BinaryImageType;
  BinaryImageType::Pointer previewItk;
  CastToItkImage( m_PreviewImage, previewItk );

  typedef itk::ImageRegionConstIterator< ImageType >  SourceIteratorType;
  typedef itk::ImageRegionIterator< BinaryImageType > TargetIteratorType;

  typename ImageType::IndexType cropIndex;
  cropIndex[0] = m_RequestedRegion.GetIndex(0);
  cropIndex[1] = m_RequestedRegion.GetIndex(1);
  cropIndex[2] = m_RequestedRegion.GetIndex(2);

  typename ImageType::SizeType cropSize;
  cropSize[0] = m_RequestedRegion.GetSize(0);
  cropSize[1] = m_RequestedRegion.GetSize(1);
  cropSize[2] = m_RequestedRegion.GetSize(2);

  typename ImageType::RegionType cropRegion(cropIndex, cropSize);

  SourceIteratorType sourceIter( input, cropRegion );
  TargetIteratorType targetIter( previewItk, previewItk->GetLargestPossibleRegion() );

  sourceIter.GoToBegin();
  targetIter.GoToBegin();

  while ( !targetIter.IsAtEnd() )
  {
    int targetValue = static_cast< int >( targetIter.Get() );
    int sourceValue = static_cast< int >( sourceIter.Get() );

    if ( (targetValue && (sourceValue != m_PaintingPixelValue)) || ((sourceValue==m_PaintingPixelValue) && !targetValue) )
      targetIter.Set(1);
    else
      targetIter.Set(0);

    ++sourceIter;
    ++targetIter;
  }
}

template<typename ImageType>
void mitk::SegTool3D::InternalAddPreview( ImageType* input )
{
  typedef itk::Image<LabelSetImage::PixelType, 3> BinaryImageType;
  BinaryImageType::Pointer previewItk;
  CastToItkImage( m_PreviewImage, previewItk );

  typedef itk::ImageRegionConstIterator< ImageType >  SourceIteratorType;
  typedef itk::ImageRegionIterator< BinaryImageType > TargetIteratorType;

  typename ImageType::IndexType cropIndex;
  cropIndex[0] = m_RequestedRegion.GetIndex(0);
  cropIndex[1] = m_RequestedRegion.GetIndex(1);
  cropIndex[2] = m_RequestedRegion.GetIndex(2);

  typename ImageType::SizeType cropSize;
  cropSize[0] = m_RequestedRegion.GetSize(0);
  cropSize[1] = m_RequestedRegion.GetSize(1);
  cropSize[2] = m_RequestedRegion.GetSize(2);

  typename ImageType::RegionType cropRegion(cropIndex,cropSize);

  SourceIteratorType sourceIter( input, cropRegion );
  TargetIteratorType targetIter( previewItk, previewItk->GetLargestPossibleRegion() );

  sourceIter.GoToBegin();
  targetIter.GoToBegin();

  while ( !targetIter.IsAtEnd() )
  {
    int targetValue = static_cast< int >( targetIter.Get() );
    int sourceValue = static_cast< int >( sourceIter.Get() );

    if (sourceValue || targetValue)
      targetIter.Set( 1 );

    ++sourceIter;
    ++targetIter;
  }
}

mitk::Image::Pointer mitk::SegTool3D::Get3DImage(mitk::Image::Pointer image, unsigned int timestep)
{
  if (image->GetDimension() != 4)
    return image;

  mitk::ImageTimeSelector::Pointer imageTimeSelector = mitk::ImageTimeSelector::New();
  imageTimeSelector->SetInput(image);
  imageTimeSelector->SetTimeNr(static_cast<int>(timestep));
  imageTimeSelector->UpdateLargestPossibleRegion();
  return imageTimeSelector->GetOutput();
}

void mitk::SegTool3D::InitializeUndoController()
{
/*
  mitk::UndoController::GetCurrentUndoModel()->Clear();
  mitk::UndoController::GetCurrentUndoModel()->ClearRedoList();

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>( m_WorkingNode->GetData() );
  assert(workingImage);

  mitk::LabelSetImage::Pointer diffImage = mitk::LabelSetImage::New(workingImage);
  diffImage->SetVolume(workingImage->GetData());

  opExchangeNodes* undoOp = new opExchangeNodes(OP_EXCHANGE, m_WorkingNode,
  m_WorkingNode->GetData(), diffImage);

  opExchangeNodes* doOp  = new opExchangeNodes(OP_EXCHANGE, m_WorkingNode,
      diffImage, m_WorkingNode->GetData());

  // TODO: MITK doesn't recognize that a new event happens in the next line,
  //       because nothing happens in the render window.
  //       As a result the undo action will happen together with the last action
  //       recognized by MITK.
  std::string msg("SegTool3D");
  mitk::UndoController::GetCurrentUndoModel()->SetOperationEvent(
     new mitk::OperationEvent(this, doOp, undoOp, msg) ); // tell the undo controller about the action

  ExecuteOperation(doOp);
*/
}
