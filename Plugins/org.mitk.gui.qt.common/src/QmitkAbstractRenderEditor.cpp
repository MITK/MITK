/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "QmitkAbstractRenderEditor.h"
#include "internal/QmitkCommonActivator.h"

#include <mitkDataStorageEditorInput.h>
#include <mitkIRenderingManager.h>
#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

#include <berryUIException.h>

class QmitkAbstractRenderEditorPrivate
{
public:

  QmitkAbstractRenderEditorPrivate()
    : m_RenderingManagerInterface(mitk::MakeRenderingManagerInterface(mitk::RenderingManager::GetInstance()))
  {
  }

  ~QmitkAbstractRenderEditorPrivate()
  {
    delete m_RenderingManagerInterface;
  }

  mitk::IRenderingManager* m_RenderingManagerInterface;
  mitk::IPreferences* m_Prefs;
};

QmitkAbstractRenderEditor::QmitkAbstractRenderEditor()
  : d(new QmitkAbstractRenderEditorPrivate)
{
}

QmitkAbstractRenderEditor::~QmitkAbstractRenderEditor()
{
  if (d->m_Prefs != nullptr)
  {
    d->m_Prefs->OnChanged.RemoveListener(mitk::MessageDelegate1<QmitkAbstractRenderEditor, const mitk::IPreferences*>
                                         (this, &QmitkAbstractRenderEditor::OnPreferencesChanged ) );
  }
}

void QmitkAbstractRenderEditor::Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input)
{
  if (input.Cast<mitk::DataStorageEditorInput>().IsNull())
    throw berry::PartInitException("Invalid Input: Must be mitk::DataStorageEditorInput");

  this->SetSite(site);
  this->SetInput(input);

  d->m_Prefs = this->GetPreferences();
  if (d->m_Prefs != nullptr)
  {
    d->m_Prefs->OnChanged.AddListener(mitk::MessageDelegate1<QmitkAbstractRenderEditor, const mitk::IPreferences*>
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
  return mitk::IDataStorageReference::Pointer(nullptr);
}

mitk::DataStorage::Pointer QmitkAbstractRenderEditor::GetDataStorage() const
{
  mitk::IDataStorageReference::Pointer ref = this->GetDataStorageReference();
  if (ref.IsNotNull()) return ref->GetDataStorage();
  return mitk::DataStorage::Pointer(nullptr);
}

mitk::IPreferences* QmitkAbstractRenderEditor::GetPreferences() const
{
  mitk::CoreServicePointer prefService(mitk::CoreServices::GetPreferencesService());
  return prefService->GetSystemPreferences()->Node(this->GetSite()->GetId().toStdString());
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
  return nullptr;
}

void QmitkAbstractRenderEditor::OnPreferencesChanged(const mitk::IPreferences *)
{}

void QmitkAbstractRenderEditor::DoSave()
{}

void QmitkAbstractRenderEditor::DoSaveAs()
{}

bool QmitkAbstractRenderEditor::IsDirty() const
{ return false; }

bool QmitkAbstractRenderEditor::IsSaveAsAllowed() const
{ return false; }

mitk::TimePointType QmitkAbstractRenderEditor::GetSelectedTimePoint(const QString& /*id*/) const
{
  auto timeNavigator = this->GetTimeNavigationController();
  if (nullptr != timeNavigator)
  {
    return timeNavigator->GetSelectedTimePoint();
  }
  return 0;
}
