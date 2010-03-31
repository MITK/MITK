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

QmitkPointSetInteractionView::QmitkPointSetInteractionView( QObject *parent )
: m_SelectionListener(0)
, m_Controls(0)
{

}

QmitkPointSetInteractionView::~QmitkPointSetInteractionView()
{
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()
    ->RemovePostSelectionListener(m_SelectionListener);

}

void QmitkPointSetInteractionView::CreateQtPartControl( QWidget *parent )
{
  m_Controls = new Ui::QmitkPointSetInteractionControls;
  m_Controls->setupUi(parent);
  m_Controls->m_PbAddPointSet->connect( m_Controls->m_PbAddPointSet, SIGNAL( clicked() )
    , this, SLOT( OnAddPointSetClicked() ) );

  m_SelectionListener = new berry::SelectionChangedAdapter<QmitkPointSetInteractionView>(this
    , &QmitkPointSetInteractionView::SelectionChanged);
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener(
    m_SelectionListener);
  
}

void QmitkPointSetInteractionView::Activated()
{
  m_Controls->m_PointListWidget->SetPointSetNode(m_SelectedPointSetNode.GetPointer());
}

void QmitkPointSetInteractionView::Deactivated()
{
  m_Controls->m_PointListWidget->DeactivateInteractor(true);
  m_Controls->m_PointListWidget->SetPointSetNode(NULL);
}

void QmitkPointSetInteractionView::OnAddPointSetClicked()
{
  //Ask for the name of the point set
  //Ask for the maximal number of points in the pointset
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
  // add the node to the data tree
  //
  this->GetDefaultDataStorage()->Add(pointSetNode);
  m_Controls->m_PointListWidget->SetPointSetNode(pointSetNode.GetPointer());
  m_Controls->m_CurrentPointSetLabel->setText(QString::fromStdString(pointSetNode->GetName()));

  // the point set is automatically shown in the
  // point list widget due to signal /slot connections
  // from the TreeNodeSelector/Activated signal to OnCurrentPointSetActivated
}

void QmitkPointSetInteractionView::SelectionChanged( berry::IWorkbenchPart::Pointer sourcepart
                                                    , berry::ISelection::ConstPointer selection )
{
  if ( sourcepart.GetPointer() == this)
    return;

  mitk::DataNodeSelection::ConstPointer _DataNodeSelection 
    = selection.Cast<const mitk::DataNodeSelection>();
  if (_DataNodeSelection.IsNull())
    return;

  mitk::DataNodeObject* _DataNodeObject = 0;
  mitk::PointSet* _PointSet;

  m_Controls->m_CurrentPointSetLabel->setText("...");

  for (mitk::DataNodeSelection::iterator it =
    _DataNodeSelection->Begin(); it != _DataNodeSelection->End(); ++it)
  {
    _PointSet = 0;

    _DataNodeObject
      = dynamic_cast<mitk::DataNodeObject*> ((*it).GetPointer());
    if (!_DataNodeObject)
      continue;

    m_SelectedPointSetNode = _DataNodeObject->GetDataNode();
    if (m_SelectedPointSetNode.IsNull())
      continue;

    _PointSet = dynamic_cast<mitk::PointSet *> (m_SelectedPointSetNode->GetData());
    // planar figure selected
    if (_PointSet)
    {
      m_Controls->m_PointListWidget->SetPointSetNode(m_SelectedPointSetNode.GetPointer());
      m_Controls->m_CurrentPointSetLabel->setText(QString::fromStdString(m_SelectedPointSetNode->GetName()));
    }
  }
}
