/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkXnatEditor_h
#define QmitkXnatEditor_h

#include <berryIReusableEditor.h>
#include <berryQtEditorPart.h>
#include <berryISelectionListener.h>

#include <mitkIDataStorageService.h>

#include <QmitkAbstractView.h>

#include "ui_QmitkXnatEditorControls.h"

#include "ctkXnatListModel.h"
#include "ctkXnatSession.h"

#include <ctkServiceTracker.h>

#include "mitkXnatSessionTracker.h"

/*!
\brief QmitkXnatEditor

\warning This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

\ingroup ${plugin_target}_internal
*/
class QmitkXnatEditor : public berry::QtEditorPart, public berry::IReusableEditor
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  berryObjectMacro(QmitkXnatEditor);

    QmitkXnatEditor();
  ~QmitkXnatEditor();

  static const QString EDITOR_ID;

  void CreateQtPartControl(QWidget *parent) override;

  void DoSave(/*IProgressMonitor monitor*/) override;
  void DoSaveAs() override;
  void Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input) override;
  bool IsDirty() const override;
  bool IsSaveAsAllowed() const override;
  void SetInput(berry::IEditorInput::Pointer input) override;

  /**
  \brief Here the root object will be set and the view reset. Additionally the breadcrumbs will set visible.
  */
  void UpdateList();

  protected slots:

  /**
  \brief Any XNAT resource (file or folder) will be downloaded to the chosen download path.
  */
  void DownloadResource();

  /**
  \brief Every time you activate a node in the list, the root item will be updated to a child of the previous parent.\
  In exception of the node is a file. The file will be downloaded and loaded to the DataManager.
  */
  void OnObjectActivated(const QModelIndex& index);

  // Breadcrumb button slots
  void OnDataModelButtonClicked();
  void OnProjectButtonClicked();
  void OnSubjectButtonClicked();
  void OnExperimentButtonClicked();
  void OnKindOfDataButtonClicked();
  void OnSessionButtonClicked();
  void OnResourceButtonClicked();

  /// \brief Updates the ctkXnatSession and the user interface
  void UpdateSession(ctkXnatSession* session);

  void CleanListModel(ctkXnatSession* session);
  void itemSelected(const QModelIndex &index);

protected:

  virtual void SetFocus() override;

  Ui::QmitkXnatEditorControls m_Controls;

private:

  int m_ParentCount;
  QString m_DownloadPath;
  ctkServiceTracker<mitk::IDataStorageService*> m_DataStorageServiceTracker;

  void InternalFileDownload(const QModelIndex& index);
  int ParentChecker(ctkXnatObject* child);
  void ToHigherLevel();

  ctkXnatListModel* m_ListModel;
  ctkXnatSession* m_Session;
  mitk::XnatSessionTracker* m_Tracker;

  QScopedPointer<berry::ISelectionListener> m_SelectionListener;
  void SelectionChanged(const berry::IWorkbenchPart::Pointer& sourcepart,
    const berry::ISelection::ConstPointer& selection);
};

#endif
