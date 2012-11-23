/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef CHANGETEXTTOLOWERCASE_H
#define CHANGETEXTTOLOWERCASE_H

#include <IChangeText.h>

class ChangeTextToLowerCase : public QObject, public IChangeText
{
  Q_OBJECT
  Q_INTERFACES(IChangeText)

public:

  QString ChangeText(const QString& s);
};

#endif // CHANGETEXTTOLOWERCASE_H
