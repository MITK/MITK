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


#include "QmitkAbstractRenderEditor.h"
#include "internal/QmitkCommonActivator.h"

#include <mitkDataStorageEditorInput.h>
#include <mitkIRenderingManager.h>

#include <berryIPreferencesService.h>
#include <berryUIException.h>

#include <ctkServiceTracker.h>

class QmitkAbstractRenderEditorPrivate
{
public:

  QmitkAbstractRenderEditorPrivate()
    : m_RenderingManagerInterface(mitk::MakeRenderingManagerInterface(mitk::RenderingManager::GetInstance()))
    , m_PrefServiceTracker(QmitkCommonActivator::GetContext())
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
