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

#ifndef QMITKXNATSIMPLESEARCHVIEW_H
#define QMITKXNATSIMPLESEARCHVIEW_H

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>
#include "QmitkXnatTreeBrowserView.h"

#include "ui_QmitkXnatSimpleSearchViewControls.h"

// ctkXnatCore
#include "ctkXnatSession.h"

// ctkXnatWidget
#include "ctkXnatTreeModel.h"

/*!
\brief QmitkXnatSimpleSearchView

\warning This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

\ingroup ${plugin_target}_internal
*/
class QmitkXnatSimpleSearchView : public QmitkXnatTreeBrowserView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  enum MatchingObject
  {
    EMPTY,
    PROJECT,
    SUBJECT,
    EXPERIMENT
  };

  QmitkXnatSimpleSearchView();
  ~QmitkXnatSimpleSearchView();
  static const std::string VIEW_ID;

  virtual void CreateQtPartControl(QWidget *parent);

  berry::QtSelectionProvider::Pointer m_SelectionProvider;

  protected slots:

    /// \brief Called when the user clicks the GUI button
    void StartSearch();

protected:

  virtual void SetFocus();

  Ui::QmitkXnatSimpleSearchViewControls m_Controls;

private:

  void SetSelectionProvider();

  ctkXnatSession* m_Session;
  ctkXnatTreeModel* m_TreeModel;

};

#endif // QMITKXNATSIMPLESEARCHVIEW_H
