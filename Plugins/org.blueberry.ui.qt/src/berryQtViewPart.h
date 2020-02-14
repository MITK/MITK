/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYQTVIEWPART_H_
#define BERRYQTVIEWPART_H_

#include <berryViewPart.h>

#include <org_blueberry_ui_qt_Export.h>

#include <QWidget>

namespace berry
{

class BERRY_UI_QT QtViewPart : public ViewPart
{

public:

  berryObjectMacro(QtViewPart);

  void CreatePartControl(QWidget* parent) override;

protected:

  virtual void CreateQtPartControl(QWidget* parent) = 0;

};

}

#endif /*BERRYQTVIEWPART_H_*/
