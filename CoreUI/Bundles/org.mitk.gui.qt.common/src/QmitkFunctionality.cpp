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
#include "mitkLogMacros.h"

// mitk Includes
#include "mitkMessage.h"
#include "mitkIDataStorageService.h"
#include "mitkDataStorageEditorInput.h"
#include <mitkDataNodeObject.h>

// berry Includes
#include <berryPlatform.h>
#include <berryIWorkbenchPage.h>

// Qmitk Includes
#include <QmitkStdMultiWidgetEditor.h>

// Qt Includes
#include <QMessageBox>
#include <QScrollArea>
#include <QVBoxLayout>

QmitkFunctionality::QmitkFunctionality()
 : m_Parent(0)
 , m_Active(false)
 , m_Visible(false)
 , m_HandlesMultipleDataStorages(false)
 , m_InDataStorageChanged(false)
 , m_SelectionProvider(0)
{
  m_PreferencesService = 
    berry::Platform::GetServiceRegistry().GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);
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
    berry::Platform::GetServiceRegistry().GetServiceById<mitk::IDataStorageService>(mitk::IDataStorageService::ID);
  
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
    berry::Platform::GetServiceRegistry().GetServiceById<mitk::IDataStorageService>(mitk::IDataStorageService::ID);

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
  // REGISTER DATASTORAGE LISTENER
  this->GetDefaultDataStorage()->AddNodeEvent.AddListener( mitk::MessageDelegate1<QmitkFunctionality, const mitk::DataNode*>
    ( this, &QmitkFunctionality::NodeAddedProxy ) );
  this->GetDefaultDataStorage()->ChangedNodeEvent.AddListener( mitk::MessageDelegate1<QmitkFunctionality, const mitk::DataNode*>
    ( this, &QmitkFunctionality::NodeChangedProxy ) );
  this->GetDefaultDataStorage()->RemoveNodeEvent.AddListener( mitk::MessageDelegate1<QmitkFunctionality, const mitk::DataNode*>
    ( this, &QmitkFunctionality::NodeRemovedProxy ) );

  // REGISTER PREFERENCES LISTENER
  berry::IBerryPreferences::Pointer prefs = this->GetPreferences().Cast<berry::IBerryPreferences>();
  if(prefs.IsNotNull())
    prefs->OnChanged.AddListener(berry::MessageDelegate1<QmitkFunctionality
    , const berry::IBerryPreferences*>(this, &QmitkFunctionality::OnPreferencesChanged));

  // REGISTER FOR WORKBENCH SELECTION EVENTS
  m_BlueBerrySelectionListener = berry::ISelectionListener::Pointer(new berry::SelectionChangedAdapter<QmitkFunctionality>(this
    , &QmitkFunctionality::BlueBerrySelectionChanged));
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener(/*"org.mitk.views.datamanager",*/ m_BlueBerrySelectionListener);

  // REGISTER A SELECTION PROVIDER
  QmitkFunctionality::SelectionProvider::Pointer _SelectionProvider
    = QmitkFunctionality::SelectionProvider::New(this);
  m_SelectionProvider = _SelectionProvider.GetPointer();
  this->GetSite()->SetSelectionProvider(berry::ISelectionProvider::Pointer(m_SelectionProvider));
  
  // EMULATE INITIAL SELECTION EVENTS

  // by default a a multi widget is always available
  this->StdMultiWidgetAvailable(*this->GetActiveStdMultiWidget());

  // send datamanager selection
  this->OnSelectionChanged(this->GetDataManagerSelection());

  // send preferences changed event
  this->OnPreferencesChanged(this->GetPreferences().Cast<berry::IBerryPreferences>().GetPointer());
}

void QmitkFunctionality::ClosePart()
{

}

void QmitkFunctionality::ClosePartProxy()
{
  this->GetDefaultDataStorage()->AddNodeEvent.RemoveListener( mitk::MessageDelegate1<QmitkFunctionality, const mitk::DataNode*>
    ( this, &QmitkFunctionality::NodeAddedProxy ) );
  this->GetDefaultDataStorage()->RemoveNodeEvent.RemoveListener( mitk::MessageDelegate1<QmitkFunctionality, const mitk::DataNode*>
    ( this, &QmitkFunctionality::NodeRemovedProxy) );
  this->GetDefaultDataStorage()->ChangedNodeEvent.RemoveListener( mitk::MessageDelegate1<QmitkFunctionality, const mitk::DataNode*>
    ( this, &QmitkFunctionality::NodeChangedProxy ) );

  berry::IBerryPreferences::Pointer prefs = this->GetPreferences().Cast<berry::IBerryPreferences>();
  if(prefs.IsNotNull())
  {
    prefs->OnChanged.RemoveListener(berry::MessageDelegate1<QmitkFunctionality
    , const berry::IBerryPreferences*>(this, &QmitkFunctionality::OnPreferencesChanged));
    // flush the preferences here (disabled, everyone should flush them by themselves at the right moment)
    // prefs->Flush();
  }

  // REMOVE SELECTION PROVIDER
  this->GetSite()->SetSelectionProvider(berry::ISelectionProvider::Pointer(NULL));

  berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  if(s)
  {
    s->RemovePostSelectionListener(m_BlueBerrySelectionListener);
  }

    this->ClosePart();
}

QmitkFunctionality::~QmitkFunctionality()
{
}

std::vector<mitk::DataNode*> QmitkFunctionality::GetCurrentSelection() const
{
  berry::ISelection::ConstPointer selection( this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection());
  // buffer for the data manager selection
  mitk::DataNodeSelection::ConstPointer currentSelection = selection.Cast<const mitk::DataNodeSelection>();
  return this->DataNodeSelectionToVector(currentSelection);
}

std::vector<mitk::DataNode*> QmitkFunctionality::GetDataManagerSelection() const
{
  berry::ISelection::ConstPointer selection( this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
    // buffer for the data manager selection
  mitk::DataNodeSelection::ConstPointer currentSelection = selection.Cast<const mitk::DataNodeSelection>();
  return this->DataNodeSelectionToVector(currentSelection);
}

void QmitkFunctionality::OnPreferencesChanged( const berry::IBerryPreferences* )
{
}

void QmitkFunctionality::OnSelectionChanged(std::vector<mitk::DataNode*> nodes)
{
}

void QmitkFunctionality::BlueBerrySelectionChanged(berry::IWorkbenchPart::Pointer sourcepart, berry::ISelection::ConstPointer selection)
{
  if(sourcepart.IsNull() || sourcepart->GetSite()->GetId() != "org.mitk.views.datamanager")
    return;

  mitk::DataNodeSelection::ConstPointer _DataNodeSelection 
    = selection.Cast<const mitk::DataNodeSelection>();
  this->OnSelectionChanged(this->DataNodeSelectionToVector(_DataNodeSelection));
}


std::vector<mitk::DataNode*> QmitkFunctionality::DataNodeSelectionToVector(mitk::DataNodeSelection::ConstPointer currentSelection) const
{

  std::vector<mitk::DataNode*> selectedNodes;
  if(currentSelection.IsNull())
    return selectedNodes;

  mitk::DataNodeObject* _DataNodeObject = 0;
  mitk::DataNode* _DataNode = 0;

  for(mitk::DataNodeSelection::iterator it = currentSelection->Begin();
    it != currentSelection->End(); ++it)
  {
    _DataNodeObject = dynamic_cast<mitk::DataNodeObject*>((*it).GetPointer());
    if(_DataNodeObject)
    {
      _DataNode = _DataNodeObject->GetDataNode();
      if(_DataNode)
        selectedNodes.push_back(_DataNode);
    }
  }

  return selectedNodes;
}

bool QmitkFunctionality::IsVisible() const
{
  return m_Visible;
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

void QmitkFunctionality::NodeAddedProxy( const mitk::DataNode* node )
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

void QmitkFunctionality::NodeAdded( const mitk::DataNode*  /*node*/ )
{

}

void QmitkFunctionality::NodeRemovedProxy( const mitk::DataNode* node )
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

void QmitkFunctionality::NodeRemoved( const mitk::DataNode*  /*node*/ )
{

}

void QmitkFunctionality::DataStorageChanged()
{

}

QmitkStdMultiWidget* QmitkFunctionality::GetActiveStdMultiWidget()
{
  QmitkStdMultiWidget* activeStdMultiWidget = 0;
  berry::IEditorPart::Pointer editor =
    this->GetSite()->GetPage()->GetActiveEditor();

  if (editor.Cast<QmitkStdMultiWidgetEditor>().IsNotNull())
  {
    activeStdMultiWidget = editor.Cast<QmitkStdMultiWidgetEditor>()->GetStdMultiWidget();
  }
  else
  {
    mitk::DataStorageEditorInput::Pointer editorInput;
    editorInput = new mitk::DataStorageEditorInput();
    // open a new multi-widget editor, but do not give it the focus
    berry::IEditorPart::Pointer editor = this->GetSite()->GetPage()->OpenEditor(editorInput, QmitkStdMultiWidgetEditor::EDITOR_ID, false);
    activeStdMultiWidget = editor.Cast<QmitkStdMultiWidgetEditor>()->GetStdMultiWidget();
  }

  return activeStdMultiWidget;
}

void QmitkFunctionality::HandleException( const char* str, QWidget* parent, bool showDialog ) const
{
  //itkGenericOutputMacro( << "Exception caught: " << str );
  MITK_ERROR << str;
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

berry::IPreferences::Pointer QmitkFunctionality::GetPreferences() const
{
  berry::IPreferencesService::Pointer prefService = m_PreferencesService.Lock();
  // const_cast workaround for bad programming: const uncorrectness this->GetViewSite() should be const
  std::string id = "/" + (const_cast<QmitkFunctionality*>(this))->GetViewSite()->GetId();
  return prefService.IsNotNull() ? prefService->GetSystemPreferences()->Node(id): berry::IPreferences::Pointer(0);
}

void QmitkFunctionality::Visible()
{

}

void QmitkFunctionality::Hidden()
{

}

void QmitkFunctionality::NodeChanged( const mitk::DataNode* node )
{

}

void QmitkFunctionality::NodeChangedProxy( const mitk::DataNode* node )
{
  // garantuee no recursions when a new node event is thrown in NodeAdded()
  if(!m_InDataStorageChanged)
  {
    m_InDataStorageChanged = true;
    this->NodeChanged(node);
    this->DataStorageChanged();
    m_InDataStorageChanged = false;
  }
}

bool QmitkFunctionality::IsExclusiveFunctionality() const
{
  return true;
}

void QmitkFunctionality::SetVisible( bool visible )
{
  m_Visible = visible;
}

void QmitkFunctionality::SetActivated( bool activated )
{
  m_Active = activated;
}

bool QmitkFunctionality::IsActivated() const
{
  return m_Active;
}

QmitkFunctionality::SelectionProvider::SelectionProvider( QmitkFunctionality* _Functionality )
: m_Functionality(_Functionality)
{

}

QmitkFunctionality::SelectionProvider::~SelectionProvider()
{
  m_Functionality = 0;
}

void QmitkFunctionality::SelectionProvider::AddSelectionChangedListener( berry::ISelectionChangedListener::Pointer listener )
{
  m_SelectionEvents.AddListener(listener);
}


berry::ISelection::ConstPointer QmitkFunctionality::SelectionProvider::GetSelection() const
{
  return m_CurrentSelection;
}

void QmitkFunctionality::SelectionProvider::RemoveSelectionChangedListener( berry::ISelectionChangedListener::Pointer listener )
{
  m_SelectionEvents.RemoveListener(listener);
}

void QmitkFunctionality::SelectionProvider::SetSelection( berry::ISelection::Pointer selection )
{
  m_CurrentSelection = selection.Cast<mitk::DataNodeSelection>();
}

void QmitkFunctionality::SelectionProvider::FireNodesSelected( std::vector<mitk::DataNode::Pointer> nodes )
{
  mitk::DataNodeSelection::Pointer sel(new mitk::DataNodeSelection(nodes));
  m_CurrentSelection = sel;
  berry::SelectionChangedEvent::Pointer event(new berry::SelectionChangedEvent(berry::ISelectionProvider::Pointer(this)
    , m_CurrentSelection));
  m_SelectionEvents.selectionChanged(event);

}

void QmitkFunctionality::FireNodeSelected( mitk::DataNode::Pointer node )
{
  std::vector<mitk::DataNode::Pointer> nodes;
  nodes.push_back(node);
  this->FireNodesSelected(nodes);
}

void QmitkFunctionality::FireNodesSelected( std::vector<mitk::DataNode::Pointer> nodes )
{
  if( !m_SelectionProvider )
    return;
  m_SelectionProvider->FireNodesSelected(nodes);

}