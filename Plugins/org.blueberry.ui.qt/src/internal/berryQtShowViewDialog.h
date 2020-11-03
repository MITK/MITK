/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYQTSHOWVIEWDIALOG_H_
#define BERRYQTSHOWVIEWDIALOG_H_

#include <berrySmartPointer.h>

#include <QDialog>

#include "ui_berryQtShowViewDialog.h"

namespace berry {

struct IViewRegistry;
struct IWorkbenchWindow;

class ViewFilterProxyModel;

class QtShowViewDialog : public QDialog
{
  Q_OBJECT

public:

  QtShowViewDialog(const IWorkbenchWindow* window, IViewRegistry* registry,
                   QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);

  QList<QString> GetSelection() const;

private:

  Q_SLOT void setDescription(const QModelIndex& index);
  Q_SLOT void enableKeywordFilter(bool enable);

  Q_SLOT void setFilter(const QString& filter);

  Q_SLOT void categoryCollapsed(const QModelIndex& index);
  Q_SLOT void categoryExpanded(const QModelIndex& index);

  Q_SLOT void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

  void RestoreExpandedState();
  void UpdateButtons();

  void RestoreState();
  void SaveState();

  void done(int r) override;

  const IWorkbenchWindow* m_Window;
  IViewRegistry* m_ViewReg;
  Ui::QtShowViewDialog_ m_UserInterface;
  ViewFilterProxyModel* m_FilterModel;
  QList<QPersistentModelIndex> m_ExpandedCategories;
};

}

#endif /*BERRYQTSHOWVIEWDIALOG_H_*/
