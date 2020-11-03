/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIQTPREFERENCEPAGE_H_
#define BERRYIQTPREFERENCEPAGE_H_

#include <QWidget>

#include "berryIPreferencePage.h"

#include <org_blueberry_ui_qt_Export.h>

namespace berry
{

/**
 * \ingroup org_blueberry_ui_qt
 *
 */
struct BERRY_UI_QT IQtPreferencePage : public IPreferencePage
{

  berryObjectMacro(berry::IQtPreferencePage);

  virtual void CreateQtControl(QWidget* parent) = 0;
  virtual QWidget* GetQtControl() const = 0;

protected:

  void CreateControl(void* parent) override;
  void* GetControl() const override;

};

}

#endif /*BERRYIQTPREFERENCEPAGE_H_*/
