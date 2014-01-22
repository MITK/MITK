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

#include "QWonderString.h"

#if QT_VERSION >= 0x050000
  // Qt 5 or higher
  #include <QUuid>
#else
  // Qt 4
  #include <QUuid>
#endif

QWonderString::QWonderString(QObject* parent, const QString& text)
:QObject(parent)
,m_Text(text)
{
}

QString QWonderString::text()
{
  return m_Text;
}

void QWonderString::setText(const QString& text)
{
  m_Text = text;

  if (m_Text.contains("y"))
    emit happy();

  if (m_Text.contains("k"))
    emit sad();
}

QString QWonderString::penguinUID()
{
  QUuid id = QUuid::createUuid();
#if QT_VERSION >= 0x050000
  QString returnValue = id.toString(); // explicit in Qt 5
#else
  QString returnValue = id; // implicit in Qt 4
#endif

  return returnValue;
}
