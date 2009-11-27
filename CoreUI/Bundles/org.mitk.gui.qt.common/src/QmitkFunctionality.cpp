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

#include "QmitkFunctionality.h"

// other includes
#include "mbilog.h"

// mitk Includes
#include "mitkMessage.h"
#include "mitkIDataStorageService.h"
#include "mitkDataStorageEditorInput.h"

// cherry Includes
#include <cherryPlatform.h>
#include <cherryIWorkbenchPage.h>

// Qmitk Includes
#include <QmitkStdMultiWidgetEditor.h>

// Qt Includes
#include <QMessageBox>
#include <QScrollArea>
#include <QVBoxLayout>

QmitkFunctionality* QmitkFunctionality::m_DeactivatedFunctionality = 0;

QmitkFunctionality::QmitkFunctionality()
 : m_Parent(0)
 , m_HandlesMultipleDataStorages(false)
 , m_InDataStorageChanged(false)
 , m_IsActivated(false)
 , m_IsVisible(false)
{
  m_PreferencesService = 
    cherry::Platform::GetServiceRegistry().GetServiceById<cherry::IPreferencesService>(cherry::IPreferencesService::ID);
}
  
void QmitkFunctionality::SetHandleMultipleDataStorages(bool multiple)
{
  m_HandlesMultipleDataStorages = multiple;
}

bool QmitkFunctionality::HandlesMultipleDataStorages() const
{
  return m_HandlesMultipleDataStorages;
}
  
mitk::DataStorage::Pointer 
QmitkFunctionality::GetDataStorage() const
{
  mitk::IDataStorageService::Pointer service = 
    cherry::Platform::GetServiceRegistry().GetServiceById<mitk::IDataStorageService>(mitk::IDataStorageService::ID);
  
  if (service.IsNotNull())
  {
    if (m_HandlesMultipleDataStorages)
      return service->GetActiveDataStorage()->GetDataStorage();
    else
      return service->GetDefaultDataStorage()->GetDataStorage();
  }
  
  return 0;
}


mitk::DataStorage::Pointer QmitkFunctionality::GetDefaultDataStorage() const
{
  mitk::IDataStorageService::Pointer service = 
    cherry::Platform::GetServiceRegistry().GetServiceById<mitk::IDataStorageService>(mitk::IDataStorageService::ID);

  return service->GetDefaultDataStorage()->GetDataStorage();
}

void QmitkFunctionality::CreatePartControl(void* parent)
{

  // scrollArea
  QScrollArea* scrollArea = new QScrollArea;  
  //QVBoxLayout* scrollAreaLayout = new QVBoxLayout(scrollArea);
  scrollArea->setFrameShadow(QFrame::Plain);
  scrollArea->setFrameShape(QFrame::NoFrame);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  // m_Parent
  m_Parent = new QWidget;
  //m_Parent->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
  this->CreateQtPartControl(m_Parent);

  //scrollAreaLayout->addWidget(m_Parent);
  //scrollArea->setLayout(scrollAreaLayout);

  // set the widget now
  scrollArea->setWidgetResizable(true);
  scrollArea->setWidget(m_Parent);

  // add the scroll area to the real parent (the view tabbar)
  QWidget* parentQWidget = static_cast<QWidget*>(parent);
  QVBoxLayout* parentLayout = new QVBoxLayout(parentQWidget);
  parentLayout->setMargin(0);
  parentLayout->setSpacing(0);
  parentLayout->addWidget(scrollArea);

  // finally set the layout containing the scroll area to the parent widget (= show it)
  parentQWidget->setLayout(parentLayout);

  this->AfterCreateQtPartControl();
}

void QmitkFunctionality::AfterCreateQtPartControl()
{
  this->GetSite()->GetPage()->AddPartListener(cherry::IPartListener::Pointer(this));
  this->GetDefaultDataStorage()->AddNodeEvent.AddListener( mitk::MessageDelegate1<QmitkFunctionality, const mitk::DataTreeNode*>
    ( this, &QmitkFunctionality::NodeAddedProxy ) );
  this->GetDefaultDataStorage()->RemoveNodeEvent.AddListener( mitk::MessageDelegate1<QmitkFunctionality, const mitk::DataTreeNode*>
    ( this, &QmitkFunctionality::NodeRemovedProxy) );

  // if a multiwidget is yet available call activated on this view part otherwise call deactivated
  QmitkStdMultiWidget* stdMultiWidget = this->GetActiveStdMultiWidget();
  if(stdMultiWidget != 0)
    this->StdMultiWidgetAvailable(*stdMultiWidget);
  else
    this->StdMultiWidgetNotAvailable();
}


bool QmitkFunctionality::IsVisible() const
{
  return m_IsVisible;
}

void QmitkFunctionality::SetFocus()
{
}

void QmitkFunctionality::Activated()
{
}

void QmitkFunctionality::Deactivated()
{
}

void QmitkFunctionality::StdMultiWidgetAvailable( QmitkStdMultiWidget&  /*stdMultiWidget*/ )
{
}
void QmitkFunctionality::StdMultiWidgetNotAvailable()
{
}

cherry::IPartListener::Events::Types QmitkFunctionality::GetPartEventTypes() const
{
  return Events::ACTIVATED | Events::DEACTIVATED | Events::CLOSED | Events::HIDDEN | Events::VISIBLE;
}

void QmitkFunctionality::PartActivated( cherry::IWorkbenchPartReference::Pointer partRef )
{
  if (partRef->GetPart(false) == this && m_DeactivatedFunctionality != this)
  {
    if (m_DeactivatedFunctionality)
    {
      m_DeactivatedFunctionality->m_IsActivated = false;
      m_DeactivatedFunctionality->Deactivated();
    }
    m_DeactivatedFunctionality = 0;
    m_IsActivated = true;
    this->Activated();
  }
  else
  {
    if (partRef->GetId() == QmitkStdMultiWidgetEditor::EDITOR_ID && m_IsActivated == true)
    {
      this->StdMultiWidgetAvailable(*(partRef->GetPart(false).Cast<QmitkStdMultiWidgetEditor>()->GetStdMultiWidget()));
    }
  }
}

void QmitkFunctionality::PartDeactivated(cherry::IWorkbenchPartReference::Pointer partRef)
{
  if (partRef->GetPart(false) == this)
  {
    m_DeactivatedFunctionality = partRef->GetPart(false).Cast<QmitkFunctionality>().GetPointer();
  }
}

void QmitkFunctionality::PartClosed( cherry::IWorkbenchPartReference::Pointer partRef )
{
  if (partRef->GetId() == QmitkStdMultiWidgetEditor::EDITOR_ID)
  {
    QmitkStdMultiWidgetEditor::Pointer stdMultiWidgetEditor = partRef->GetPart(false).Cast<QmitkStdMultiWidgetEditor>();

    this->StdMultiWidgetClosed(*(stdMultiWidgetEditor->GetStdMultiWidget()));
    // if no other multi widget is available inform plugins bout that
    if(this->GetActiveStdMultiWidget() == 0)
      this->StdMultiWidgetNotAvailable();
  }
}

void QmitkFunctionality::PartHidden( cherry::IWorkbenchPartReference::Pointer partRef )
{
  if(partRef->GetPart(false).Cast<QmitkFunctionality>())
	  m_VisibleFunctionalities.erase(partRef->GetId());

  if(partRef->GetPart(false) == this)
  {
    m_IsVisible = false;
    this->Hidden();
  }
  else
    this->ActivateLastVisibleFunctionality();
}

void QmitkFunctionality::PartVisible( cherry::IWorkbenchPartReference::Pointer  partRef )
{
  if(partRef->GetPart(false).Cast<QmitkFunctionality>())
    m_VisibleFunctionalities.insert(partRef->GetId());

  if(partRef->GetPart(false) == this)
  {
    m_IsVisible = true;
    this->Visible();
    this->ActivateLastVisibleFunctionality();
  }
}

void QmitkFunctionality::NodeAddedProxy( const mitk::DataTreeNode* node )
{
  // garantuee no recursions when a new node event is thrown in NodeAdded()
  if(!m_InDataStorageChanged)
  {
    m_InDataStorageChanged = true;
    this->NodeAdded(node);
    this->DataStorageChanged();
    m_InDataStorageChanged = false;
  }

}

void QmitkFunctionality::NodeAdded( const mitk::DataTreeNode*  /*node*/ )
{

}

void QmitkFunctionality::NodeRemovedProxy( const mitk::DataTreeNode* node )
{
  // garantuee no recursions when a new node event is thrown in NodeAdded()
  if(!m_InDataStorageChanged)
  {
    m_InDataStorageChanged = true;
    this->NodeRemoved(node);
    this->DataStorageChanged();
    m_InDataStorageChanged = false;
  }
}

void QmitkFunctionality::NodeRemoved( const mitk::DataTreeNode*  /*node*/ )
{

}

void QmitkFunctionality::DataStorageChanged()
{

}

QmitkFunctionality::~QmitkFunctionality()
{
  this->Register();
  this->GetSite()->GetPage()->RemovePartListener(cherry::IPartListener::Pointer(this));
  this->UnRegister(false);

  this->GetDefaultDataStorage()->AddNodeEvent.RemoveListener( mitk::MessageDelegate1<QmitkFunctionality, const mitk::DataTreeNode*>
    ( this, &QmitkFunctionality::NodeAddedProxy ) );
  this->GetDefaultDataStorage()->RemoveNodeEvent.RemoveListener( mitk::MessageDelegate1<QmitkFunctionality, const mitk::DataTreeNode*>
    ( this, &QmitkFunctionality::NodeRemovedProxy) );
}


QmitkStdMultiWidget* QmitkFunctionality::GetActiveStdMultiWidget()
{
  QmitkStdMultiWidget* activeStdMultiWidget = 0;
  cherry::IEditorPart::Pointer editor =
    this->GetSite()->GetPage()->GetActiveEditor();

  if (editor.Cast<QmitkStdMultiWidgetEditor>().IsNotNull())
  {
    activeStdMultiWidget = editor.Cast<QmitkStdMultiWidgetEditor>()->GetStdMultiWidget();
  }
  else
  {
    mitk::DataStorageEditorInput::Pointer editorInput;
    editorInput = new mitk::DataStorageEditorInput();
    cherry::IEditorPart::Pointer editor = this->GetSite()->GetPage()->OpenEditor(editorInput, QmitkStdMultiWidgetEditor::EDITOR_ID);
    activeStdMultiWidget = editor.Cast<QmitkStdMultiWidgetEditor>()->GetStdMultiWidget();
  }

  return activeStdMultiWidget;
}

void QmitkFunctionality::HandleException( const char* str, QWidget* parent, bool showDialog ) const
{
  //itkGenericOutputMacro( << "Exception caught: " << str );
  LOG_ERROR << str;
  if ( showDialog )
  {
    QMessageBox::critical ( parent, "Exception caught!", str );
  }
}


void QmitkFunctionality::HandleException( std::exception& e, QWidget* parent, bool showDialog ) const
{
  HandleException( e.what(), parent, showDialog );
}

void QmitkFunctionality::StdMultiWidgetClosed( QmitkStdMultiWidget&  /*stdMultiWidget*/ )
{
  
}

void QmitkFunctionality::WaitCursorOn()
{
  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
}

void QmitkFunctionality::BusyCursorOn()
{
  QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
}

void QmitkFunctionality::WaitCursorOff()
{
  this->RestoreOverrideCursor();
}

void QmitkFunctionality::BusyCursorOff()
{
  this->RestoreOverrideCursor();
}

void QmitkFunctionality::RestoreOverrideCursor()
{
  QApplication::restoreOverrideCursor();
}

cherry::IPreferences::Pointer QmitkFunctionality::GetPreferences() const
{
  cherry::IPreferencesService::Pointer prefService = m_PreferencesService.Lock();
  // const_cast workaround for bad programming: const uncorrectness this->GetViewSite() should be const
  std::string id = "/" + (const_cast<QmitkFunctionality*>(this))->GetViewSite()->GetId();
  return prefService.IsNotNull() ? prefService->GetSystemPreferences()->Node(id): cherry::IPreferences::Pointer(0);
}

void QmitkFunctionality::Visible()
{

}

void QmitkFunctionality::Hidden()
{

}

void QmitkFunctionality::ActivateLastVisibleFunctionality()
{
  if(m_VisibleFunctionalities.size() == 1)
  {    
    this->GetSite()->GetPage()->Activate(this->GetSite()->GetPage()->FindView((*m_VisibleFunctionalities.begin())));
  }
}