/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/
/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

#include "mitkImageTimeSelector.h"
#include "QmitkCommonFunctionality.h"
#include "itkImage.h"
#include <limits>
#include "mitkDataTreeNode.h"
#include "mitkRenderWindow.h"

void QmitkVolumetryWidget::SetDataTreeNodeIterator( mitk::DataTreeIteratorClone it )
{
  m_DataTreeIteratorClone = it;
}

void QmitkVolumetryWidget::SetDataTreeNode(mitk::DataTreeNode* node)
{
  m_Node = node;
  if (m_OverlayNode)
  {
    // remove it from the tree
    m_OverlayNode = NULL;
  }
  CreateOverlayChild();
  std::string name;
  if (node->GetName(name))
  {
    m_TreeNodeNameLabel->setText( name.c_str() );
  }
  m_CalcButton->setEnabled(false);
  m_TimeSeriesButton->setEnabled(false);

  if (m_Node)
  {
    mitk::Image* image = dynamic_cast<mitk::Image*>(m_Node->GetData());
    if (image)
    {
      if (image->GetDimension() == 4)
      {
        m_TimeSeriesButton->setEnabled(true);
      }
      else
      {
        m_CalcButton->setEnabled(true);
      }
      UpdateSlider();
    }
  }
}

void QmitkVolumetryWidget::CalculateVolume()
{
  if (m_Node)
  {
    mitk::Image* image = dynamic_cast<mitk::Image*>(m_Node->GetData());
    if (image)
    {
      std::cout << "Dimension:" << image->GetDimension() << std::endl;
      std::cout << "Dimension[3]:" << image->GetDimension(3) << std::endl;
      mitk::VolumeCalculator::Pointer volCalc = mitk::VolumeCalculator::New();
      volCalc->SetImage(image);
      volCalc->SetThreshold(m_ThresholdSlider->value());
      volCalc->ComputeVolume();
      std::stringstream vs;
      vs << volCalc->GetVolume() << " ml";
      m_Result->setText(vs.str().c_str() );
    }
  }
}

void QmitkVolumetryWidget::CalculateTimeSeries()
{
  if (m_Node)
  {
    mitk::Image* image = dynamic_cast<mitk::Image*>(m_Node->GetData());
    if (image)
    {
      mitk::VolumeCalculator::Pointer volCalc = mitk::VolumeCalculator::New();
      volCalc->SetImage(image);
      volCalc->SetThreshold(m_ThresholdSlider->value());
      volCalc->ComputeVolume();
      std::vector<float> volumes = volCalc->GetVolumes();
      std::stringstream vs;
      int timeStep = 0;
      for (std::vector<float>::iterator it = volumes.begin(); it != volumes.end(); it++)
      {
        vs << timeStep++ << "," << *it << std::endl;
      }
      m_TextEdit->setText(vs.str().c_str());
    }
  }
}

void QmitkVolumetryWidget::UpdateSlider()
{

  if (m_Node && dynamic_cast<mitk::Image*>(m_Node->GetData()))
  {
    typedef itk::Image<int,3> ItkInt3DImage;
    ItkInt3DImage::Pointer itkImage = ItkInt3DImage::New();
    mitk::Image* image = dynamic_cast<mitk::Image*>(m_Node->GetData());

    if (image->GetDimension() == 4)
    {

      mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
      timeSelector->SetInput(image);
      float gMin = std::numeric_limits<float>::max();
      float gMax = std::numeric_limits<float>::min();
      for (unsigned int timeStep = 0; timeStep<image->GetDimension(3); timeStep++)
      {
        timeSelector->SetTimeNr(timeStep);
        timeSelector->Update();
        mitk::CastToItkImage(timeSelector->GetOutput(),itkImage);
        float min,max;
        CommonFunctionality::MinMax<ItkInt3DImage>(itkImage,min,max);
        if (min<gMin) { gMin = min; }
        if (max>gMax) { gMax = max; }
        m_ThresholdSlider->setMinValue((int)gMin);
        m_ThresholdSlider->setMaxValue((int)gMax);
        m_ThresholdSlider->setEnabled(true);
        m_ThresholdLabel->setNum(m_ThresholdSlider->value());
      }
    }
    else if (image->GetDimension() == 3)
    {
      mitk::CastToItkImage(image,itkImage);
      float min,max;
      CommonFunctionality::MinMax<ItkInt3DImage>(itkImage,min,max);
      m_ThresholdSlider->setMinValue((int)min);
      m_ThresholdSlider->setMaxValue((int)max);
      m_ThresholdSlider->setEnabled(true);
      m_ThresholdLabel->setNum(m_ThresholdSlider->value());
    }
    else
    {
      m_ThresholdSlider->setEnabled(false);
    }

  }
}


void QmitkVolumetryWidget::m_ThresholdSlider_valueChanged( int value)
{
  if (m_OverlayNode) {m_OverlayNode->SetLevelWindow(mitk::LevelWindow(value,1)); mitk::RenderWindow::UpdateAllInstances();}
}

void QmitkVolumetryWidget::CreateOverlayChild()
{
  if (m_Node)
  {
    m_OverlayNode = mitk::DataTreeNode::New();
    mitk::StringProperty::Pointer nameProp = new mitk::StringProperty("volume threshold overlay image" );
    m_OverlayNode->SetProperty( "name", nameProp );
    m_OverlayNode->SetData(m_Node->GetData());
    m_OverlayNode->SetColor(1.0,0.0,0.0);
    m_OverlayNode->SetOpacity(.7);
    m_OverlayNode->SetLevelWindow(mitk::LevelWindow(m_ThresholdSlider->value(),1));
  
    // hack!!!
    mitk::DataTreeIteratorClone iteratorClone = m_DataTreeIteratorClone;
    while ( !iteratorClone->IsAtEnd() )
    {
      mitk::DataTreeNode::Pointer node = iteratorClone->Get();
      if (  node == m_Node )
      {
        iteratorClone->Add(m_OverlayNode);
      }
      ++iteratorClone;
    }
  }
}
