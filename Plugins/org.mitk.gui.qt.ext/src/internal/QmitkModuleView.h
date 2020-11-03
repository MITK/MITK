/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QMITKMODULEVIEW_H
#define QMITKMODULEVIEW_H

#include <berryQtViewPart.h>

class QTableView;

class QmitkModuleView : public berry::QtViewPart
{
  Q_OBJECT

public:
  QmitkModuleView();

protected:

  void SetFocus() override;
  void CreateQtPartControl(QWidget *parent) override;

  void Init(berry::IViewSite::Pointer site, berry::IMemento::Pointer memento) override;
  void SaveState(berry::IMemento::Pointer memento) override;

private:

  QTableView* tableView;
  berry::IMemento::Pointer viewState;
};

#endif // QMITKMODULEVIEW_H
