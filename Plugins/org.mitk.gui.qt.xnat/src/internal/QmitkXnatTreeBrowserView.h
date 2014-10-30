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
#include "ctkXnatTreeModel.h"

// MitkXNAT Module
#include "mitkXnatSessionTracker.h"

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

protected:

  virtual void SetFocus();

  Ui::QmitkXnatTreeBrowserViewControls m_Controls;

private:

  berry::QtSelectionProvider::Pointer m_SelectionProvider;
  void SetSelectionProvider();

  ctkXnatTreeModel* m_TreeModel;
  mitk::XnatSessionTracker* m_Tracker;
};

#endif // QMITKXNATTREEBROWSERVIEW_H
