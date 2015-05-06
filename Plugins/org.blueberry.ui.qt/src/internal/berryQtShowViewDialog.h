/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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
                   QWidget* parent = 0, Qt::WindowFlags f = 0);

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
