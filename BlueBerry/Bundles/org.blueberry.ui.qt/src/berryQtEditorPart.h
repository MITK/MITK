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

#ifndef BERRYQTEDITORPART_H_
#define BERRYQTEDITORPART_H_

#include <berryEditorPart.h>
#include <QWidget>

#include <org_blueberry_ui_qt_Export.h>

namespace berry
{

class BERRY_UI_QT QtEditorPart : public EditorPart
{
public:

  berryObjectMacro(QtEditorPart);

  void CreatePartControl(void* parent);

protected:

  virtual void CreateQtPartControl(QWidget* parent) = 0;
};

}
#endif /*BERRYQTEDITORPART_H_*/
