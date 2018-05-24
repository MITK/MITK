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
