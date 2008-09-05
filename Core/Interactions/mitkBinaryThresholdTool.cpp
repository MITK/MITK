/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkBinaryThresholdTool.h"

#include "mitkBinaryThresholdTool.xpm"

#include "mitkToolManager.h"

#include "mitkDataTreeNodeFactory.h"
#include "mitkLevelWindowProperty.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkOrganTypeProperty.h"
#include "mitkVtkResliceInterpolationProperty.h"
#include "mitkDataStorage.h"
#include "mitkRenderingManager.h"

#include "mitkImageCast.h"
#include "mitkImageTimeSelector.h"
#include <itkImageRegionIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>

mitk::BinaryThresholdTool::BinaryThresholdTool()
:m_SensibleMinimumThresholdValue(-100),
 m_SensibleMaximumThresholdValue(+100),
 m_CurrentThresholdValue(1)
{
  m_ThresholdFeedbackNode = DataTreeNode::New();
  DataTreeNodeFactory::SetDefaultImageProperties ( m_ThresholdFeedbackNode );
  m_ThresholdFeedbackNode->SetProperty( "color", ColorProperty::New(0.2, 1.0, 0.2) );
  m_ThresholdFeedbackNode->SetProperty( "texture interpolation", BoolProperty::New(false) );
  m_ThresholdFeedbackNode->SetProperty( "layer", IntProperty::New( 20 ) );
  m_ThresholdFeedbackNode->SetProperty( "levelwindow", LevelWindowProperty::New( LevelWindow(100, 1) ) );
  m_ThresholdFeedbackNode->SetProperty( "name", StringProperty::New("Thresholding feedback") );
  m_ThresholdFeedbackNode->SetProperty( "opacity", FloatProperty::New(0.2) );
  m_ThresholdFeedbackNode->SetProperty( "helper object", BoolProperty::New(true) );
}

mitk::BinaryThresholdTool::~BinaryThresholdTool()
{
}

const char** mitk::BinaryThresholdTool::GetXPM() const
{
  return mitkBinaryThresholdTool_xpm;
}

const char* mitk::BinaryThresholdTool::GetName() const
{
  return "Thresholding";
}

void mitk::BinaryThresholdTool::Activated()
{
  m_NodeForThresholding = m_ToolManager->GetReferenceData(0);

  if ( m_NodeForThresholding.IsNotNull() )
  {
    SetupPreviewNodeFor( m_NodeForThresholding );
  }
  else
  {
    m_ToolManager->ActivateTool(-1);
  }
}

void mitk::BinaryThresholdTool::Deactivated()
{
  m_NodeForThresholding = NULL;
  try
  {
    DataStorage::GetInstance()->Remove( m_ThresholdFeedbackNode );
  }
  catch(...)
  {
    // don't care
  }
  m_ThresholdFeedbackNode->SetData(NULL);
}
    
void mitk::BinaryThresholdTool::SetThresholdValue(int value)
{
  if (m_ThresholdFeedbackNode.IsNotNull())
  {
    m_CurrentThresholdValue = value;
    m_ThresholdFeedbackNode->SetProperty( "levelwindow", LevelWindowProperty::New( LevelWindow(m_CurrentThresholdValue, 1) ) );
    RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void mitk::BinaryThresholdTool::AcceptCurrentThresholdValue(const std::string& organType, const std::string& organName)
{

  CreateNewSegmentationFromThreshold(m_NodeForThresholding, organType, organName );

  RenderingManager::GetInstance()->RequestUpdateAll();
  m_ToolManager->ActivateTool(-1);
}

void mitk::BinaryThresholdTool::SetupPreviewNodeFor( DataTreeNode* nodeForThresholding )
{
  if (nodeForThresholding)
  {
    Image::Pointer image = dynamic_cast<Image*>( nodeForThresholding->GetData() );
    if (image.IsNotNull())
    {
      // initialize and a new node with the same image as our reference image
      // use the level window property of this image copy to display the result of a thresholding operation
      m_ThresholdFeedbackNode->SetData( image );
      int layer(50);
      nodeForThresholding->GetIntProperty("layer", layer);
      m_ThresholdFeedbackNode->SetIntProperty("layer", layer+1);
       
      DataStorage::GetInstance()->Add( m_ThresholdFeedbackNode, nodeForThresholding );

      m_SensibleMinimumThresholdValue = static_cast<int>( image->GetScalarValueMin() );
      m_SensibleMaximumThresholdValue = static_cast<int>( image->GetScalarValueMax() );

      LevelWindowProperty::Pointer lwp = dynamic_cast<LevelWindowProperty*>( m_ThresholdFeedbackNode->GetProperty( "levelwindow" ));
      if (lwp)
      {
        m_CurrentThresholdValue = static_cast<int>( lwp->GetLevelWindow().GetLevel() );
      }
      else
      {
        m_CurrentThresholdValue = (m_SensibleMaximumThresholdValue + m_SensibleMinimumThresholdValue) / 2;
      }

      IntervalBordersChanged.Send(m_SensibleMinimumThresholdValue, m_SensibleMaximumThresholdValue);
      ThresholdingValueChanged.Send(m_CurrentThresholdValue);
    }
  }
}

void mitk::BinaryThresholdTool::CreateNewSegmentationFromThreshold(DataTreeNode* node, const std::string& organType, const std::string& organName)
{
  if (node)
  {
    Image::Pointer image = dynamic_cast<Image*>( m_NodeForThresholding->GetData() );
    if (image.IsNotNull())
    {
      // create a new image of the same dimensions and smallest possible pixel type
      DataTreeNode::Pointer emptySegmentation = Tool::CreateEmptySegmentationNode( image, organType, organName );

      if (emptySegmentation)
      {
        // actually perform a thresholding and ask for an organ type
        for (unsigned int timeStep = 0; timeStep < image->GetTimeSteps(); ++timeStep)
        {
          try
          {
            ImageTimeSelector::Pointer timeSelector = ImageTimeSelector::New();
            timeSelector->SetInput( image );
            timeSelector->SetTimeNr( timeStep );
            timeSelector->UpdateLargestPossibleRegion();
            Image::Pointer image3D = timeSelector->GetOutput();

            AccessFixedDimensionByItk_2( image3D, ITKThresholding, 3, dynamic_cast<Image*>(emptySegmentation->GetData()), timeStep );
          }
          catch(...)
          {
            EmitErrorMessage("Error accessing single time steps of the original image. Cannot create segmentation.");
          }
        }

        DataStorage::GetInstance()->Add( emptySegmentation, m_NodeForThresholding ); // add as a child, because the segmentation "derives" from the original

        m_ToolManager->SetWorkingData( emptySegmentation );
      }
    }
  }
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::BinaryThresholdTool::ITKThresholding( itk::Image<TPixel, VImageDimension>* originalImage, Image* segmentation, unsigned int timeStep )
{
  ImageTimeSelector::Pointer timeSelector = ImageTimeSelector::New();
  timeSelector->SetInput( segmentation );
  timeSelector->SetTimeNr( timeStep );
  timeSelector->UpdateLargestPossibleRegion();
  Image::Pointer segmentation3D = timeSelector->GetOutput();

  typedef itk::Image< Tool::DefaultSegmentationDataType, 3> SegmentationType; // this is sure for new segmentations
  SegmentationType::Pointer itkSegmentation;
  CastToItkImage( segmentation3D, itkSegmentation );

  // iterate over original and segmentation
  typedef itk::ImageRegionConstIterator< itk::Image<TPixel, VImageDimension> >     InputIteratorType;
  typedef itk::ImageRegionIterator< SegmentationType >     SegmentationIteratorType;

  InputIteratorType inputIterator( originalImage, originalImage->GetLargestPossibleRegion() );
  SegmentationIteratorType outputIterator( itkSegmentation, itkSegmentation->GetLargestPossibleRegion() );

  inputIterator.GoToBegin();
  outputIterator.GoToBegin();

  while (!outputIterator.IsAtEnd())
  {
    if ( (signed)inputIterator.Get() >= m_CurrentThresholdValue )
    {
      outputIterator.Set( 1 );
    }
    else
    {
      outputIterator.Set( 0 );
    }

    ++inputIterator;
    ++outputIterator;
  }
}

