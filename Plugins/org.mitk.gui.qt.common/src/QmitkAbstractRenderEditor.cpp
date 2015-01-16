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
#include <mitkPickedDataNodeSelectionProvider.h>

#include <berryIPreferencesService.h>
#include <berryUIException.h>
#include <berryISelectionService.h>
#include <berryISelectionListener.h>
#include <berryINullSelectionListener.h>

#include <ctkServiceTracker.h>

class QmitkAbstractRenderEditorPrivate
{
public:

  QmitkAbstractRenderEditorPrivate(QmitkAbstractRenderEditor* qq)
      : q(qq)
    , m_RenderingManagerInterface(mitk::MakeRenderingManagerInterface(mitk::RenderingManager::GetInstance()))
    , m_PrefServiceTracker(QmitkCommonActivator::GetContext())
  {
    m_PrefServiceTracker.open();
  }

  ~QmitkAbstractRenderEditorPrivate()
  {
    delete m_RenderingManagerInterface;
  }

  /**
  * reactions to selection events from views
  */
  void BlueBerrySelectionChanged(berry::IWorkbenchPart::Pointer sourcepart, berry::ISelection::ConstPointer selection)
  {
      if (sourcepart.IsNull() || sourcepart.GetPointer() == static_cast<berry::IWorkbenchPart*>(q))
          return;

      m_SelectionProvider->SetSelection(selection);
  }

  QmitkAbstractRenderEditor* q;
  mitk::IRenderingManager* m_RenderingManagerInterface;
  ctkServiceTracker<berry::IPreferencesService*> m_PrefServiceTracker;
  berry::IBerryPreferences::Pointer m_Prefs;
  berry::ISelectionListener::Pointer m_BlueBerrySelectionListener;
  berry::ISelectionProvider::Pointer m_SelectionProvider;
};

QmitkAbstractRenderEditor::QmitkAbstractRenderEditor()
  : d(new QmitkAbstractRenderEditorPrivate(this))
{
}

QmitkAbstractRenderEditor::~QmitkAbstractRenderEditor()
{
  if (d->m_Prefs.IsNotNull())
  {
    d->m_Prefs->OnChanged.RemoveListener(berry::MessageDelegate1<QmitkAbstractRenderEditor, const berry::IBerryPreferences*>
                                         (this, &QmitkAbstractRenderEditor::OnPreferencesChanged ) );
  }

  berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  if (s)
  {
      s->RemovePostSelectionListener(d->m_BlueBerrySelectionListener);
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

  this->SetSelectionProvider();

  d->m_BlueBerrySelectionListener = berry::ISelectionListener::Pointer(
      new berry::NullSelectionChangedAdapter<QmitkAbstractRenderEditorPrivate>(d.data(),
      &QmitkAbstractRenderEditorPrivate::BlueBerrySelectionChanged));
  this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->AddPostSelectionListener(d->m_BlueBerrySelectionListener);

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

void QmitkAbstractRenderEditor::SetSelectionProvider()
{
    d->m_SelectionProvider = berry::ISelectionProvider::Pointer(new mitk::PickedDataNodeSelectionProvider());
    this->GetSite()->SetSelectionProvider(d->m_SelectionProvider);
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
