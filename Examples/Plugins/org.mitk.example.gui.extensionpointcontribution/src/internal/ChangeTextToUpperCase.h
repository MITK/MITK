/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef CHANGETEXTTOUPPERCASE_H
#define CHANGETEXTTOUPPERCASE_H

#include <IChangeText.h>

class ChangeTextToUpperCase : public QObject, public IChangeText
{
  Q_OBJECT
  Q_INTERFACES(IChangeText)

public:
  QString ChangeText(const QString &s) override;
};

#endif // CHANGETEXTTOUPPERCASE_H
