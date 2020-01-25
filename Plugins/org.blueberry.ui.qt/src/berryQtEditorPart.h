/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

  berryObjectMacro(QtEditorPart, EditorPart);

  void CreatePartControl(QWidget* parent) override;

protected:

  virtual void CreateQtPartControl(QWidget* parent) = 0;
};

}
#endif /*BERRYQTEDITORPART_H_*/
