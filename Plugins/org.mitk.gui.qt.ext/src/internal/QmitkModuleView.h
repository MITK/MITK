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
