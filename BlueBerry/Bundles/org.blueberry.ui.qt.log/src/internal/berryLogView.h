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
  LogView(const LogView& other);

  void SetFocus();

protected:

  void CreateQtPartControl(QWidget* parent);

};

} // namespace berry

#endif /*BERRYLOGVIEW_H_*/
