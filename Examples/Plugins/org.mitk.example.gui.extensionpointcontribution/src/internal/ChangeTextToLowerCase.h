/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef CHANGETEXTTOLOWERCASE_H
#define CHANGETEXTTOLOWERCASE_H

#include <IChangeText.h>

class ChangeTextToLowerCase : public QObject, public IChangeText
{
  Q_OBJECT
  Q_INTERFACES(IChangeText)

public:
  QString ChangeText(const QString &s) override;
};

#endif // CHANGETEXTTOLOWERCASE_H
