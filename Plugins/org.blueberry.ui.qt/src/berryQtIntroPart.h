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

  void CreatePartControl(void* parent);

protected:

  virtual void CreateQtPartControl(QWidget* parent) = 0;

};

}

#endif /* BERRYQTINTROPART_H_ */
