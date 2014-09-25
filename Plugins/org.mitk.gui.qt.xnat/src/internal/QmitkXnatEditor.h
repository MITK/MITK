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


#ifndef QMITKXNATEDITOR_h
#define QMITKXNATEDITOR_h

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

\sa QmitkFunctionality
\ingroup ${plugin_target}_internal
*/
class QmitkXnatEditor : public berry::QtEditorPart, public berry::IReusableEditor
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  berryObjectMacro(QmitkXnatEditor)

  QmitkXnatEditor();
  ~QmitkXnatEditor();

  static const std::string EDITOR_ID;

  void CreateQtPartControl(QWidget *parent);

  void DoSave(/*IProgressMonitor monitor*/);
  void DoSaveAs();
  void Init(berry::IEditorSite::Pointer site, berry::IEditorInput::Pointer input);
  bool IsDirty() const;
  bool IsSaveAsAllowed() const;
  void SetInput(berry::IEditorInput::Pointer input);

  /**
  \brief Here the root object will be set and the view reset. Additionally the breadcrumbs will set visible.
  */
  void UpdateList();

  protected slots:

    /**
    \brief A resource folder will be downloaded to the chosen download path.
    */
    void DownloadResource();

    /**
    \brief A file will be downloaded to the chosen download path.
    */
    void DownloadFile();

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

protected:

  virtual void SetFocus();

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

  berry::ISelectionListener::Pointer m_SelectionListener;
  void SelectionChanged(berry::IWorkbenchPart::Pointer sourcepart, berry::ISelection::ConstPointer selection);
};

#endif // QMITKXNATEDITOR_h
