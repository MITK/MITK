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

#ifndef QWonderString_h
#define QWonderString_h

#include <QObject>

#include "Qt4Qt5TestModuleExports.h"

/**
  \brief PURPOSELESS class for testing.

  This class is meant to demonstrate a module that builds with both Qt4 and Qt5!

  There is nothing here to copy..
*/
class Qt4Qt5TestModule_EXPORT QWonderString : public QObject
{
  Q_OBJECT

  public:

    QWonderString(QObject* parent = 0, const QString& text = "");

    QString text();

    static QString penguinUID();

  public slots:
    void setText(const QString& text);

  signals:

    void happy();
    void sad();

  protected:

    QString m_Text;
};

#endif
