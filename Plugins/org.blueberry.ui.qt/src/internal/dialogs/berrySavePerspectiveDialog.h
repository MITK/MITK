/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYSAVEPERSPECTIVEDIALOG_H
#define BERRYSAVEPERSPECTIVEDIALOG_H

#include <berrySmartPointer.h>

#include <QDialog>

class QItemSelection;
class QSortFilterProxyModel;

namespace Ui {
class SavePerspectiveDialog;
}

namespace berry {

struct IPerspectiveDescriptor;
class PerspectiveRegistry;
class PerspectiveListModel;

class SavePerspectiveDialog : public QDialog
{
  Q_OBJECT

public:

  SavePerspectiveDialog(PerspectiveRegistry& perspReg, QWidget *parent = nullptr);
  ~SavePerspectiveDialog() override;

  void SetInitialSelection(const SmartPointer<IPerspectiveDescriptor>& initialSelection);

  SmartPointer<IPerspectiveDescriptor> GetPersp() const;

  QString GetPerspName() const;

protected:

  void accept() override;

  Q_SLOT void PerspectiveNameChanged(const QString& name);
  Q_SLOT void PerspectiveSelected(const QItemSelection& selected, const QItemSelection& deselected);

private:

  void UpdateButtons();

  Ui::SavePerspectiveDialog* ui;
  PerspectiveListModel* model;
  QSortFilterProxyModel* proxyModel;

  PerspectiveRegistry& perspReg;
  SmartPointer<IPerspectiveDescriptor> persp;
  QString perspName;
};

}

#endif // BERRYSAVEPERSPECTIVEDIALOG_H
