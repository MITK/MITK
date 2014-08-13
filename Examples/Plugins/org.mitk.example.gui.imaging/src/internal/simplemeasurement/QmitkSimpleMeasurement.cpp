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

#include "QmitkSimpleMeasurement.h"

#include "ui_QmitkSimpleMeasurementControls.h"

#include <mitkGlobalInteraction.h>
#include <mitkPointSet.h>
#include <mitkProperties.h>
#include <mitkStringProperty.h>
#include <mitkIDataStorageService.h>

#include <berryIEditorPart.h>
#include <berryIWorkbenchPage.h>
#include <berryPlatform.h>


QmitkSimpleMeasurement::QmitkSimpleMeasurement()
{
}

QmitkSimpleMeasurement::~QmitkSimpleMeasurement()
{
  //remove all measurements when view is closed
  for (std::size_t i=0; i<m_CreatedDistances.size(); i++)
  {
    this->GetDataStorage()->Remove(m_CreatedDistances.at(i));
  }
  for (std::size_t i=0; i<m_CreatedAngles.size(); i++)
  {
    this->GetDataStorage()->Remove(m_CreatedAngles.at(i));
  }
  for (std::size_t i=0; i<m_CreatedPaths.size(); i++)
  {
    this->GetDataStorage()->Remove(m_CreatedPaths.at(i));
  }

  if (m_PointSetInteractor.IsNotNull())
  {
    m_SelectedPointSetNode->SetDataInteractor(NULL);
  }
}

void QmitkSimpleMeasurement::Activated()
{
  this->OnSelectionChanged(berry::IWorkbenchPart::Pointer(), this->GetCurrentSelection());
}

void QmitkSimpleMeasurement::Deactivated()
{
}

void QmitkSimpleMeasurement::Visible()
{
}

void QmitkSimpleMeasurement::Hidden()
{
}

void QmitkSimpleMeasurement::ActivatedZombieView(berry::SmartPointer<berry::IWorkbenchPartReference> /*zombieView*/)
{
  // something else was selected. remove old interactor
  if (m_PointSetInteractor.IsNotNull())
  {
    m_SelectedPointSetNode->SetDataInteractor(NULL);
  }
}

void QmitkSimpleMeasurement::AddDistanceSimpleMeasurement()
{
  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();

  QString name = "Distance " + QString::number(m_CreatedDistances.size()+1);

  mitk::DataNode::Pointer CurrentPointSetNode = mitk::DataNode::New();
  CurrentPointSetNode->SetData(pointSet);
  CurrentPointSetNode->SetProperty("show contour", mitk::BoolProperty::New(true));
  CurrentPointSetNode->SetProperty("name", mitk::StringProperty::New(name.toStdString()));
  CurrentPointSetNode->SetProperty("show distances", mitk::BoolProperty::New(true));

  // add to ds and remember as created
  m_CreatedDistances.push_back(CurrentPointSetNode);
  this->GetDataStorage()->Add(CurrentPointSetNode);

  // make new selection
  QList<mitk::DataNode::Pointer> selection;
  selection.push_back( CurrentPointSetNode );
  this->FireNodesSelected( selection );
  this->OnSelectionChanged(berry::IWorkbenchPart::Pointer(), selection );
}

void QmitkSimpleMeasurement::AddAngleSimpleMeasurement()
{
  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();

  QString name = "Angle " + QString::number(m_CreatedAngles.size()+1);

  mitk::DataNode::Pointer _CurrentPointSetNode = mitk::DataNode::New();
  _CurrentPointSetNode->SetData(pointSet);
  _CurrentPointSetNode->SetProperty("show contour", mitk::BoolProperty::New(true));
  _CurrentPointSetNode->SetProperty("name", mitk::StringProperty::New(name.toStdString()));
  _CurrentPointSetNode->SetProperty("show angles", mitk::BoolProperty::New(true));

  // add to ds and remember as created
  this->GetDataStorage()->Add(_CurrentPointSetNode);
  m_CreatedAngles.push_back( _CurrentPointSetNode );

  // make new selection
  QList<mitk::DataNode::Pointer> selection;
  selection.push_back( _CurrentPointSetNode );
  this->FireNodesSelected( selection );
  this->OnSelectionChanged(berry::IWorkbenchPart::Pointer(), selection );
}

void QmitkSimpleMeasurement::AddPathSimpleMeasurement()
{
  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();

  QString name = "Path " + QString::number(m_CreatedPaths.size()+1);

  mitk::DataNode::Pointer _CurrentPointSetNode = mitk::DataNode::New();
  _CurrentPointSetNode->SetData(pointSet);
  _CurrentPointSetNode->SetProperty("show contour", mitk::BoolProperty::New(true));
  _CurrentPointSetNode->SetProperty("name", mitk::StringProperty::New(name.toStdString()));
  _CurrentPointSetNode->SetProperty("show distances", mitk::BoolProperty::New(true));
  _CurrentPointSetNode->SetProperty("show angles", mitk::BoolProperty::New(true));

  // add to ds and remember as created
  this->GetDataStorage()->Add(_CurrentPointSetNode);
  m_CreatedPaths.push_back( _CurrentPointSetNode );

  // make new selection
  QList<mitk::DataNode::Pointer> selection;
  selection.push_back( _CurrentPointSetNode );
  this->FireNodesSelected( selection );
  this->OnSelectionChanged(berry::IWorkbenchPart::Pointer(), selection );
}

void QmitkSimpleMeasurement::CreateQtPartControl( QWidget* parent )
{
  m_CreatedDistances = std::vector<mitk::DataNode::Pointer>();
  m_CreatedAngles = std::vector<mitk::DataNode::Pointer>();
  m_CreatedPaths = std::vector<mitk::DataNode::Pointer>();


  m_Controls = new Ui::QmitkSimpleMeasurementControls;
  m_Controls->setupUi(parent);

  connect( (QObject*)(m_Controls->pbDistance), SIGNAL(clicked()),(QObject*) this, SLOT(AddDistanceSimpleMeasurement()) );
  connect( (QObject*)(m_Controls->pbAngle), SIGNAL(clicked()),(QObject*) this, SLOT(AddAngleSimpleMeasurement()) );
  connect( (QObject*)(m_Controls->pbPath), SIGNAL(clicked()),(QObject*) this, SLOT(AddPathSimpleMeasurement()) );
  connect( (QObject*)(m_Controls->m_Finished), SIGNAL(clicked()),(QObject*) this, SLOT(Finished()) );

  EndEditingMeasurement();

}

void QmitkSimpleMeasurement::SetFocus()
{
  m_Controls->m_Finished->setFocus();
}

void QmitkSimpleMeasurement::Finished()
{
  OnSelectionChanged(berry::IWorkbenchPart::Pointer(), QList<mitk::DataNode::Pointer>());
}

void QmitkSimpleMeasurement::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/, const QList<mitk::DataNode::Pointer> &nodes)
{
  mitk::DataNode::Pointer selectedNode ;
  if(nodes.size() > 0)
    selectedNode = nodes.front();

  mitk::PointSet* pointSet = 0;
  if(selectedNode.IsNotNull())
    pointSet = dynamic_cast<mitk::PointSet*> ( selectedNode->GetData() );

  // something else was selected. remove old interactor
  if (m_PointSetInteractor.IsNotNull())
  {
    m_SelectedPointSetNode->SetDataInteractor(NULL);
  }

  bool pointsetCreatedByThis = false;

  // only go further if a pointset was selected
  if(pointSet)
  {
    // see if this pointset was created by us
    std::vector<mitk::DataNode::Pointer>::iterator it = std::find( m_CreatedDistances.begin()
      , m_CreatedDistances.end(), selectedNode);
    if (it != m_CreatedDistances.end()) pointsetCreatedByThis = true;

    it = std::find( m_CreatedAngles.begin()
      , m_CreatedAngles.end(), selectedNode);
    if (it != m_CreatedAngles.end()) pointsetCreatedByThis = true;

    it = std::find( m_CreatedPaths.begin()
      , m_CreatedPaths.end(), selectedNode);
    if (it != m_CreatedPaths.end()) pointsetCreatedByThis = true;
  }

  // do nothing if it was not created by us or it is no pointset node or we are not activated
  if(pointsetCreatedByThis)
  {
    // otherwise: set text and add interactor for the pointset
    m_Controls->selectedPointSet->setText( QString::fromStdString(selectedNode->GetName()) );

    m_PointSetInteractor = mitk::PointSetDataInteractor::New();
    m_PointSetInteractor->LoadStateMachine("PointSet.xml");
    m_PointSetInteractor->SetEventConfig("PointSetConfig.xml");
    m_PointSetInteractor->SetDataNode(selectedNode);

    float green[] = { 0, 255, 0 };
    float red[] = { 255, 0, 0 };
    selectedNode->SetColor(green);
    if(m_SelectedPointSetNode.IsNotNull())
      m_SelectedPointSetNode->SetColor(red);
    m_SelectedPointSetNode = selectedNode;
    StartEditingMeasurement();
  }
  else
  {
    EndEditingMeasurement();
  }
}

void QmitkSimpleMeasurement::NodeRemoved( const mitk::DataNode* node )
{
  // remove a node if it is destroyed from our created array
  m_CreatedDistances.erase(std::remove(m_CreatedDistances.begin(), m_CreatedDistances.end(), node), m_CreatedDistances.end());
  m_CreatedAngles.erase(std::remove(m_CreatedAngles.begin(), m_CreatedAngles.end(), node), m_CreatedAngles.end());
  m_CreatedPaths.erase(std::remove(m_CreatedPaths.begin(), m_CreatedPaths.end(), node), m_CreatedPaths.end());
}

void QmitkSimpleMeasurement::StartEditingMeasurement()
{
  m_Controls->explain_label->setVisible(true);
  m_Controls->m_Finished->setVisible(true);
  m_Controls->pbDistance->setEnabled(false);
  m_Controls->pbAngle->setEnabled(false);
  m_Controls->pbPath->setEnabled(false);
  UpdateMeasurementList();
}

void QmitkSimpleMeasurement::EndEditingMeasurement()
{
  m_Controls->pbDistance->setEnabled(true);
  m_Controls->pbAngle->setEnabled(true);
  m_Controls->pbPath->setEnabled(true);
  m_Controls->explain_label->setVisible(false);
  m_Controls->m_Finished->setVisible(false);
  m_Controls->selectedPointSet->setText( "None" );
  UpdateMeasurementList();
}

void QmitkSimpleMeasurement::UpdateMeasurementList()
{
  m_Controls->m_MeasurementList->clear();

  for (std::size_t i=0; i<m_CreatedDistances.size(); i++)
  {
    QListWidgetItem *newItem = new QListWidgetItem;
    QString distance;
    mitk::PointSet::Pointer points = dynamic_cast<mitk::PointSet*>(m_CreatedDistances.at(i)->GetData());
    if(points->GetSize()<2)
    {
      distance = "not available";
    }
    else
    {
      distance = QString::number(points->GetPoint(0).EuclideanDistanceTo(points->GetPoint(1))) + " mm";
    }
    QString name = QString(m_CreatedDistances.at(i)->GetName().c_str()) + " (" + distance + ")";
    newItem->setText(name);
    m_Controls->m_MeasurementList->insertItem(m_Controls->m_MeasurementList->count(), newItem);
  }
  for (std::size_t i=0; i<m_CreatedAngles.size() && i<m_CreatedDistances.size(); i++)
  {
    QListWidgetItem *newItem = new QListWidgetItem;
    QString name = m_CreatedDistances.at(i)->GetName().c_str();
    newItem->setText(name);
    m_Controls->m_MeasurementList->insertItem(m_Controls->m_MeasurementList->count(), newItem);
  }
  for (std::size_t i=0; i<m_CreatedPaths.size() && i<m_CreatedDistances.size(); i++)
  {
    QListWidgetItem *newItem = new QListWidgetItem;
    QString name = m_CreatedDistances.at(i)->GetName().c_str();
    newItem->setText(name);
    m_Controls->m_MeasurementList->insertItem(m_Controls->m_MeasurementList->count(), newItem);
  }
}
