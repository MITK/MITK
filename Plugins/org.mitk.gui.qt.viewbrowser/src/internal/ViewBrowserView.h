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


#ifndef ViewBrowserView_h
#define ViewBrowserView_h

#include <berryISelectionListener.h>
#include <berryIPerspectiveListener.h>
#include <berryIWorkbench.h>
#include <berryIWorkbenchPage.h>
#include <berryIPerspectiveDescriptor.h>

#include <QmitkAbstractView.h>

#include "ui_ViewBrowserViewControls.h"

#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QMenu>
#include <src/internal/mitkQtPerspectiveItem.h>
#include <src/internal/mitkQtViewItem.h>


/**
  \brief ViewBrowserView

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class ClassFilterProxyModel;

class ViewBrowserView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;


    /// \brief Fills the TreeList with the available perspectives and views.
    void FillTreeList();
    void RemoveListener();


  protected slots:

    /// \brief Called when the user clicks the GUI button
    void CustomMenuRequested(QPoint pos);
    void ItemClicked(const QModelIndex &index);
    void AddPerspective();
    void ClonePerspective();
    void ResetPerspective();
    void DeletePerspective();
    void ClosePerspectives();
    void ClosePerspective();
    void ButtonClicked();
    void FilterChanged();

  protected:

    virtual void CreateQtPartControl(QWidget *parent);

    virtual void SetFocus();

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
                                     const QList<mitk::DataNode::Pointer>& nodes );

    QWidget*                                    m_Parent;
    Ui::ViewBrowserViewControls                 m_Controls;
    QStandardItemModel*                         m_TreeModel;
    ClassFilterProxyModel*                      m_FilterProxyModel;
    QMenu*                                      m_ContextMenu;
    berry::IPerspectiveDescriptor::Pointer      m_RegisteredPerspective;
    berry::IPerspectiveListener::Pointer        perspListener;
    berry::IWindowListener::Pointer                perspListener2;

    friend struct ViewBrowserViewListener;
};

#endif // ViewBrowserView_h
