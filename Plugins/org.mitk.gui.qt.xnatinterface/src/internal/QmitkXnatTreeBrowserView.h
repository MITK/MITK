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

#include <berryISelectionListener.h>
#include <berryQtSelectionProvider.h>

#include <QmitkAbstractView.h>

#include "ui_QmitkXnatTreeBrowserViewControls.h"

// ctkXnatCore
#include "ctkXnatSession.h"
#include "ctkXnatLoginProfile.h"

// ctkXnatWidget
#include "ctkXnatTreeModel.h"

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

  //berryObjectMacro(QmitkXnatTreeBrowserView);
  QmitkXnatTreeBrowserView();
  ~QmitkXnatTreeBrowserView();
  static const std::string VIEW_ID;

  virtual void CreateQtPartControl(QWidget *parent);

  berry::QtSelectionProvider::Pointer m_SelectionProvider;

  protected slots:

    /// \brief Called when the user clicks the GUI button
    void StartBrowser();
    void OnActivatedNode(const QModelIndex& index);
    void DoFetchMore(const QModelIndex& index);

protected:

  virtual void SetFocus();

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
    const QList<mitk::DataNode::Pointer>& nodes );

  Ui::QmitkXnatTreeBrowserViewControls m_Controls;

private:

  friend class QmitkXnatEditor;

  ctkXnatTreeModel* GetTreeModel();
  void SetSelectionProvider();

  ctkXnatSession* m_Session;
  ctkXnatTreeModel* m_TreeModel;
  ctkXnatLoginProfile* m_Profile;
};

#endif // QMITKXNATTREEBROWSERVIEW_H
