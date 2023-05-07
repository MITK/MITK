/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkXnatTreeBrowserView_h
#define QmitkXnatTreeBrowserView_h

#include <QmitkAbstractView.h>

#include "ui_QmitkXnatTreeBrowserViewControls.h"
#include "QmitkHttpStatusCodeHandler.h"

// ctkXnatCore
#include "ctkXnatSession.h"

// ctkXnatWidget
#include "QmitkXnatTreeModel.h"

// MitkXNAT Module
#include "mitkXnatSessionTracker.h"

#include <mitkIDataStorageService.h>
#include <ctkServiceTracker.h>

class QMenu;

/*!
\brief QmitkXnatTreeBrowserView

\warning This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

\ingroup ${plugin_target}_internal
*/
class QmitkXnatTreeBrowserView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  QmitkXnatTreeBrowserView();
  ~QmitkXnatTreeBrowserView() override;

  static const QString VIEW_ID;

  void CreateQtPartControl(QWidget *parent) override;

  enum SearchMethod {
    ProjectLevel = 0,
    SubjectLevel = 1
  };

protected slots:

  /// \brief Opens or reuses the xnat editor with the activated node as root item.
  void OnActivatedNode(const QModelIndex& index);

  /// \brief Updates the ctkXnatSession and the user interface
  void UpdateSession(ctkXnatSession* session);

  /// \brief Cleans the tree model
  void CleanTreeModel(ctkXnatSession* session);

  /// \brief Searches the tree model
  void Search(const QString &toSearch);

  void OnContextMenuRequested(const QPoint & pos);
  void OnContextMenuDownloadAndOpenFile();
  void OnContextMenuDownloadFile();
  void OnContextMenuCreateResourceFolder();
  void OnContextMenuUploadFile();
  void OnContextMenuCreateNewSubject();
  void OnContextMenuCreateNewExperiment();
  void OnContextMenuCopyXNATUrlToClipboard();
  void OnContextMenuRefreshItem();

  void OnUploadResource(const QList<mitk::DataNode*>& , ctkXnatObject *, const QModelIndex &parentIndex);

  void OnProgress(QUuid, double);

  void ItemSelected(const QModelIndex& index);

  void OnUploadFromDataStorage();

  void SessionTimedOutMsg();
  void SessionTimesOutSoonMsg();

  void ToggleConnection();

protected:

  void SetFocus() override;

  Ui::QmitkXnatTreeBrowserViewControls m_Controls;

private slots:
  void OnXnatNodeSelected(const QModelIndex &index);
  void OnDownloadSelectedXnatFile();
  void OnCreateResourceFolder();

private:

  void OnPreferencesChanged(const mitk::IPreferences*) override;

  void InternalFileDownload(const QModelIndex& index, bool loadData);
  void InternalDICOMDownload(ctkXnatObject* obj, QDir &DICOMDirPath);
  void InternalFileUpload(ctkXnatFile *file);
  ctkXnatResource* InternalAddResourceFolder(ctkXnatObject* parent);

  void InternalOpenFiles(const QFileInfoList&, mitk::StringProperty::Pointer xnatURL);

  void SetStatusInformation(const QString&);
  void FilePathNotAvailableWarning(QString file);

  void CleanUp();

  ctkServiceTracker<mitk::IDataStorageService*> m_DataStorageServiceTracker;
  QmitkXnatTreeModel* m_TreeModel;

  mitk::XnatSessionTracker* m_Tracker;
  QString m_DownloadPath;

  QMenu* m_ContextMenu;

  bool m_SilentMode;
  QModelIndexList m_hiddenItems;

  bool m_AlreadyInSearch = false;

  std::string ReplaceSpecialChars(const std::string& input) const;
};

#endif
