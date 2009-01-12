/*=========================================================================

 Program:   openCherry Platform
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

#include "QmitkViewPart.h"

#include <cherryPlatform.h>
#include <cherryIWorkbenchPage.h>
#include <mitkIDataStorageService.h>
#include <QmitkStdMultiWidgetEditor.h>
#include "mitkMessage.h"

QmitkViewPart::QmitkViewPart()
 : m_Parent(0)
 , m_HandlesMultipleDataStorages(false)
{
}
  
void QmitkViewPart::SetHandleMultipleDataStorages(bool multiple)
{
  m_HandlesMultipleDataStorages = multiple;
}

bool QmitkViewPart::HandlesMultipleDataStorages() const
{
  return m_HandlesMultipleDataStorages;
}
  
mitk::DataStorage::Pointer 
QmitkViewPart::GetDataStorage() const
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


mitk::DataStorage::Pointer QmitkViewPart::GetDefaultDataStorage() const
{
  mitk::IDataStorageService::Pointer service = 
    cherry::Platform::GetServiceRegistry().GetServiceById<mitk::IDataStorageService>(mitk::IDataStorageService::ID);

  return service->GetDefaultDataStorage()->GetDataStorage();
}

void QmitkViewPart::BeforeCreateQtPartControl(QWidget* parent)
{
  m_Parent = parent;
}

void QmitkViewPart::AfterCreateQtPartControl(QWidget* parent)
{
  this->GetSite()->GetPage()->AddPartListener(cherry::IPartListener::Pointer(this));
  this->GetDefaultDataStorage()->AddNodeEvent.AddListener( mitk::MessageDelegate1<QmitkViewPart, const mitk::DataTreeNode*>
    ( this, &QmitkViewPart::NodeAddedProxy ) );
  this->GetDefaultDataStorage()->RemoveNodeEvent.AddListener( mitk::MessageDelegate1<QmitkViewPart, const mitk::DataTreeNode*>
    ( this, &QmitkViewPart::NodeRemovedProxy) );

  // if a multiwidget is yet available call activated on this view part otherwise call deactivated
  if(this->GetActiveStdMultiWidget() != 0)
    this->StdMultiWidgetAvailable();
  else
    this->StdMultiWidgetNotAvailable();
}

void QmitkViewPart::SetFocus()
{
}

void QmitkViewPart::Activated()
{
}

void QmitkViewPart::Deactivated()
{
}

void QmitkViewPart::StdMultiWidgetAvailable()
{
}
void QmitkViewPart::StdMultiWidgetNotAvailable()
{
}

void QmitkViewPart::PartActivated( cherry::IWorkbenchPartReference::Pointer partRef )
{
  if(partRef->GetPart(false) == this)
    this->Activated();
}

void QmitkViewPart::PartBroughtToTop( cherry::IWorkbenchPartReference::Pointer partRef )
{

}

void QmitkViewPart::PartClosed( cherry::IWorkbenchPartReference::Pointer partRef )
{
  // when the last stdmutliwidget got closed disable view
  if ((partRef->GetPart(false).Cast<QmitkStdMultiWidgetEditor>()).IsNotNull()
    && this->GetActiveStdMultiWidget() == 0)
  {
    this->StdMultiWidgetNotAvailable();
  }
}

void QmitkViewPart::PartDeactivated( cherry::IWorkbenchPartReference::Pointer partRef )
{
  if(partRef->GetPart(false) == this)
    this->Deactivated();
}

void QmitkViewPart::PartOpened( cherry::IWorkbenchPartReference::Pointer partRef )
{
  // inform ViewPart that multi widget is available now
  if ((partRef->GetPart(false).Cast<QmitkStdMultiWidgetEditor>()).IsNotNull())
  {
    this->StdMultiWidgetAvailable();
  }
}

void QmitkViewPart::PartHidden( cherry::IWorkbenchPartReference::Pointer partRef )
{
}

void QmitkViewPart::PartVisible( cherry::IWorkbenchPartReference::Pointer partRef )
{
}

void QmitkViewPart::PartInputChanged( cherry::IWorkbenchPartReference::Pointer partRef )
{
}

void QmitkViewPart::NodeAddedProxy( const mitk::DataTreeNode* node )
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

void QmitkViewPart::NodeAdded( const mitk::DataTreeNode* node )
{

}

void QmitkViewPart::NodeRemovedProxy( const mitk::DataTreeNode* node )
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

void QmitkViewPart::NodeRemoved( const mitk::DataTreeNode* node )
{

}

void QmitkViewPart::DataStorageChanged()
{

}

QmitkViewPart::~QmitkViewPart()
{
  this->GetDefaultDataStorage()->AddNodeEvent.RemoveListener( mitk::MessageDelegate1<QmitkViewPart, const mitk::DataTreeNode*>
    ( this, &QmitkViewPart::NodeAddedProxy ) );
  this->GetDefaultDataStorage()->RemoveNodeEvent.RemoveListener( mitk::MessageDelegate1<QmitkViewPart, const mitk::DataTreeNode*>
    ( this, &QmitkViewPart::NodeRemovedProxy) );
}


QmitkStdMultiWidget* QmitkViewPart::GetActiveStdMultiWidget()
{
  QmitkStdMultiWidget* activeStdMultiWidget = 0;
  cherry::IEditorPart::Pointer editor =
    this->GetSite()->GetPage()->GetActiveEditor();

  if (editor.Cast<QmitkStdMultiWidgetEditor>().IsNotNull())
  {
    activeStdMultiWidget = editor.Cast<QmitkStdMultiWidgetEditor>()->GetStdMultiWidget();
  }

  return activeStdMultiWidget;
}