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

#include <itkDataObject.h>
#include <mitkNavigationDataSource.h>
#include <QWidgetItem>



QmitkToolTrackingStatusWidget::QmitkToolTrackingStatusWidget(QWidget* parent)  
: QWidget(parent), m_Controls(NULL), m_StatusLabels (NULL)
{
  this->CreateQtPartControl( this );
}

QmitkToolTrackingStatusWidget::~QmitkToolTrackingStatusWidget()
{
  m_Controls = NULL;
}

void QmitkToolTrackingStatusWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkToolTrackingStatusWidgetControls;
    m_Controls->setupUi(parent);

    this->CreateConnections();
  }
}

void QmitkToolTrackingStatusWidget::CreateConnections()
{

}

void QmitkToolTrackingStatusWidget::ShowStatus(itk::ProcessObject::DataObjectPointerArray & outputs)
{
  mitk::NavigationData* navData; 

  for(unsigned int i = 0; i < outputs.size(); i++)
  {
    navData = dynamic_cast<mitk::NavigationData*>(outputs.at(i).GetPointer());

    QString name(navData->GetName());

    if(name.compare(m_StatusLabels->at(i)->text()) == 0)
    {
      if(navData->IsDataValid())
        m_StatusLabels->at(i)->setStyleSheet("QLabel{background-color: #8bff8b }");
      else
        m_StatusLabels->at(i)->setStyleSheet("QLabel{background-color: #ff7878 }");
    }

  }

}


void QmitkToolTrackingStatusWidget::SetupStatusLabels(itk::ProcessObject::DataObjectPointerArray & outputs)
{
  m_StatusLabels = new QVector<QLabel*>();
  mitk::NavigationData* navData; 
  QLabel* label;

  for(unsigned int i = 0; i < outputs.size(); i++)
  {
    navData = dynamic_cast<mitk::NavigationData*>(outputs.at(i).GetPointer());

    QString name(navData->GetName());

    label = new QLabel(name, this);
    label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    label->setFrameStyle(QFrame::Panel | QFrame::Sunken);


    m_StatusLabels->append(label);
    m_Controls->m_GridLayout->addWidget(m_StatusLabels->at(i),0,i);
  }
}


void QmitkToolTrackingStatusWidget::RemoveStatusLabels()
{

  while(m_Controls->m_GridLayout->count() > 0)
  {
    QWidget* actWidget = m_Controls->m_GridLayout->itemAt(0)->widget();
    m_Controls->m_GridLayout->removeWidget(actWidget);
    delete actWidget;
  }

  delete this->m_StatusLabels;
  this->m_StatusLabels = NULL;

}




