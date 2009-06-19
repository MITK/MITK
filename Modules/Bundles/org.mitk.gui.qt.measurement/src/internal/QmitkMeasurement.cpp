/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 15412 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkMeasurement.h"
// #include <qaction.h>

#include <mitkGlobalInteraction.h>

#include <mitkPointSet.h>

#include <mitkProperties.h>
#include <mitkStringProperty.h>
#include <mitkIDataStorageService.h>

#include <cherryIEditorPart.h>
#include <cherryIWorkbenchPage.h>
#include <cherryPlatform.h>
/*
QmitkMeasurement::QmitkMeasurement(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
  : QmitkFunctionality(parent, name, it) , m_MultiWidget(mitkStdMultiWidget), m_Controls(NULL)
{
  SetAvailability(true);
}
*/

QmitkMeasurement::~QmitkMeasurement()
{
  this->Deactivated();
  delete m_Controls;
}

/*
QWidget * QmitkMeasurement::CreateMainWidget(QWidget *)
{
  return NULL;
}

QWidget * QmitkMeasurement::CreateControlWidget(QWidget *parent)
{
  if (m_Controls == NULL)
  {
    m_Controls = new Ui::QmitkMeasurementControls(parent);
  }
  return m_Controls;
}
*/

void QmitkMeasurement::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->pbDistance), SIGNAL(clicked()),(QObject*) this, SLOT(AddDistanceMeasurement()) );
    connect( (QObject*)(m_Controls->pbAngle), SIGNAL(clicked()),(QObject*) this, SLOT(AddAngleMeasurement()) );
    connect( (QObject*)(m_Controls->pbPath), SIGNAL(clicked()),(QObject*) this, SLOT(AddPathMeasurement()) );
  }
}

/*
QAction * QmitkMeasurement::CreateAction(QActionGroup *parent)
{
  QAction* action;
  action = new QAction( tr( "Measurement" ), QPixmap((const char**)icon_xpm), tr( "&Measurement" ), 0, parent, "Measurement" );
  return action;
}
*/

void QmitkMeasurement::TreeChanged()
{

}

void QmitkMeasurement::Activated()
{
  m_PointSetInteractor = 0u;
  m_CurrentPointSetNode = 0u;
}

void QmitkMeasurement::Deactivated()
{
  if (m_PointSetInteractor.IsNotNull())
  {
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_PointSetInteractor);
  }

  if (m_CurrentPointSetNode.IsNotNull() && static_cast<mitk::PointSet*>(m_CurrentPointSetNode->GetData())->IsEmpty(0u))
  {
    this->GetDataStorage()->Remove(m_CurrentPointSetNode);
  }
}

void QmitkMeasurement::AddDistanceMeasurement()
{
  /*
   * make sure the last interactor that has been used is removed
   */
  if (m_PointSetInteractor.IsNotNull())
  {
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_PointSetInteractor);
  }

  /*
   * if the last point set was not used for a measurement delete it
   */
  if (m_CurrentPointSetNode.IsNotNull() && static_cast<mitk::PointSet*>(m_CurrentPointSetNode->GetData())->IsEmpty(0u))
  {
    this->GetDataStorage()->Remove(m_CurrentPointSetNode);
  }

  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();

  m_CurrentPointSetNode = mitk::DataTreeNode::New();
  m_CurrentPointSetNode->SetData(pointSet);
  m_CurrentPointSetNode->SetProperty("show contour", mitk::BoolProperty::New(true));
  m_CurrentPointSetNode->SetProperty("name", mitk::StringProperty::New("distance"));
  m_CurrentPointSetNode->SetProperty("show distances", mitk::BoolProperty::New(true));

  this->GetDataStorage()->Add(m_CurrentPointSetNode);

  m_PointSetInteractor = mitk::PointSetInteractor::New("pointsetinteractor", m_CurrentPointSetNode, 2);
  mitk::GlobalInteraction::GetInstance()->AddInteractor(m_PointSetInteractor);
}

void QmitkMeasurement::AddAngleMeasurement()
{
  /*
   * make sure the last interactor that has been used is removed
   */
  if (m_PointSetInteractor.IsNotNull())
  {
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_PointSetInteractor);
  }

 /*
  * if the last point set was not used for a measurement delete it
  */
  if (m_CurrentPointSetNode.IsNotNull() && static_cast<mitk::PointSet*>(m_CurrentPointSetNode->GetData())->IsEmpty(0u))
  {
    this->GetDataStorage()->Remove(m_CurrentPointSetNode);
  }

  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();

  m_CurrentPointSetNode = mitk::DataTreeNode::New();
  m_CurrentPointSetNode->SetData(pointSet);
  m_CurrentPointSetNode->SetProperty("show contour", mitk::BoolProperty::New(true));
  m_CurrentPointSetNode->SetProperty("name", mitk::StringProperty::New("angle"));
  m_CurrentPointSetNode->SetProperty("show angles", mitk::BoolProperty::New(true));

  this->GetDataStorage()->Add(m_CurrentPointSetNode);

  m_PointSetInteractor = mitk::PointSetInteractor::New("pointsetinteractor", m_CurrentPointSetNode, 3);
  mitk::GlobalInteraction::GetInstance()->AddInteractor(m_PointSetInteractor);
}

void QmitkMeasurement::AddPathMeasurement()
{
  /*
   * make sure the last interactor that has been used is removed
   */
  if (m_PointSetInteractor.IsNotNull())
  {
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_PointSetInteractor);
  }

 /*
  * if the last point set was not used for a measurement delete it
  */
  if (m_CurrentPointSetNode.IsNotNull() && static_cast<mitk::PointSet*>(m_CurrentPointSetNode->GetData())->IsEmpty(0u))
  {
    this->GetDataStorage()->Remove(m_CurrentPointSetNode);
  }

  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();

  m_CurrentPointSetNode = mitk::DataTreeNode::New();
  m_CurrentPointSetNode->SetData(pointSet);
  m_CurrentPointSetNode->SetProperty("show contour", mitk::BoolProperty::New(true));
  m_CurrentPointSetNode->SetProperty("name", mitk::StringProperty::New("path"));
  m_CurrentPointSetNode->SetProperty("show distances", mitk::BoolProperty::New(true));
  m_CurrentPointSetNode->SetProperty("show angles", mitk::BoolProperty::New(true));

  //m_DataTreeIterator->Add(m_CurrentPointSetNode);
  this->GetDataStorage()->Add(m_CurrentPointSetNode);

  m_PointSetInteractor = mitk::PointSetInteractor::New("pointsetinteractor", m_CurrentPointSetNode);
  mitk::GlobalInteraction::GetInstance()->AddInteractor(m_PointSetInteractor);
}

void QmitkMeasurement::CreateQtPartControl( QWidget* parent )
{
  m_Parent = parent;

  m_Controls = new Ui::QmitkMeasurementControls;
  m_Controls->setupUi(parent);

  m_PointSetInteractor = 0u;

  m_CurrentPointSetNode = 0u;

  this->CreateConnections();
  //SetAvailability(true);
}

void QmitkMeasurement::StdMultiWidgetAvailable(QmitkStdMultiWidget& stdMultiWidget)
{
  this->m_Controls->pbAngle->setEnabled(true);
  this->m_Controls->pbDistance->setEnabled(true);
  this->m_Controls->pbPath->setEnabled(true);
}

void QmitkMeasurement::StdMultiWidgetNotAvailable()
{
  this->m_Controls->pbAngle->setEnabled(false);
  this->m_Controls->pbDistance->setEnabled(false);
  this->m_Controls->pbPath->setEnabled(false);
}
