/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-03-21 19:27:37 +0100 (Sa, 21 Mrz 2009) $
Version:   $Revision: 16719 $ 

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkToolTrackingStatusWidget.h"





QmitkToolTrackingStatusWidget::QmitkToolTrackingStatusWidget(QWidget* parent)  
: QWidget(parent), m_Controls(NULL), m_StatusLabels (NULL), m_NavigationDatas(NULL)
{
  m_ShowPositions = false;
  m_ShowQuaternions = false;
  m_Alignment = Qt::AlignHCenter;
  m_Style = QmitkToolTrackingStatusWidget::VerticalUpperStyle;
  
  CreateQtPartControl( this );
 
}


void QmitkToolTrackingStatusWidget::SetStyle(QmitkToolTrackingStatusWidget::Style newStyle)
  {
  //set new style
  m_Style = newStyle;

  //update current labels to show them in the new style
  Refresh();
  }

void QmitkToolTrackingStatusWidget::SetShowPositions(bool enable)
{
  m_ShowPositions = enable;

  //update current labels
  Refresh();
}

void QmitkToolTrackingStatusWidget::SetShowQuaternions(bool enable)
{
  m_ShowQuaternions = enable;

  //update current labels
  Refresh();
}

void QmitkToolTrackingStatusWidget::SetTextAlignment(Qt::AlignmentFlag alignment)
{
  m_Alignment = alignment;

  //update current labels
  Refresh();
}

QmitkToolTrackingStatusWidget::~QmitkToolTrackingStatusWidget()
{
  //m_Controls = NULL;
  delete m_StatusLabels;
  delete m_NavigationDatas;
}

void QmitkToolTrackingStatusWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkToolTrackingStatusWidgetControls;
    m_Controls->setupUi(parent);

    this->CreateConnections();

    //add empty label
    AddEmptyLabel();
  }
}

void QmitkToolTrackingStatusWidget::CreateConnections()
{

}


void QmitkToolTrackingStatusWidget::SetNavigationDatas(std::vector<mitk::NavigationData::Pointer>* navDatas)
{  
  m_NavigationDatas = navDatas;
}


void QmitkToolTrackingStatusWidget::AddNavigationData(mitk::NavigationData::Pointer nd)
{
  if(m_NavigationDatas == NULL)
    m_NavigationDatas = new std::vector<mitk::NavigationData::Pointer>();

  m_NavigationDatas->push_back(nd);
}


void QmitkToolTrackingStatusWidget::Refresh()
{

  if(m_NavigationDatas == NULL || m_NavigationDatas->size() <= 0)
    { 
    RemoveGuiLabels();
    AddEmptyLabel();
    return;
    }


  mitk::NavigationData* navData; 

  for(unsigned int i = 0; i < m_NavigationDatas->size(); i++)
  {
    navData = m_NavigationDatas->at(i).GetPointer();
    QString name(navData->GetName());
    QString pos = "";
    QString quat = "";
    if (m_ShowPositions)
      {
      mitk::Point3D position = navData->GetPosition();
      pos = " [" + QString::number(position[0]) + ";" + QString::number(position[1]) + ";" + QString::number(position[2]) + "]";
      }
    if (m_ShowQuaternions)
      {
      mitk::Quaternion quaternion = navData->GetOrientation();
      quat = " / [qx:" + QString::number(quaternion.x()) + ";qy:" + QString::number(quaternion.y()) + ";qz:" + QString::number(quaternion.z()) + ";qr:" + QString::number(quaternion.r()) + "]";
      }

    if(name.compare(m_StatusLabels->at(i)->objectName()) == 0)
      {
      m_StatusLabels->at(i)->setText(name+pos+quat);
      if(navData->IsDataValid())
        m_StatusLabels->at(i)->setStyleSheet("QLabel{background-color: #8bff8b }");
      
      else
        m_StatusLabels->at(i)->setStyleSheet("QLabel{background-color: #ff7878 }");
      }
  }
}


void QmitkToolTrackingStatusWidget::ShowStatusLabels()
{
  RemoveGuiLabels(); 

  if(m_NavigationDatas == NULL || m_NavigationDatas->size() <= 0)
    {
    RemoveGuiLabels();
    AddEmptyLabel();
    return;
    }


  m_StatusLabels = new QVector<QLabel*>();
  mitk::NavigationData* navData;
  QLabel* label;


  for(unsigned int i = 0; i < m_NavigationDatas->size(); i++)
  {
    navData = m_NavigationDatas->at(i).GetPointer();

    QString name(navData->GetName());
   
    label = new QLabel(name, this);
    label->setObjectName(name);
    label->setAlignment(m_Alignment | Qt::AlignVCenter);
    label->setFrameStyle(QFrame::Panel | QFrame::Sunken);

    m_StatusLabels->append(label);
    if (m_Style == QmitkToolTrackingStatusWidget::VerticalUpperStyle) m_Controls->m_VerticalLayout->addWidget(m_StatusLabels->at(i));
    else m_Controls->m_GridLayout->addWidget(m_StatusLabels->at(i),0,i);
    
  }
}

void QmitkToolTrackingStatusWidget::PreShowTools(mitk::NavigationToolStorage::Pointer toolStorage)
  {
  RemoveGuiLabels();
  QLabel* label;

  for(unsigned int i = 0; i < toolStorage->GetToolCount(); i++)
    {
    QString name(toolStorage->GetTool(i)->GetToolName().c_str());
   
    label = new QLabel(name, this);
    label->setObjectName(name);
    label->setAlignment(m_Alignment | Qt::AlignVCenter);
    label->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    label->setStyleSheet("QLabel{background-color: #dddddd }");
    if (m_Style == QmitkToolTrackingStatusWidget::VerticalUpperStyle) m_Controls->m_VerticalLayout->addWidget(label);
    else m_Controls->m_GridLayout->addWidget(label);
    
    }

  }


void QmitkToolTrackingStatusWidget::RemoveStatusLabels()
{
  //remove GUI elements
  RemoveGuiLabels();  

  //clear members
  if(m_StatusLabels != NULL && m_StatusLabels->size() > 0)
    {
    delete m_StatusLabels;
    m_StatusLabels = new QVector< QLabel* >();
    }

  if(m_NavigationDatas != NULL && m_NavigationDatas->size() > 0)
    {
    delete m_NavigationDatas;
    m_NavigationDatas = new std::vector<mitk::NavigationData::Pointer>();
    }

  //add empty label
  AddEmptyLabel();

}

void QmitkToolTrackingStatusWidget::RemoveGuiLabels()
  {
  while(m_Controls->m_GridLayout->count() > 0 || m_Controls->m_VerticalLayout->count() > 0)
    {
    if (m_Controls->m_GridLayout->count() > 0)
      {
      QWidget* actWidget = m_Controls->m_GridLayout->itemAt(0)->widget();
      m_Controls->m_GridLayout->removeWidget(actWidget);
      delete actWidget;
      }
    else if (m_Controls->m_VerticalLayout->count() > 0)
      {
      QWidget* actWidget = m_Controls->m_VerticalLayout->itemAt(0)->widget();
      m_Controls->m_VerticalLayout->removeWidget(actWidget);
      delete actWidget;
      }
    }
  }

void QmitkToolTrackingStatusWidget::AddEmptyLabel()
  {
   //add a label which tells that no tools are loaded yet
  QLabel* label = new QLabel("No tools loaded yet.", this);
  label->setObjectName("No tools loaded yet.");
  label->setAlignment(m_Alignment | Qt::AlignVCenter);
  label->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  label->setStyleSheet("QLabel{background-color: #dddddd }");
  if (m_Style == QmitkToolTrackingStatusWidget::VerticalUpperStyle) m_Controls->m_VerticalLayout->addWidget(label);
  else m_Controls->m_GridLayout->addWidget(label);
  }



