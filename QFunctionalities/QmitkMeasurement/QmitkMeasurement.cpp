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

#include "QmitkMeasurement.h"
#include "QmitkMeasurementControls.h"
#include <qaction.h>
#include "icon.xpm"

#include <mitkGlobalInteraction.h>

#include <mitkPointSet.h>
#include <mitkPointSetInteractor.h>

#include <mitkProperties.h>
#include <mitkStringProperty.h>

QmitkMeasurement::QmitkMeasurement(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
  : QmitkFunctionality(parent, name, it) , m_MultiWidget(mitkStdMultiWidget), m_Controls(NULL)
{
  SetAvailability(true);
}

QmitkMeasurement::~QmitkMeasurement()
{
}

QWidget * QmitkMeasurement::CreateMainWidget(QWidget *)
{
  return NULL;
}

QWidget * QmitkMeasurement::CreateControlWidget(QWidget *parent)
{
  if (m_Controls == NULL)
  {
    m_Controls = new QmitkMeasurementControls(parent);
  }
  return m_Controls;
}

void QmitkMeasurement::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->pbDistance), SIGNAL(clicked()),(QObject*) this, SLOT(AddDistanceMeasurement()) );
    connect( (QObject*)(m_Controls->pbAngle), SIGNAL(clicked()),(QObject*) this, SLOT(AddAngleMeasurement()) );
    connect( (QObject*)(m_Controls->pbPath), SIGNAL(clicked()),(QObject*) this, SLOT(AddPathMeasurement()) );
  }
}

QAction * QmitkMeasurement::CreateAction(QActionGroup *parent)
{
  QAction* action;
  action = new QAction( tr( "Measurement" ), QPixmap((const char**)icon_xpm), tr( "&Measurement" ), 0, parent, "Measurement" );
  return action;
}

void QmitkMeasurement::TreeChanged()
{

}

void QmitkMeasurement::Activated()
{
  QmitkFunctionality::Activated();
}

void QmitkMeasurement::AddDistanceMeasurement()
{
  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
  mitk::DataTreeNode::Pointer pointSetNode = mitk::DataTreeNode::New();
  pointSetNode->SetData(pointSet);
  pointSetNode->SetProperty("contour", new mitk::BoolProperty(true));
  pointSetNode->SetProperty("name", new mitk::StringProperty("distance"));
  pointSetNode->SetProperty("show distances", new mitk::BoolProperty(true));

  m_DataTreeIterator->Add(pointSetNode);

  mitk::GlobalInteraction::GetInstance()->AddInteractor(
    mitk::PointSetInteractor::New("pointsetinteractor", pointSetNode,2)
  );
}

void QmitkMeasurement::AddAngleMeasurement()
{
  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
  mitk::DataTreeNode::Pointer pointSetNode = mitk::DataTreeNode::New();
  pointSetNode->SetData(pointSet);
  pointSetNode->SetProperty("contour", new mitk::BoolProperty(true));
  pointSetNode->SetProperty("name", new mitk::StringProperty("angle"));
  pointSetNode->SetProperty("show angles", new mitk::BoolProperty(true));

  m_DataTreeIterator->Add(pointSetNode);

  mitk::GlobalInteraction::GetInstance()->AddInteractor(
    mitk::PointSetInteractor::New("pointsetinteractor", pointSetNode,3)
  );
}

void QmitkMeasurement::AddPathMeasurement()
{
  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
  mitk::DataTreeNode::Pointer pointSetNode = mitk::DataTreeNode::New();
  pointSetNode->SetData(pointSet);
  pointSetNode->SetProperty("contour", new mitk::BoolProperty(true));
  pointSetNode->SetProperty("name", new mitk::StringProperty("path"));
  pointSetNode->SetProperty("show distances", new mitk::BoolProperty(true));
  pointSetNode->SetProperty("show angles", new mitk::BoolProperty(true));

  m_DataTreeIterator->Add(pointSetNode);

  mitk::GlobalInteraction::GetInstance()->AddInteractor(
    mitk::PointSetInteractor::New("pointsetinteractor", pointSetNode)
  );
}
