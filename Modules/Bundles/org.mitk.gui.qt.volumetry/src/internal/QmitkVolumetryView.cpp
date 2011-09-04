/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-28 17:19:30 +0200 (Do, 28 Mai 2009) $
Version:   $Revision: 17495 $ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkVolumetryView.h"

#include "mitkNodePredicateDataType.h"

#include "QmitkDataStorageComboBox.h"
#include "QmitkStdMultiWidget.h"

#include <QMessageBox>




#include "itkImage.h"
#include <limits>
#include "mitkDataNode.h"
#include "mitkRenderingManager.h"
#include "mitkVolumeCalculator.h"

#include <qdir.h>
#include <qfiledialog.h>
#include <QTextStream>

const std::string QmitkVolumetryView::VIEW_ID = "org.mitk.views.volumetry";

QmitkVolumetryView::QmitkVolumetryView()
: QmitkFunctionality(),
  m_Controls(NULL),
  m_MultiWidget(NULL)
{
  //m_DataStorage = this->GetDefaultDataStorage();
}

QmitkVolumetryView::~QmitkVolumetryView()
{
}

void QmitkVolumetryView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkVolumetryViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();

    // define data type for combobox
    m_Controls->m_ImageSelector->SetDataStorage( this->GetDefaultDataStorage() );
    m_Controls->m_ImageSelector->SetPredicate( mitk::NodePredicateDataType::New("Image") );
  }
}

void QmitkVolumetryView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}

void QmitkVolumetryView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}

void QmitkVolumetryView::CreateConnections()
{
  if ( m_Controls )
  {

    connect( m_Controls->m_ImageSelector, SIGNAL(OnSelectionChanged(const mitk::DataNode*)), this, SLOT(OnImageSelected(const mitk::DataNode*)) );

    //connect( (QObject*)(m_Controls->m_Button), SIGNAL(clicked()), this, SLOT(DoSomething()) );
    connect( (QObject*)(m_Controls->m_ThresholdSlider), SIGNAL(valueChanged(int)), this, SLOT(OnThresholdSliderChanged(int)) );
    //connect( (QObject*)(m_Controls->m_ThresholdLineEdit), SIGNAL(valueChanged(int)), this, SLOT(OnThresholdLineEdit(int)) );
    //connect( (QObject*)(m_Controls->m_TextEdit), SIGNAL(valueChanged(int)), this, SLOT(OnTextEditChanged(int)) );
    connect( (QObject*)(m_Controls->m_CalcButton), SIGNAL(clicked()), this, SLOT(OnCalculateVolume()) ); 
    connect( (QObject*)(m_Controls->m_TimeSeriesButton), SIGNAL(clicked()), this, SLOT(OnTimeSeriesButtonClicked()) ); 
    connect( (QObject*)(m_Controls->m_SaveCsvButton), SIGNAL(clicked()), this, SLOT(OnSaveCsvButtonClicked()) ); 
  }
}

void QmitkVolumetryView::Activated()
{
  QmitkFunctionality::Activated();
}

void QmitkVolumetryView::Deactivated()
{
  QmitkFunctionality::Deactivated();
}


void QmitkVolumetryView::OnImageSelected(const mitk::DataNode* item)
{
  // nothing selected (NULL selection)
  if( item == NULL  || item->GetData() == NULL )
    return;

  if( item->GetData()->GetSource().IsNull() )
  {
    m_SelectedDataNode = 0;
    return; 
  }

  m_SelectedDataNode = const_cast<mitk::DataNode*>(item);

  std::cout << "Selected mitk::Image at address " << m_SelectedDataNode->GetData() << std::endl;


  if(item == m_OverlayNode.GetPointer())
    return;

  if (m_OverlayNode)
  {
    this->GetDefaultDataStorage()->Remove(m_OverlayNode);
    // remove it from the tree
    //mitk::DataTreeIteratorClone overlayIt = mitk::DataTreeHelper::FindIteratorToNode(m_DataTreeIteratorClone->GetTree(),m_OverlayNode);
    //if (overlayIt.IsNotNull()) { overlayIt->Remove(); }
    m_OverlayNode = NULL;
  }

  this->CreateOverlayChild();
  
  //std::string name;
  //if (node->GetName(name))
  //{
  //  //    m_TreeNodeNameLabel->setText( name.c_str() );
  //}

  m_Controls->m_CalcButton->setEnabled(false);
  m_Controls->m_TimeSeriesButton->setEnabled(false);
  m_Controls->m_SaveCsvButton->setEnabled(false);
  m_Controls->m_TextEdit->clear();

  if (m_SelectedDataNode.IsNotNull() )
  {
    mitk::Image* image = dynamic_cast<mitk::Image*>(m_SelectedDataNode->GetData());
    image->Update();
    if (image && image->IsInitialized())
    {
      if (image->GetDimension() == 4)
      {
        m_Controls->m_TimeSeriesButton->setEnabled(true);
      }
      else
      {
        m_Controls->m_CalcButton->setEnabled(true);
      }
      int minVal = (int)image->GetScalarValue2ndMin();
      int maxVal = (int)image->GetScalarValueMaxNoRecompute();
      if (minVal == maxVal)
        --minVal;
      m_Controls->m_ThresholdSlider->setMinimum(minVal);
      m_Controls->m_ThresholdSlider->setMaximum(maxVal);
      m_Controls->m_ThresholdSlider->setEnabled(true);
      this->UpdateSlider();
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
}

//void QmitkVolumetryView::DoSomething()
//{
//  mitk::DataNode* node = m_Controls->m_ImageSelector->GetSelectedNode();
//  if (!node)
//  {
//    // Nothing selected. Inform the user and return
//    QMessageBox::information( NULL, "Template functionality", "Please load and select an image before starting some action.");
//    return;
//  }
//
//  // here we have a valid mitk::DataNode
//
//  // a node itself is not very useful, we need its data item (the image)
//  mitk::BaseData* data = node->GetData();
//  if (data)
//  {
//    // test if this data item is really an image or not (could also be a surface or something totally different)
//    mitk::Image* image = dynamic_cast<mitk::Image*>( data );
//    if (image)
//    {
//      std::string name("(no name)");
//      node->GetName(name);
//
//      QMessageBox::information( NULL, "Image processing",
//                                QString( "Doing something to '%1'" ).arg(name.c_str()) );
//
//      // at this point anything can be done using the mitk::Image image.
//    }
//  }
//}








//void QmitkVolumetryView::SetDataNode(const mitk::DataNode* node)
//{
//  if(node == m_OverlayNode.GetPointer())
//    return;
//  m_SelectedDataNode = node; // is the non-const m_SelectedDataNode needed anyway?
//  if (m_OverlayNode)
//  {
//    // remove it from the tree
//    mitk::DataTreeIteratorClone overlayIt = mitk::DataTreeHelper::FindIteratorToNode(m_DataTreeIteratorClone->GetTree(),m_OverlayNode);
//    if (overlayIt.IsNotNull()) { overlayIt->Remove(); }
//    m_OverlayNode = NULL;
//  }
//  CreateOverlayChild();
//  std::string name;
//  if (node->GetName(name))
//  {
//    //    m_TreeNodeNameLabel->setText( name.c_str() );
//  }
//  m_CalcButton->setEnabled(false);
//  m_TimeSeriesButton->setEnabled(false);
//  m_SaveCsvButton->setEnabled(false);
//  m_TextEdit->clear();
//
//  if (m_SelectedDataNode)
//  {
//    mitk::Image* image = dynamic_cast<mitk::Image*>(m_SelectedDataNode->GetData());
//    image->Update();
//    if (image && image->IsInitialized())
//    {
//      if (image->GetDimension() == 4)
//      {
//        m_TimeSeriesButton->setEnabled(true);
//      }
//      else
//      {
//        m_CalcButton->setEnabled(true);
//      }
//      int minVal = (int)image->GetScalarValue2ndMin();
//      int maxVal = (int)image->GetScalarValueMaxNoRecompute();
//      if (minVal == maxVal)
//        --minVal;
//      m_ThresholdSlider->setMinValue(minVal);
//      m_ThresholdSlider->setMaxValue(maxVal);
//      m_ThresholdSlider->setEnabled(true);
//      UpdateSlider();
//      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
//    }
//  }
//}

void QmitkVolumetryView::OnCalculateVolume()
{
  if (m_SelectedDataNode.IsNotNull() )
  {
    mitk::Image* image = dynamic_cast<mitk::Image*>(m_SelectedDataNode->GetData());
    if (image)
    {
      std::cout << "Dimension:" << image->GetDimension() << std::endl;
      std::cout << "Dimension[3]:" << image->GetDimension(3) << std::endl;
      mitk::VolumeCalculator::Pointer volCalc = mitk::VolumeCalculator::New();
      volCalc->SetImage(image);
      volCalc->SetThreshold(m_Controls->m_ThresholdSlider->value());
      volCalc->ComputeVolume();
      std::stringstream vs;
      vs << volCalc->GetVolume() << " ml";
      m_Controls->m_Result->setText(vs.str().c_str() );
    }
  }
}

void QmitkVolumetryView::OnTimeSeriesButtonClicked()
{
  if (m_SelectedDataNode.IsNotNull() )
  {
    mitk::Image* image = dynamic_cast<mitk::Image*>(m_SelectedDataNode->GetData());
    if (image)
    {
      mitk::VolumeCalculator::Pointer volCalc = mitk::VolumeCalculator::New();
      volCalc->SetImage(image);
      volCalc->SetThreshold(m_Controls->m_ThresholdSlider->value());
      volCalc->ComputeVolume();
      std::vector<float> volumes = volCalc->GetVolumes();
      std::stringstream vs;
      int timeStep = 0;
      for (std::vector<float>::iterator it = volumes.begin(); it != volumes.end(); it++)
      {
        vs << timeStep++ << "\t" << *it << std::endl;
      }
      m_Controls->m_TextEdit->setText(vs.str().c_str());
      m_Controls->m_TextEdit->setTabStopWidth(20);
      m_Controls->m_SaveCsvButton->setEnabled(true);
    }
  }
}

const mitk::DataNode* QmitkVolumetryView::GetImageNode()
{
  return m_SelectedDataNode;
}

void QmitkVolumetryView::UpdateSlider()
{
  if (m_SelectedDataNode.IsNotNull() && dynamic_cast<mitk::Image*>(m_SelectedDataNode->GetData()))
  {
    int intSliderValue = (int)m_Controls->m_ThresholdSlider->value();
    QString stringSliderValue;
    stringSliderValue.setNum(intSliderValue);
    m_Controls->m_ThresholdLineEdit->setText(stringSliderValue);
  }
}


void QmitkVolumetryView::UpdateSliderLabel()
{
  int sliderValue = atoi(m_Controls->m_ThresholdLineEdit->text().toLatin1());
  m_Controls->m_ThresholdSlider->setValue(sliderValue);
  this->UpdateSlider();

}

void QmitkVolumetryView::OnThresholdSliderChanged( int value)
{
  if (m_OverlayNode)
  {
    m_OverlayNode->SetLevelWindow(mitk::LevelWindow(value,1));
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    this->UpdateSlider();
  }
}

void QmitkVolumetryView::CreateOverlayChild()
{
  if (m_SelectedDataNode.IsNotNull())
  {
    m_OverlayNode = mitk::DataNode::New();
    mitk::StringProperty::Pointer nameProp = mitk::StringProperty::New("volume threshold overlay image" );
    m_OverlayNode->SetProperty( "reslice interpolation", m_SelectedDataNode->GetProperty("reslice interpolation") );
    m_OverlayNode->SetProperty( "name", nameProp );
    m_OverlayNode->SetData(m_SelectedDataNode->GetData());
    m_OverlayNode->SetColor(0.0,1.0,0.0);
    m_OverlayNode->SetOpacity(.25);
    int layer = 0;
    m_SelectedDataNode->GetIntProperty("layer", layer);
    m_OverlayNode->SetIntProperty("layer", layer+1);
    m_OverlayNode->SetLevelWindow(mitk::LevelWindow(m_Controls->m_ThresholdSlider->value(),1));


    this->GetDefaultDataStorage()->Add(m_OverlayNode);

    //// hack!!!
    //mitk::DataTreeIteratorClone iteratorClone = m_DataTreeIteratorClone;
    //while ( !iteratorClone->IsAtEnd() )
    //{
    //  mitk::DataNode::Pointer node = iteratorClone->Get();
    //  if (  node == m_SelectedDataNode )
    //  {
    //    iteratorClone->Add(m_OverlayNode);
    //    break;
    //  }
    //  ++iteratorClone;
    //}
  }
}


mitk::DataNode* QmitkVolumetryView::GetOverlayNode()
{
  return m_OverlayNode;
}




mitk::Image* QmitkVolumetryView::GetImage()
{
  return dynamic_cast<mitk::Image*>(m_SelectedDataNode->GetData());
}
void QmitkVolumetryView::OnSaveCsvButtonClicked()
{
  static QString lastSavePath = QDir::homePath();
  

  QString s = QFileDialog::getSaveFileName( this->m_Parent, "Save as..", lastSavePath, "CSV Files (*.csv)");
    /*"Save file dialog"
    "Choose a filename to save under" );*/


  if (! s.isEmpty())
  {
    lastSavePath = s;
    QFile saveFile(s);
    
    if ( saveFile.open(QIODevice::WriteOnly))
    {
      QTextStream stream( &saveFile );
      stream << m_Controls->m_TextEdit->toPlainText().replace('\t',';');
      saveFile.close();
    }
    else
    {
      // QMessageBox::critical(NULL,"Save Error!",QString("Saving of CSV failed! Couldn't open output file \"") + saveFile + QString("\""),QMessageBox:Ok,QMessageBox::NoButton);
      //QMessageBox::critical(NULL,"Save Error!","Saving of CSV failed! Couldn't open output file \"" + saveFile.name() +"\"",QMessageBox::Ok,QMessageBox::NoButton);
    }
  }
}





