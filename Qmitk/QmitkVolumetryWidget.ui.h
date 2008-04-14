/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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

#include "QmitkCommonFunctionality.h"
#include "itkImage.h"
#include <limits>
#include "mitkDataTreeNode.h"
#include "mitkDataTreeHelper.h"
#include "mitkRenderingManager.h"

#include <qdir.h>
#include <qfiledialog.h>
#include <qmessagebox.h>

void QmitkVolumetryWidget::SetDataTreeNodeIterator( mitk::DataTreeIteratorClone it )
{
  m_DataTreeIteratorClone = it;
}

void QmitkVolumetryWidget::SetDataTreeNode(const mitk::DataTreeNode* node)
{
  if(node == m_OverlayNode.GetPointer())
    return;
  m_Node = node; // is the non-const m_Node needed anyway?
  if (m_OverlayNode)
  {
    // remove it from the tree
    mitk::DataTreeIteratorClone overlayIt = mitk::DataTreeHelper::FindIteratorToNode(m_DataTreeIteratorClone->GetTree(),m_OverlayNode);
    if (overlayIt.IsNotNull()) { overlayIt->Remove(); }
    m_OverlayNode = NULL;
  }
  CreateOverlayChild();
  std::string name;
  if (node->GetName(name))
  {
//    m_TreeNodeNameLabel->setText( name.c_str() );
  }
  m_CalcButton->setEnabled(false);
  m_TimeSeriesButton->setEnabled(false);
  m_SaveCsvButton->setEnabled(false);
  m_TextEdit->clear();

  if (m_Node)
  {
    mitk::Image* image = dynamic_cast<mitk::Image*>(m_Node->GetData());
    image->Update();
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
      int minVal = (int)image->GetScalarValue2ndMin();
      int maxVal = (int)image->GetScalarValueMaxNoRecompute();
      if (minVal == maxVal)
        --minVal;
      m_ThresholdSlider->setMinValue(minVal);
      m_ThresholdSlider->setMaxValue(maxVal);
      m_ThresholdSlider->setEnabled(true);
      UpdateSlider();
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
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
        vs << timeStep++ << "\t" << *it << std::endl;
      }
      m_TextEdit->setText(vs.str().c_str());
      m_TextEdit->setTabStopWidth(20);
      m_SaveCsvButton->setEnabled(true);
    }
  }
}

const mitk::DataTreeNode* QmitkVolumetryWidget::GetImageNode()
{
  return m_Node;
}

void QmitkVolumetryWidget::UpdateSlider()
{
  if (m_Node && dynamic_cast<mitk::Image*>(m_Node->GetData()))
  {
    int intSliderValue = (int)m_ThresholdSlider->value();
    QString stringSliderValue;
    stringSliderValue.setNum(intSliderValue);
    m_ThresholdLineEdit->setText(stringSliderValue);
  }
}


void QmitkVolumetryWidget::UpdateSliderLabel()
{
   int sliderValue = atoi(m_ThresholdLineEdit->text());
    m_ThresholdSlider->setValue(sliderValue);
    UpdateSlider();
  
}

void QmitkVolumetryWidget::m_ThresholdSlider_valueChanged( int value)
{
  if (m_OverlayNode)
  {
    m_OverlayNode->SetLevelWindow(mitk::LevelWindow(value,1));
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkVolumetryWidget::CreateOverlayChild()
{
  if (m_DataTreeIteratorClone.IsNull()) return;

  if (m_Node)
  {
    m_OverlayNode = mitk::DataTreeNode::New();
    mitk::DataTreeNodeFactory::SetDefaultImageProperties(m_OverlayNode);
    mitk::StringProperty::Pointer nameProp = mitk::StringProperty::New("volume threshold overlay image" );
    m_OverlayNode->SetProperty( "reslice interpolation", m_Node->GetProperty("reslice interpolation") );
    m_OverlayNode->SetProperty( "name", nameProp );
    m_OverlayNode->SetData(m_Node->GetData());
    m_OverlayNode->SetColor(0.0,1.0,0.0);
    m_OverlayNode->SetOpacity(.25);
    int layer = 0;
    m_Node->GetIntProperty("layer", layer);
    m_OverlayNode->SetIntProperty("layer", layer+1);
    m_OverlayNode->SetLevelWindow(mitk::LevelWindow(m_ThresholdSlider->value(),1));

    // hack!!!
    mitk::DataTreeIteratorClone iteratorClone = m_DataTreeIteratorClone;
    while ( !iteratorClone->IsAtEnd() )
    {
      mitk::DataTreeNode::Pointer node = iteratorClone->Get();
      if (  node == m_Node )
      {
        iteratorClone->Add(m_OverlayNode);
        break;
      }
      ++iteratorClone;
    }
  }
}


mitk::DataTreeNode* QmitkVolumetryWidget::GetOverlayNode()
{
  return m_OverlayNode;
}




mitk::Image* QmitkVolumetryWidget::GetImage()
{
  return dynamic_cast<mitk::Image*>(m_Node->GetData());
}
void QmitkVolumetryWidget::SaveCsvButtonClicked()
{
  static QString lastSavePath = QDir::home().absPath();
  QString s = QFileDialog::getSaveFileName(
                lastSavePath,
                "CSV Files (*.csv)",
                NULL,
                "Save file dialog"
                "Choose a filename to save under" );
  if (! s.isEmpty())
  {
    lastSavePath = s;
    QFile saveFile(s);
    if ( saveFile.open( IO_WriteOnly ) )
    {
      QTextStream stream( &saveFile );
      stream << m_TextEdit->text().replace('\t',';');
      saveFile.close();
    }
    else
    {
      // QMessageBox::critical(NULL,"Save Error!",QString("Saving of CSV failed! Couldn't open output file \"") + saveFile + QString("\""),QMessageBox:Ok,QMessageBox::NoButton);
      QMessageBox::critical(NULL,"Save Error!","Saving of CSV failed! Couldn't open output file \"" + saveFile.name() +"\"",QMessageBox::Ok,QMessageBox::NoButton);
    }
  }
}

