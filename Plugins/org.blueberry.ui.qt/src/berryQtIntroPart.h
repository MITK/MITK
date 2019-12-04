/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYQTINTROPART_H_
#define BERRYQTINTROPART_H_

#include <berryIntroPart.h>

#include <org_blueberry_ui_qt_Export.h>

#include <QWidget>

namespace berry
{

class BERRY_UI_QT QtIntroPart : public IntroPart
{

public:

  berryObjectMacro(QtIntroPart);

  void CreatePartControl(void* parent) override;

protected:

  virtual void CreateQtPartControl(QWidget* parent) = 0;

};

}

#endif /* BERRYQTINTROPART_H_ */
