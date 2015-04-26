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

#ifndef QMITKXNATTREEBROWSERVIEW_H
#define QMITKXNATTREEBROWSERVIEW_H

#include <berryQtSelectionProvider.h>

#include <QmitkAbstractView.h>

#include "ui_QmitkXnatTreeBrowserViewControls.h"

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

\sa QmitkFunctionality
\ingroup ${plugin_target}_internal
*/
class QmitkXnatTreeBrowserView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  QmitkXnatTreeBrowserView();
  ~QmitkXnatTreeBrowserView();

  static const std::string VIEW_ID;

  virtual void CreateQtPartControl(QWidget *parent);

  protected slots:

  /// \brief Opens or reuses the xnat editor with the activated node as root item.
  void OnActivatedNode(const QModelIndex& index);

  /// \brief Updates the ctkXnatSession and the user interface
  void UpdateSession(ctkXnatSession* session);

  /// \brief Cleans the tree model
  void CleanTreeModel(ctkXnatSession* session);

  void OnContextMenuRequested(const QPoint & pos);
  void OnContextMenuDownloadAndOpenFile();
  void OnContextMenuDownloadFile();
  void OnContextMenuCreateResourceFolder();
  void OnContextMenuUploadFile();

  void OnUploadResource(const QList<mitk::DataNode*>& , ctkXnatObject *);

  void OnProgress(QUuid, double);

protected:

  virtual void SetFocus();

  Ui::QmitkXnatTreeBrowserViewControls m_Controls;

private:

  void InternalFileDownload(const QModelIndex& index, bool loadData);
  void InternalFileUpload(ctkXnatFile *file);
  ctkXnatResource* InternalAddResourceFolder(ctkXnatObject* parent);

  berry::QtSelectionProvider::Pointer m_SelectionProvider;
  void SetSelectionProvider();

  ctkServiceTracker<mitk::IDataStorageService*> m_DataStorageServiceTracker;
  QmitkXnatTreeModel* m_TreeModel;

  mitk::XnatSessionTracker* m_Tracker;
  QString m_DownloadPath;

  QMenu* m_ContextMenu;
};

#endif // QMITKXNATTREEBROWSERVIEW_H
