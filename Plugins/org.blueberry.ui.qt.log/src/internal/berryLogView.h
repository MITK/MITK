/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYLOGVIEW_H_
#define BERRYLOGVIEW_H_

#include <QWidget>

#include "berryQtViewPart.h"

namespace berry {

class LogView : public QtViewPart
{
  Q_OBJECT

public:

  LogView();

  void SetFocus() override;

protected:

  void CreateQtPartControl(QWidget* parent) override;

};

} // namespace berry

#endif /*BERRYLOGVIEW_H_*/
