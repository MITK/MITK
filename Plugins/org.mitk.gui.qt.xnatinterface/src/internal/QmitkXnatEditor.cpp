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


// Qmitk
#include "QmitkXnatEditor.h"
#include "QmitkXnatObjectEditorInput.h"
#include "QmitkXnatTreeBrowserView.h"
#include "org_mitk_gui_qt_xnatinterface_Activator.h"

// Standard
#include <iostream>
#include <string>
#include <fstream>

// CTK XNAT Core
#include "ctkXnatLoginProfile.h"
#include "ctkXnatObject.h"
#include "ctkXnatProject.h"
#include "ctkXnatSubject.h"
#include "ctkXnatListModel.h"
#include "ctkXnatScanFolder.h"
#include "ctkXnatScanResource.h"

// CTK XNAT Widgets
#include "ctkXnatTreeModel.h"
#include "ctkXnatTreeItem.h"

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
#include <berryQModelIndexObject.h>
#include <berryIWorkbenchPage.h>
#include <berryIWorkbenchPart.h>

// Qt
#include <QScopedPointer>
#include <QSharedPointer>
#include <QMessageBox>
#include <QTreeView>
#include <QAbstractItemModel>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QString>
#include <QBrush>
#include <QListView>
#include <QRegExp>
#include <QModelIndex>

const std::string QmitkXnatEditor::EDITOR_ID = "org.mitk.editors.qmitkxnateditor";
const QString QmitkXnatEditor::DOWNLOAD_PATH = QString("C:/Users/knorr/Downloads/MITK_XNAT/");

QmitkXnatEditor::QmitkXnatEditor() :
  m_Session(0),
  m_ListModel(new ctkXnatListModel()),
  m_SelectionListener(new berry::SelectionChangedAdapter<QmitkXnatEditor>(this, &QmitkXnatEditor::SelectionChanged))
{

}

QmitkXnatEditor::~QmitkXnatEditor()
{
  delete m_ListModel;
  berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  s->RemoveSelectionListener(m_SelectionListener);
}

bool QmitkXnatEditor::IsDirty() const
{
  return false;
}

bool QmitkXnatEditor::IsSaveAsAllowed() const
{
  return false;
}

void QmitkXnatEditor::Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input)
{
  this->SetInput(input);
  this->SetSite(site);
}

void QmitkXnatEditor::DoSave()
{
}

void QmitkXnatEditor::DoSaveAs()
{
}

void QmitkXnatEditor::SetInput(berry::IEditorInput::Pointer input)
{
  QmitkXnatObjectEditorInput::Pointer oPtr = input.Cast<QmitkXnatObjectEditorInput>();
  if(oPtr.IsNotNull())
  {
    SetInputWithNotify(oPtr);
  }
  else
  {
    m_Session = mitk::org_mitk_gui_qt_xnatinterface_Activator::GetXnatConnectionManager()->GetXnatConnection();
    QmitkXnatObjectEditorInput::Pointer xoPtr = QmitkXnatObjectEditorInput::New( m_Session->dataModel() );
    berry::IEditorInput::Pointer editorInput( xoPtr );
    SetInputWithNotify(editorInput);
    if ( !this->GetEditorInput().Cast<QmitkXnatObjectEditorInput>()->GetXnatObject()->isFetched() )
    {
      this->GetEditorInput().Cast<QmitkXnatObjectEditorInput>()->GetXnatObject()->fetch();
    }
  }
}

const char* QmitkXnatEditor::GetClassNameA() const
{
  return QtEditorPart::GetClassNameA();
}

void QmitkXnatEditor::SetFocus()
{
}

void QmitkXnatEditor::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  m_Controls.treeView->setModel(m_ListModel);

  GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddSelectionListener(m_SelectionListener);

  connect( m_Controls.treeView, SIGNAL(activated(const QModelIndex&)), this, SLOT(OnObjectActivated(const QModelIndex&)) );
  //connect( m_Controls.buttonDownloadResource, SIGNAL(clicked()), this, SLOT(DownloadResource()) );
  //connect( m_Controls.buttonDownloadFile, SIGNAL(clicked()), this, SLOT(DownloadFile()) );
  connect( m_Controls.buttonDataModel, SIGNAL(clicked()), this, SLOT(OnDataModelButtonClicked()) );
  connect( m_Controls.buttonProject, SIGNAL(clicked()), this, SLOT(OnProjectButtonClicked()) );
  connect( m_Controls.buttonSubject, SIGNAL(clicked()), this, SLOT(OnSubjectButtonClicked()) );
  connect( m_Controls.buttonExperiment, SIGNAL(clicked()), this, SLOT(OnExperimentButtonClicked()) );
  connect( m_Controls.buttonKindOfData, SIGNAL(clicked()), this, SLOT(OnKindOfDataButtonClicked()) );
  connect( m_Controls.buttonSession, SIGNAL(clicked()), this, SLOT(OnSessionButtonClicked()) );
  connect( m_Controls.buttonResource, SIGNAL(clicked()), this, SLOT(OnResourceButtonClicked()) );

  for(int i = 0; i < m_Controls.breadcrumbHorizontalLayout->count()-1; i++)
  {
    QLayoutItem* child = m_Controls.breadcrumbHorizontalLayout->itemAt(i);
    child->widget()->setVisible(false);
  }
  for(int i = 0; i < m_Controls.breadcrumbDescriptionLayout->count()-1; i++)
  {
    QLayoutItem* child = m_Controls.breadcrumbDescriptionLayout->itemAt(i);
    child->widget()->setVisible(false);
  }
  UpdateList();
}

void QmitkXnatEditor::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/, const QList<mitk::DataNode::Pointer>& nodes )
{
}

/**
\brief Here the root object will be set and the view reset.
*/
void QmitkXnatEditor::UpdateList()
{
  ctkXnatObject* inputObject = GetEditorInput().Cast<QmitkXnatObjectEditorInput>()->GetXnatObject();
  if( inputObject == NULL )
    return;
  m_ListModel->setRootObject( inputObject );
  m_Controls.treeView->reset();

  // recursive method to check parents of the inputObject
  m_ParentCount = ParentChecker(inputObject);

  for(int i = 0; i < m_Controls.breadcrumbHorizontalLayout->count()-1; i++)
  {
    QLayoutItem* child = m_Controls.breadcrumbHorizontalLayout->itemAt(i);
    child->widget()->setVisible(false);
  }
  for(int i = 0; i < m_Controls.breadcrumbDescriptionLayout->count()-1; i++)
  {
    QLayoutItem* child = m_Controls.breadcrumbDescriptionLayout->itemAt(i);
    child->widget()->setVisible(false);
  }

  ctkXnatObject* parent = NULL;
  for(int i = m_ParentCount*2; i >= 0; i--)
  {
    m_Controls.breadcrumbDescriptionLayout->itemAt(i)->widget()->setVisible(true);
    QLayoutItem* child = m_Controls.breadcrumbHorizontalLayout->itemAt(i);
    child->widget()->setVisible(true);
    if(i>0)
    {
      m_Controls.breadcrumbHorizontalLayout->itemAt(i-1)->widget()->setVisible(true);
      m_Controls.breadcrumbDescriptionLayout->itemAt(i-1)->widget()->setVisible(true);
    }
    if(parent == NULL)
    {
      parent = inputObject;
    }
    QPushButton* breadcrumbButton = dynamic_cast<QPushButton*>(child->widget());
    breadcrumbButton->setText(parent->id());
    parent = parent->parent();
    i--;
  }
}

void QmitkXnatEditor::SelectionChanged(berry::IWorkbenchPart::Pointer sourcepart,
                               berry::ISelection::ConstPointer selection)
{
  // check for null selection
  if (selection.IsNull())
  {
    return;
  }
  // exclude own selection events and check whether this kind of selection can be handled
  if (sourcepart != this &&
      selection.Cast<const berry::IStructuredSelection>())
  {
    berry::IStructuredSelection::ConstPointer currentSelection = selection.Cast<const berry::IStructuredSelection>();
    // iterates over the selection
    for (berry::IStructuredSelection::iterator itr = currentSelection->Begin();
         itr != currentSelection->End(); ++itr)
    {
      if (berry::SmartPointer<berry::QModelIndexObject> objectPointer = itr->Cast<berry::QModelIndexObject>())
      {
        // get object of selected ListWidgetElement
        ctkXnatObject* object = objectPointer->GetQModelIndex().data(Qt::EditRole).value<ctkXnatObject*>();
        QmitkXnatObjectEditorInput::Pointer oPtr = QmitkXnatObjectEditorInput::New( object );
        berry::IEditorInput::Pointer editorInput( oPtr );
        if ( !(editorInput == this->GetEditorInput()) )
          SetInput(editorInput);

        UpdateList();
      }
    }
  }
}

void QmitkXnatEditor::ToHigherLevel()
{
  ctkXnatObject* parent = GetEditorInput().Cast<QmitkXnatObjectEditorInput>()->GetXnatObject()->parent();
  if( parent ==  NULL)
  {
    return;
  }
  QmitkXnatObjectEditorInput::Pointer oPtr = QmitkXnatObjectEditorInput::New( parent );
  berry::IEditorInput::Pointer editorInput( oPtr );
  SetInput(editorInput);
  UpdateList();
}

void QmitkXnatEditor::OnObjectActivated(const QModelIndex &index)
{
  if (!index.isValid()) return;

  ctkXnatObject* child = GetEditorInput().Cast<QmitkXnatObjectEditorInput>()->GetXnatObject()->children().takeAt(index.row());
  if( child ==  NULL)
  {
    return;
  }
  QmitkXnatObjectEditorInput::Pointer oPtr = QmitkXnatObjectEditorInput::New( child );
  berry::IEditorInput::Pointer editorInput( oPtr );
  SetInput(editorInput);

  if ( !this->GetEditorInput().Cast<QmitkXnatObjectEditorInput>()->GetXnatObject()->isFetched() )
  {
    this->GetEditorInput().Cast<QmitkXnatObjectEditorInput>()->GetXnatObject()->fetch();
  }
  UpdateList();
}

int QmitkXnatEditor::ParentChecker(ctkXnatObject* child)
{
  int sum;
  if( child->parent() == NULL )
  {
    return 0;
  }
  else
  {
    sum = 1 + ParentChecker(child->parent());
  }
  return sum;
}

void QmitkXnatEditor::OnDataModelButtonClicked()
{
  for(int i = m_ParentCount; i > 0; i--)
  {
    ToHigherLevel();
  }
}

void QmitkXnatEditor::OnProjectButtonClicked()
{
  for(int i = m_ParentCount-1; i > 0; i--)
  {
    ToHigherLevel();
  }
}

void QmitkXnatEditor::OnSubjectButtonClicked()
{
  for(int i = m_ParentCount-2; i > 0; i--)
  {
    ToHigherLevel();
  }
}

void QmitkXnatEditor::OnExperimentButtonClicked()
{
  for(int i = m_ParentCount-3; i > 0; i--)
  {
    ToHigherLevel();
  }
}

void QmitkXnatEditor::OnKindOfDataButtonClicked()
{
  for(int i = m_ParentCount-4; i > 0; i--)
  {
    ToHigherLevel();
  }
}

void QmitkXnatEditor::OnSessionButtonClicked()
{
  for(int i = m_ParentCount-5; i > 0; i--)
  {
    ToHigherLevel();
  }
}

void QmitkXnatEditor::OnResourceButtonClicked()
{
  for(int i = m_ParentCount-6; i > 0; i--)
  {
    ToHigherLevel();
  }
}
