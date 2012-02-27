/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef QTHANDLENEWAPPINSTANCE_H
#define QTHANDLENEWAPPINSTANCE_H

#include <QtCore/QString>

class QtSingleApplication;

bool createTemporaryDir(QString& path);

QString handleNewAppInstance(QtSingleApplication* singleApp, int argc, char** argv, const QString& newInstanceArg);

#endif // QTHANDLENEWAPPINSTANCE_H
