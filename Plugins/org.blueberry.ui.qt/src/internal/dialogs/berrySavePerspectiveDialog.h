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

#ifndef BERRYSAVEPERSPECTIVEDIALOG_H
#define BERRYSAVEPERSPECTIVEDIALOG_H

#include <berrySmartPointer.h>

#include <QDialog>

class QItemSelection;

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

  SavePerspectiveDialog(PerspectiveRegistry& perspReg, QWidget *parent = 0);
  ~SavePerspectiveDialog();

  void SetInitialSelection(const SmartPointer<IPerspectiveDescriptor>& initialSelection);

  SmartPointer<IPerspectiveDescriptor> GetPersp() const;

  QString GetPerspName() const;

protected:

  virtual void accept();

  Q_SLOT void PerspectiveNameChanged(const QString& name);
  Q_SLOT void PerspectiveSelected(const QItemSelection& selected, const QItemSelection& deselected);

private:

  void UpdateButtons();

  Ui::SavePerspectiveDialog* ui;
  PerspectiveListModel* model;

  PerspectiveRegistry& perspReg;
  SmartPointer<IPerspectiveDescriptor> persp;
  QString perspName;
};

}

#endif // BERRYSAVEPERSPECTIVEDIALOG_H
