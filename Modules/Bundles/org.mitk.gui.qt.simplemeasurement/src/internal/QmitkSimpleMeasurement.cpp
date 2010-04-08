/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-06-18 16:47:58 +0200 (Thu, 18 Jun 2009) $
Version:   $Revision: 15412 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkSimpleMeasurement.h"
// #include <qaction.h>

#include <mitkGlobalInteraction.h>

#include <mitkPointSet.h>

#include <mitkProperties.h>
#include <mitkStringProperty.h>
#include <mitkIDataStorageService.h>

#include <berryIEditorPart.h>
#include <berryIWorkbenchPage.h>
#include <berryPlatform.h>
/*
QmitkSimpleMeasurement::QmitkSimpleMeasurement(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
  : QmitkFunctionality(parent, name, it) , m_MultiWidget(mitkStdMultiWidget), m_Controls(NULL)
{
  SetAvailability(true);
}
*/

QmitkSimpleMeasurement::~QmitkSimpleMeasurement()
{
}

void QmitkSimpleMeasurement::Activated()
{
  std::vector<mitk::DataNode*> selection = this->GetDataManagerSelection();
  this->OnSelectionChanged( selection );
}

void QmitkSimpleMeasurement::Deactivated()
{
  std::vector<mitk::DataNode*> selection;
  this->OnSelectionChanged( selection );
}

void QmitkSimpleMeasurement::AddDistanceSimpleMeasurement()
{
  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();

  mitk::DataNode::Pointer _CurrentPointSetNode = mitk::DataNode::New();
  _CurrentPointSetNode->SetData(pointSet);
  _CurrentPointSetNode->SetProperty("show contour", mitk::BoolProperty::New(true));
  _CurrentPointSetNode->SetProperty("name", mitk::StringProperty::New("distance"));
  _CurrentPointSetNode->SetProperty("show distances", mitk::BoolProperty::New(true));

  // add to ds and remember as created
  this->GetDataStorage()->Add(_CurrentPointSetNode);
  m_CreatedPointSetNodes.push_back( _CurrentPointSetNode );

  // make new selection
  std::vector<mitk::DataNode*> selection;
  selection.push_back( _CurrentPointSetNode );
  this->FireNodesSelected( selection );
  this->OnSelectionChanged( selection );
}

void QmitkSimpleMeasurement::AddAngleSimpleMeasurement()
{
  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();

  mitk::DataNode::Pointer _CurrentPointSetNode = mitk::DataNode::New();
  _CurrentPointSetNode->SetData(pointSet);
  _CurrentPointSetNode->SetProperty("show contour", mitk::BoolProperty::New(true));
  _CurrentPointSetNode->SetProperty("name", mitk::StringProperty::New("angle"));
  _CurrentPointSetNode->SetProperty("show angles", mitk::BoolProperty::New(true));

  // add to ds and remember as created
  this->GetDataStorage()->Add(_CurrentPointSetNode);
  m_CreatedPointSetNodes.push_back( _CurrentPointSetNode );

  // make new selection
  std::vector<mitk::DataNode*> selection;
  selection.push_back( _CurrentPointSetNode );
  this->FireNodesSelected( selection );
  this->OnSelectionChanged( selection );
}

void QmitkSimpleMeasurement::AddPathSimpleMeasurement()
{
  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();

  mitk::DataNode::Pointer _CurrentPointSetNode = mitk::DataNode::New();
  _CurrentPointSetNode->SetData(pointSet);
  _CurrentPointSetNode->SetProperty("show contour", mitk::BoolProperty::New(true));
  _CurrentPointSetNode->SetProperty("name", mitk::StringProperty::New("path"));
  _CurrentPointSetNode->SetProperty("show distances", mitk::BoolProperty::New(true));
  _CurrentPointSetNode->SetProperty("show angles", mitk::BoolProperty::New(true));

  // add to ds and remember as created
  this->GetDataStorage()->Add(_CurrentPointSetNode);
  m_CreatedPointSetNodes.push_back( _CurrentPointSetNode );

  // make new selection
  std::vector<mitk::DataNode*> selection;
  selection.push_back( _CurrentPointSetNode );
  this->FireNodesSelected( selection );
  this->OnSelectionChanged( selection );
}

void QmitkSimpleMeasurement::CreateQtPartControl( QWidget* parent )
{
  m_Controls = new Ui::QmitkSimpleMeasurementControls;
  m_Controls->setupUi(parent);

  connect( (QObject*)(m_Controls->pbDistance), SIGNAL(clicked()),(QObject*) this, SLOT(AddDistanceSimpleMeasurement()) );
  connect( (QObject*)(m_Controls->pbAngle), SIGNAL(clicked()),(QObject*) this, SLOT(AddAngleSimpleMeasurement()) );
  connect( (QObject*)(m_Controls->pbPath), SIGNAL(clicked()),(QObject*) this, SLOT(AddPathSimpleMeasurement()) );
}

void QmitkSimpleMeasurement::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
  if(!this->IsActivated())
    return;

  mitk::DataNode* selectedNode = 0;
  if(nodes.size() > 0)
    selectedNode = nodes.front();

  mitk::PointSet* pointSet = 0;
  if(selectedNode)
    pointSet = dynamic_cast<mitk::PointSet*> ( selectedNode->GetData() );

  // something else was selected. remove old interactor
  if (m_PointSetInteractor.IsNotNull())
  {
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_PointSetInteractor);
  }

  bool pointsetCreatedByThis = false;

  // only go further if a pointset was selected
  if(pointSet)
  {
    // see if this pointset was created by us
    std::vector<mitk::DataNode*>::iterator it = std::find( m_CreatedPointSetNodes.begin()
      , m_CreatedPointSetNodes.end(), selectedNode);
    if(it != m_CreatedPointSetNodes.end())
     pointsetCreatedByThis = true;
  }

  // do nothing if it was not created by us or it is no pointset node
  if(pointsetCreatedByThis)
  {
    // otherwise: set text and add interactor for the pointset
    m_Controls->selectedPointSet->setText( QString::fromStdString(selectedNode->GetName()) );
    mitk::PointSetInteractor::Pointer newPointSetInteractor 
      = mitk::PointSetInteractor::New("pointsetinteractor", selectedNode);
    mitk::GlobalInteraction::GetInstance()->AddInteractor(newPointSetInteractor);
    m_PointSetInteractor = newPointSetInteractor;
    float green[] = { 0, 255, 0 };
    float red[] = { 255, 0, 0 };
    selectedNode->SetColor(green);
    if(m_SelectedPointSetNode.IsNotNull())
      m_SelectedPointSetNode->SetColor(red);
    m_SelectedPointSetNode = selectedNode;
  }
  else
  {
    // revert text
    m_Controls->selectedPointSet->setText( "None" );
  }
}

bool QmitkSimpleMeasurement::IsExclusiveFunctionality() const
{
  return true;
}

void QmitkSimpleMeasurement::NodeRemoved( const mitk::DataNode* node )
{
  // remove a node if it is destroyed from our created array
  std::vector<mitk::DataNode*>::iterator it = std::find( m_CreatedPointSetNodes.begin()
    , m_CreatedPointSetNodes.end(), node);
  if(it != m_CreatedPointSetNodes.end())
    m_CreatedPointSetNodes.erase(it);
}