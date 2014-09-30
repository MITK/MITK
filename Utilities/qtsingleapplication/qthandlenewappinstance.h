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


#ifndef QTHANDLENEWAPPINSTANCE_H
#define QTHANDLENEWAPPINSTANCE_H

#include <QString>

class QtSingleApplication;

bool createTemporaryDir(QString& path);

QString handleNewAppInstance(QtSingleApplication* singleApp, int argc, char** argv, const QString& newInstanceArg);

#endif // QTHANDLENEWAPPINSTANCE_H
