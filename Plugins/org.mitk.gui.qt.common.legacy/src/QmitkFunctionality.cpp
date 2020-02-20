/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkFunctionality.h"
#include "internal/QmitkFunctionalityUtil.h"
#include "internal/QmitkCommonLegacyActivator.h"

// other includes
#include <mitkLogMacros.h>

// mitk Includes
#include <mitkIDataStorageService.h>
#include <mitkDataStorageEditorInput.h>

// berry Includes
#include <berryIWorkbenchPage.h>
#include <berryIBerryPreferences.h>
#include <berryIEditorPart.h>
#include <berryPlatform.h>

// Qmitk Includes
#include <QmitkStdMultiWidgetEditor.h>

// Qt Includes
#include <QMessageBox>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QApplication>

QmitkFunctionality::QmitkFunctionality()
 : m_Parent(nullptr)
 , m_Active(false)
 , m_Visible(false)
 , m_SelectionProvider(nullptr)
 , m_DataStorageServiceTracker(QmitkCommonLegacyActivator::GetContext())
 , m_HandlesMultipleDataStorages(false)
 , m_InDataStorageChanged(false)
{
  m_DataStorageServiceTracker.open();
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
  mitk::IDataStorageService* service = m_DataStorageServiceTracker.getService();

  if (service != nullptr)
  {
    if(m_HandlesMultipleDataStorages)
      return service->GetActiveDataStorage()->GetDataStorage();
    else
      return service->GetDefaultDataStorage()->GetDataStorage();
  }

  return nullptr;
}

mitk::DataStorage::Pointer QmitkFunctionality::GetDefaultDataStorage() const
{
  mitk::IDataStorageService* service = m_DataStorageServiceTracker.getService();

  if (service != nullptr)
  {
    return service->GetDefaultDataStorage()->GetDataStorage();
  }

  return nullptr;
}

mitk::IDataStorageReference::Pointer QmitkFunctionality::GetDataStorageReference() const
{
  mitk::IDataStorageService* dsService = m_DataStorageServiceTracker.getService();

  if (dsService != nullptr)
  {
    return dsService->GetDataStorage();
  }

  return mitk::IDataStorageReference::Pointer(nullptr);
}

void QmitkFunctionality::CreatePartControl(QWidget* parent)
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
  m_BlueBerrySelectionListener.reset(new berry::SelectionChangedAdapter<QmitkFunctionality>(
                                       this,
                                       &QmitkFunctionality::BlueBerrySelectionChanged)
                                     );
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener(
        /*"org.mitk.views.datamanager",*/ m_BlueBerrySelectionListener.data());

  // REGISTER A SELECTION PROVIDER
  QmitkFunctionalitySelectionProvider::Pointer _SelectionProvider(
        new QmitkFunctionalitySelectionProvider(this));
  m_SelectionProvider = _SelectionProvider.GetPointer();
  this->GetSite()->SetSelectionProvider(berry::ISelectionProvider::Pointer(m_SelectionProvider));

  // EMULATE INITIAL SELECTION EVENTS

  // by default a multi widget is always available
  this->MultiWidgetAvailable(*this->GetActiveMultiWidget());

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
  this->GetSite()->SetSelectionProvider(berry::ISelectionProvider::Pointer(nullptr));

  berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  if(s)
  {
    s->RemovePostSelectionListener(m_BlueBerrySelectionListener.data());
  }

  this->ClosePart();
}

QmitkFunctionality::~QmitkFunctionality()
{
  this->Register();
  this->ClosePartProxy();

  this->UnRegister(false);

  m_DataStorageServiceTracker.close();
}

void QmitkFunctionality::OnPreferencesChanged( const berry::IBerryPreferences* )
{
}

void QmitkFunctionality::BlueBerrySelectionChanged(const berry::IWorkbenchPart::Pointer& sourcepart,
                                                   const berry::ISelection::ConstPointer& selection)
{
  if(sourcepart.IsNull() || sourcepart->GetSite()->GetId() != "org.mitk.views.datamanager")
    return;

  mitk::DataNodeSelection::ConstPointer _DataNodeSelection
    = selection.Cast<const mitk::DataNodeSelection>();
  this->OnSelectionChanged(this->DataNodeSelectionToVector(_DataNodeSelection));
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

void QmitkFunctionality::MultiWidgetAvailable( QmitkAbstractMultiWidget&  /*multiWidget*/ )
{
}
void QmitkFunctionality::MultiWidgetNotAvailable()
{
}

void QmitkFunctionality::DataStorageChanged()
{

}

QmitkAbstractMultiWidget* QmitkFunctionality::GetActiveMultiWidget( bool reCreateWidget )
{
  QmitkAbstractMultiWidget* activeMultiWidget = nullptr;

  berry::IEditorPart::Pointer editor =
    this->GetSite()->GetPage()->GetActiveEditor();

  if (reCreateWidget || editor.Cast<QmitkStdMultiWidgetEditor>().IsNull())
  {
    mitk::DataStorageEditorInput::Pointer editorInput(
          new mitk::DataStorageEditorInput( this->GetDataStorageReference() ));
    // open a new multi-widget editor, but do not give it the focus
    berry::IEditorPart::Pointer editor = this->GetSite()->GetPage()->OpenEditor(editorInput, QmitkStdMultiWidgetEditor::EDITOR_ID, false, berry::IWorkbenchPage::MATCH_ID);
    activeMultiWidget = editor.Cast<QmitkStdMultiWidgetEditor>()->GetMultiWidget();
  }
  else if (editor.Cast<QmitkStdMultiWidgetEditor>().IsNotNull())
  {
    activeMultiWidget = editor.Cast<QmitkStdMultiWidgetEditor>()->GetMultiWidget();
  }

  return activeMultiWidget;
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

void QmitkFunctionality::MultiWidgetClosed( QmitkAbstractMultiWidget&  /*multiWidget*/ )
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
  berry::IPreferencesService* prefService = berry::Platform::GetPreferencesService();
  // const_cast workaround for bad programming: const uncorrectness this->GetViewSite() should be const
  QString id = "/" + (const_cast<QmitkFunctionality*>(this))->GetViewSite()->GetId();
  return prefService != nullptr ? prefService->GetSystemPreferences()->Node(id): berry::IPreferences::Pointer(nullptr);
}

void QmitkFunctionality::Visible()
{

}

void QmitkFunctionality::Hidden()
{

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

