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


#include "QmitkAbstractRenderEditor.h"
#include "internal/QmitkCommonActivator.h"

#include <mitkDataStorageEditorInput.h>
#include <mitkIRenderingManager.h>

#include <berryIPreferencesService.h>
#include <berryUIException.h>

#include <ctkServiceTracker.h>
#include <ctkPluginContext.h>
#include <service/event/ctkEvent.h>
#include <service/event/ctkEventConstants.h>

class QmitkAbstractRenderEditorPrivate
{
public:

  QmitkAbstractRenderEditorPrivate()
    : m_RenderingManagerInterface(mitk::MakeRenderingManagerInterface(mitk::RenderingManager::GetInstance()))
    , m_PrefServiceTracker(QmitkCommonActivator::GetContext())
    , m_Context(0)
    , m_EventAdmin(0)
  {
    m_PrefServiceTracker.open();
  }

  ~QmitkAbstractRenderEditorPrivate()
  {
    delete m_RenderingManagerInterface;
  }

  mitk::IRenderingManager* m_RenderingManagerInterface;
  ctkServiceTracker<berry::IPreferencesService*> m_PrefServiceTracker;
  berry::IBerryPreferences::Pointer m_Prefs;

  ctkPluginContext* m_Context;
  ctkServiceReference m_EventAdminRef;
  ctkEventAdmin* m_EventAdmin;
};

QmitkAbstractRenderEditor::QmitkAbstractRenderEditor()
  : d(new QmitkAbstractRenderEditorPrivate)
{
}

QmitkAbstractRenderEditor::~QmitkAbstractRenderEditor()
{
  if (d->m_Prefs.IsNotNull())
  {
    d->m_Prefs->OnChanged.RemoveListener(berry::MessageDelegate1<QmitkAbstractRenderEditor, const berry::IBerryPreferences*>
                                         (this, &QmitkAbstractRenderEditor::OnPreferencesChanged ) );
  }
}

void QmitkAbstractRenderEditor::Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input)
{
  if (input.Cast<mitk::DataStorageEditorInput>().IsNull())
    throw berry::PartInitException("Invalid Input: Must be mitk::DataStorageEditorInput");

  this->SetSite(site);
  this->SetInput(input);

  d->m_Prefs = this->GetPreferences().Cast<berry::IBerryPreferences>();
  if (d->m_Prefs.IsNotNull())
  {
    d->m_Prefs->OnChanged.AddListener(berry::MessageDelegate1<QmitkAbstractRenderEditor, const berry::IBerryPreferences*>
                                      (this, &QmitkAbstractRenderEditor::OnPreferencesChanged ) );
  }

  d->m_Context = QmitkCommonActivator::GetContext();
  d->m_EventAdminRef = d->m_Context->getServiceReference<ctkEventAdmin>();
  d->m_EventAdmin = d->m_Context->getService<ctkEventAdmin>(d->m_EventAdminRef);

  ctkDictionary propsForSlot;
  propsForSlot[ctkEventConstants::EVENT_TOPIC] = "org/mitk/gui/qt/INTERACTOR_REQUEST";
  d->m_EventAdmin->subscribeSlot(this, SLOT(handleEvent(ctkEvent)), propsForSlot);
}

mitk::IDataStorageReference::Pointer QmitkAbstractRenderEditor::GetDataStorageReference() const
{
  mitk::DataStorageEditorInput::Pointer input = this->GetEditorInput().Cast<mitk::DataStorageEditorInput>();
  if (input.IsNotNull())
  {
    return input->GetDataStorageReference();
  }
  return mitk::IDataStorageReference::Pointer(0);
}

mitk::DataStorage::Pointer QmitkAbstractRenderEditor::GetDataStorage() const
{
  mitk::IDataStorageReference::Pointer ref = this->GetDataStorageReference();
  if (ref.IsNotNull()) return ref->GetDataStorage();
  return mitk::DataStorage::Pointer(0);
}

berry::IPreferences::Pointer QmitkAbstractRenderEditor::GetPreferences() const
{
  berry::IPreferencesService* prefService = d->m_PrefServiceTracker.getService();
  if (prefService != 0)
  {
    return prefService->GetSystemPreferences()->Node(this->GetSite()->GetId());
  }
  return berry::IPreferences::Pointer(0);
}

mitk::IRenderingManager* QmitkAbstractRenderEditor::GetRenderingManager() const
{
  // we use the global rendering manager here. This should maybe replaced
  // by a local one, managing only the render windows specific for the editor
  return d->m_RenderingManagerInterface;
}

void QmitkAbstractRenderEditor::RequestUpdate(mitk::RenderingManager::RequestType requestType)
{
  if (GetRenderingManager())
    GetRenderingManager()->RequestUpdateAll(requestType);
}

void QmitkAbstractRenderEditor::ForceImmediateUpdate(mitk::RenderingManager::RequestType requestType)
{
  if (GetRenderingManager())
    GetRenderingManager()->ForceImmediateUpdateAll(requestType);
}

mitk::SliceNavigationController* QmitkAbstractRenderEditor::GetTimeNavigationController() const
{
  if (GetRenderingManager())
    return GetRenderingManager()->GetTimeNavigationController();
  return 0;
}

void QmitkAbstractRenderEditor::OnPreferencesChanged(const berry::IBerryPreferences *)
{}

void QmitkAbstractRenderEditor::DoSave()
{}

void QmitkAbstractRenderEditor::DoSaveAs()
{}

bool QmitkAbstractRenderEditor::IsDirty() const
{ return false; }

bool QmitkAbstractRenderEditor::IsSaveAsAllowed() const
{ return false; }

void QmitkAbstractRenderEditor::EnableInteractors(bool enable, const QStringList& interactors)
{
  Q_UNUSED(enable);
  Q_UNUSED(interactors);
}

bool QmitkAbstractRenderEditor::IsInteractorEnabled(const QString& interactor) const
{
  Q_UNUSED(interactor);
  return false;
}

QStringList QmitkAbstractRenderEditor::GetInteractors() const
{
  return QStringList();
}

void QmitkAbstractRenderEditor::handleEvent(const ctkEvent& event)
{
  try
  {
    QString topic = event.getProperty(ctkEventConstants::EVENT_TOPIC).toString();
    if (topic == "org/mitk/gui/qt/INTERACTOR_REQUEST")
    {
      bool enabled = event.getProperty("enabled").toBool();
      QStringList interactors = event.getProperty("interactors").toStringList();

      this->EnableInteractors(enabled, interactors);
    }
  }
  catch (const ctkRuntimeException& e)
  {
    MITK_ERROR << "QmitkAbstractRenderEditor::handleEvent, failed with:" << e.what() \
        << ", caused by " << e.message().toLocal8Bit().constData() \
        << std::endl;
  }
}
