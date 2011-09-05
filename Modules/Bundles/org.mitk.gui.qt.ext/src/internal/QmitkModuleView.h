/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


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

  void SetFocus();
  void CreateQtPartControl(QWidget *parent);

  void Init(berry::IViewSite::Pointer site, berry::IMemento::Pointer memento);
  void SaveState(berry::IMemento::Pointer memento);

private:

  QTableView* tableView;
  berry::IMemento::Pointer viewState;
};

#endif // QMITKMODULEVIEW_H
