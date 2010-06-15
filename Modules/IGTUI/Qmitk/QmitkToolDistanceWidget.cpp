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

#include "QmitkToolDistanceWidget.h"

#include <QGridLayout>
#include <itkDataObject.h>
#include <mitkNavigationDataSource.h>
#include <math.h>




QmitkToolDistanceWidget::QmitkToolDistanceWidget(QWidget* parent)  
: QWidget(parent), m_Controls(NULL), m_DistanceLabels(NULL)
{
  this->CreateQtPartControl( this );
}

QmitkToolDistanceWidget::~QmitkToolDistanceWidget()
{
  ClearDistanceMatrix();
  delete m_DistanceLabels;
  m_DistanceLabels = NULL;
  m_Controls = NULL;
}

void QmitkToolDistanceWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkToolDistanceWidgetControls;
    m_Controls->setupUi(parent);
    m_Controls->m_StatusLabel->setText(QString("No tracking tools connected. Please set up a connection first."));

    this->CreateConnections();
  }
}

void QmitkToolDistanceWidget::CreateConnections()
{

}

void QmitkToolDistanceWidget::CreateToolDistanceMatrix(itk::ProcessObject::DataObjectPointerArray & outputs)
{

  if(outputs.size() > 1)
  {
    this->show();

    mitk::NavigationData* navData; 


    if(m_DistanceLabels == NULL)
    {
      m_DistanceLabels = new DistanceLabelType;
    }

    if(m_DistanceLabels->isEmpty()) 
    {
      this->m_Controls->m_StatusLabel->setText("");

      QLabel* label;


      // labeling of matrix
      for (unsigned int i = 0; i < outputs.size()-1; i++)
      { 
        navData = dynamic_cast<mitk::NavigationData*>(outputs.at(i).GetPointer());
        label = new QLabel(navData->GetName(),this);
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        this->m_Controls->m_GridLayout->addWidget(label,i+1,0);

        navData = dynamic_cast<mitk::NavigationData*>(outputs.at(i+1).GetPointer());
        label = new QLabel(navData->GetName(),this);
        label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        this->m_Controls->m_GridLayout->addWidget(label,0,i+1);
      }


      for(unsigned int i = 0; i < outputs.size()-1; i++)
      {
        QVector<QLabel*>* rowDistances = new QVector<QLabel*>();

        for(unsigned int j = i+1; j < outputs.size(); j++) 
        {
          // distance labels initializing
          label = new QLabel(QString("---"), this);
          label->setFrameStyle(QFrame::Box | QFrame::Sunken);
          label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
          rowDistances->append(label);

          //initial distance label adding to the QGridLayout
          this->m_Controls->m_GridLayout->addWidget(label,i+1,j);
        }
        this->m_DistanceLabels->append(*rowDistances);
      }
    }
  }
  else
  {
    this->m_Controls->m_StatusLabel->setText(QString("For distance information please connect at least two tools"));
  }

}



  void QmitkToolDistanceWidget::ShowDistanceValues(itk::ProcessObject::DataObjectPointerArray & outputs)
  {

    mitk::NavigationData* navData; 
    mitk::NavigationData* nextNavData;

    for(int i=0; i < m_DistanceLabels->size(); i++)
    {
      int j = i+1;

      for(int k=0; k < m_DistanceLabels->at(i).size(); k++)
      {
        navData = dynamic_cast<mitk::NavigationData*>(outputs.at(i).GetPointer());
        nextNavData = dynamic_cast<mitk::NavigationData*>(outputs.at(j++).GetPointer());

        if(navData == NULL || nextNavData == NULL)
          return;

        mitk::NavigationData::PositionType::RealType distance =  navData->GetPosition().EuclideanDistanceTo(nextNavData->GetPosition());
        distance = floor(distance * 10.) / 10.;
        QString distanceStr;
        if(!navData->IsDataValid() || !nextNavData->IsDataValid()) 
        {
          distanceStr = "---";
        }
        else
        {
          distanceStr.setNum(distance);
          distanceStr += " mm";
        }
        this->m_DistanceLabels->at(i).at(k)->setText(distanceStr);
      }
    }
  }

  void QmitkToolDistanceWidget::ClearDistanceMatrix() 
  {

    while(m_Controls->m_GridLayout->count() > 0)
    {
      QWidget* widget = m_Controls->m_GridLayout->itemAt(0)->widget();
      m_Controls->m_GridLayout->removeWidget(widget);
      delete widget;
    }
   delete this->m_DistanceLabels;
   this->m_DistanceLabels = NULL;

   this->m_Controls->m_StatusLabel->setText(QString("For distance information please set up the connection again."));
 
  }

  void QmitkToolDistanceWidget::SetDistanceLabelValuesInvalid()
  {
    for(int i = 0; i < m_DistanceLabels->size(); i++)
    {
      for(int j= 0; j < m_DistanceLabels->at(i).size(); j++)
      {
        this->m_DistanceLabels->at(i).at(j)->setText(QString("---"));
      }
    }
  }




