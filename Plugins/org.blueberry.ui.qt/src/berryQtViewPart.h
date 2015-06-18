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

#ifndef BERRYQTVIEWPART_H_
#define BERRYQTVIEWPART_H_

#ifdef __MINGW32__
// We need to inlclude winbase.h here in order to declare
// atomic intrinsics like InterlockedIncrement correctly.
// Otherwhise, they would be declared wrong within qatomic_windows.h .
#include <windows.h>
#endif

#include <berryViewPart.h>

#include <org_blueberry_ui_qt_Export.h>

#include <QWidget>

namespace berry
{

class BERRY_UI_QT QtViewPart : public ViewPart
{

public:

  berryObjectMacro(QtViewPart)

  void CreatePartControl(QWidget* parent) override;

protected:

  virtual void CreateQtPartControl(QWidget* parent) = 0;

};

}

#endif /*BERRYQTVIEWPART_H_*/
