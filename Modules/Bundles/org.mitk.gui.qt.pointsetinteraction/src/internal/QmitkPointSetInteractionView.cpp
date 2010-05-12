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

// Qmitk related includes
#include "QmitkPointSetInteractionView.h"
#include "ui_QmitkPointSetInteractionViewControls.h"

#include <berryIWorkbenchWindow.h>
#include <berryISelectionService.h>
#include <QInputDialog>
#include <QLineEdit>
#include <mitkProperties.h>
#include <QmitkPointListWidget.h>
#include <mitkDataNodeObject.h>
#include <mitkDataNodeSelection.h>
#include <mitkPointSetInteractor.h>
#include <mitkGlobalInteraction.h>

QmitkPointSetInteractionView::QmitkPointSetInteractionView( QObject *parent )
: m_Controls(0)
{

}

QmitkPointSetInteractionView::~QmitkPointSetInteractionView()
{
}

void QmitkPointSetInteractionView::CreateQtPartControl( QWidget *parent )
{
  m_Controls = new Ui::QmitkPointSetInteractionControls;
  m_Controls->setupUi(parent);
  m_Controls->m_PbAddPointSet->connect( m_Controls->m_PbAddPointSet, SIGNAL( clicked() )
    , this, SLOT( OnAddPointSetClicked() ) );  
}

void QmitkPointSetInteractionView::Activated()
{
  // emulate datamanager selection
  std::vector<mitk::DataNode*> selection = this->GetDataManagerSelection();
  this->OnSelectionChanged( selection );
}

void QmitkPointSetInteractionView::Deactivated()
{
  // emulate empty selection
  std::vector<mitk::DataNode*> selection;
  this->OnSelectionChanged( selection );
  m_Controls->m_PointListWidget->DeactivateInteractor(true);
}

void QmitkPointSetInteractionView::OnAddPointSetClicked()
{
  //Ask for the name of the point set
  bool ok = false;
  QString name = QInputDialog::getText( QApplication::activeWindow()
    , "Add point set...", "Enter name for the new point set", QLineEdit::Normal, "PointSet", &ok );
  if ( ! ok || name.isEmpty() )
    return;

  //
  //Create a new empty pointset
  //
  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
  //
  // Create a new data tree node 
  //
  mitk::DataNode::Pointer pointSetNode = mitk::DataNode::New();
  //
  // fill the data tree node with the appropriate information
  //
  pointSetNode->SetData( pointSet );
  pointSetNode->SetProperty( "name", mitk::StringProperty::New( name.toStdString() ) );
  pointSetNode->SetProperty( "opacity", mitk::FloatProperty::New( 1 ) );
  pointSetNode->SetColor( 1.0, 1.0, 0.0 );
  //
  // add the node to the ds
  //
  this->GetDefaultDataStorage()->Add(pointSetNode);

  // make new selection and emulate selection for this
  std::vector<mitk::DataNode*> selection;
  selection.push_back( pointSetNode );
  this->FireNodesSelected( selection );
  this->OnSelectionChanged( selection );
}

void QmitkPointSetInteractionView::OnSelectionChanged(std::vector<mitk::DataNode*> nodes)
{
  mitk::DataNode* selectedNode = 0;
  if(nodes.size() > 0)
    selectedNode = nodes.front();

  mitk::PointSet* pointSet = 0;
  if(selectedNode)
    pointSet = dynamic_cast<mitk::PointSet*> ( selectedNode->GetData() );

  //if( m_SelectedPointSetInteractor.IsNotNull() )
  //{
  //  mitk::GlobalInteraction::GetInstance()->RemoveInteractor( m_SelectedPointSetInteractor );
  //  m_SelectedPointSetInteractor = NULL;
  //}

  if (pointSet && this->IsActivated())
  {
    m_SelectedPointSetNode = selectedNode;
    m_Controls->m_CurrentPointSetLabel->setText(QString::fromStdString(selectedNode->GetName()));
    m_Controls->m_PointListWidget->SetPointSetNode(selectedNode);
    // add interactor
    //mitk::PointSetInteractor::Pointer _Interactor = mitk::PointSetInteractor::New("pointsetinteractor", selectedNode);
    //mitk::GlobalInteraction::GetInstance()->AddInteractor( _Interactor );
    //m_SelectedPointSetInteractor = _Interactor;
  }
  else
  {
    m_Controls->m_CurrentPointSetLabel->setText("None");
    m_Controls->m_PointListWidget->SetPointSetNode(0);
  }

}

bool QmitkPointSetInteractionView::IsExclusiveFunctionality() const
{
  return true;
}

void QmitkPointSetInteractionView::NodeChanged( const mitk::DataNode* node )
{
  if(node == m_SelectedPointSetNode && m_Controls->m_CurrentPointSetLabel->text().toStdString() != node->GetName())
  {
    m_Controls->m_CurrentPointSetLabel->setText(QString::fromStdString(node->GetName()));
  }
}

void QmitkPointSetInteractionView::StdMultiWidgetAvailable( QmitkStdMultiWidget& stdMultiWidget )
{
  if(m_Controls)
    m_Controls->m_PointListWidget->SetMultiWidget( &stdMultiWidget );
}

void QmitkPointSetInteractionView::StdMultiWidgetClosed( QmitkStdMultiWidget& stdMultiWidget )
{
  if(m_Controls)
    m_Controls->m_PointListWidget->SetMultiWidget( 0 );
}