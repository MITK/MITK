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


QmitkPointSetInteractionView::QmitkPointSetInteractionView( QObject* /*parent*/ )
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

void QmitkPointSetInteractionView::SetFocus()
{
  m_Controls->m_PbAddPointSet->setFocus();
}

void QmitkPointSetInteractionView::Activated()
{
  // emulate datamanager selection
  berry::IWorkbenchPart::Pointer nullPart;
  QList<mitk::DataNode::Pointer> selection = this->GetDataManagerSelection();
  this->OnSelectionChanged(nullPart, selection);
}

void QmitkPointSetInteractionView::Deactivated()
{
  // emulate empty selection
  berry::IWorkbenchPart::Pointer nullPart;
  QList<mitk::DataNode::Pointer> selection;
  this->OnSelectionChanged(nullPart, selection);
  m_Controls->m_PointListWidget->DeactivateInteractor(true);
}

void QmitkPointSetInteractionView::Visible()
{
}

void QmitkPointSetInteractionView::Hidden()
{
}

void QmitkPointSetInteractionView::OnAddPointSetClicked()
{
  //Ask for the name of the point set
  bool ok = false;
  QString name = QInputDialog::getText( QApplication::activeWindow()
    , tr("Add point set..."), tr("Enter name for the new point set"), QLineEdit::Normal, tr("PointSet"), &ok );
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
  this->GetDataStorage()->Add(pointSetNode);

  // make new selection and emulate selection for this
  berry::IWorkbenchPart::Pointer nullPart;
  QList<mitk::DataNode::Pointer> selection;
  selection.push_back(pointSetNode);
  this->FireNodesSelected(selection);
  this->OnSelectionChanged(nullPart, selection);
}

void QmitkPointSetInteractionView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/, const QList<mitk::DataNode::Pointer>& nodes)
{
  mitk::DataNode* selectedNode = 0;
  if(nodes.size() > 0)
    selectedNode = nodes.front();

  mitk::PointSet* pointSet = 0;
  if(selectedNode)
    pointSet = dynamic_cast<mitk::PointSet*> ( selectedNode->GetData() );

  if (pointSet)
  {
    m_SelectedPointSetNode = selectedNode;
    m_Controls->m_CurrentPointSetLabel->setText(QString::fromStdString(selectedNode->GetName()));
    m_Controls->m_PointListWidget->SetPointSetNode(selectedNode);
  }
  else
  {
    m_Controls->m_CurrentPointSetLabel->setText(tr("None"));
    m_Controls->m_PointListWidget->SetPointSetNode(nullptr);
    std::vector<mitk::DataNode*> emptyList;
    emptyList.push_back(nullptr);
    this->FireNodesSelected( emptyList );
  }

}

void QmitkPointSetInteractionView::NodeChanged( const mitk::DataNode* node )
{
  if(node == m_SelectedPointSetNode && m_Controls->m_CurrentPointSetLabel->text().toStdString() != node->GetName())
  {
    m_Controls->m_CurrentPointSetLabel->setText(QString::fromStdString(node->GetName()));
  }
}

void QmitkPointSetInteractionView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  if(m_Controls)
  {
    m_Controls->m_PointListWidget->AddSliceNavigationController(renderWindowPart->GetQmitkRenderWindow("axial")->GetSliceNavigationController());
    m_Controls->m_PointListWidget->AddSliceNavigationController(renderWindowPart->GetQmitkRenderWindow("sagittal")->GetSliceNavigationController());
    m_Controls->m_PointListWidget->AddSliceNavigationController(renderWindowPart->GetQmitkRenderWindow("coronal")->GetSliceNavigationController());
  }
}

void QmitkPointSetInteractionView::RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart)
{
  if(m_Controls)
  {
    m_Controls->m_PointListWidget->RemoveSliceNavigationController(renderWindowPart->GetQmitkRenderWindow("axial")->GetSliceNavigationController());
    m_Controls->m_PointListWidget->RemoveSliceNavigationController(renderWindowPart->GetQmitkRenderWindow("sagittal")->GetSliceNavigationController());
    m_Controls->m_PointListWidget->RemoveSliceNavigationController(renderWindowPart->GetQmitkRenderWindow("coronal")->GetSliceNavigationController());
  }
}
