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

#ifndef BERRYERRORAPPLICATION_H
#define BERRYERRORAPPLICATION_H

#include <berryIApplication.h>

#include <QObject>

namespace berry {

/*
 * Special case class only used to throw exceptions when an application
 * cannot be found.
 */
class ErrorApplication : public QObject, public IApplication
{
  Q_OBJECT
  Q_INTERFACES(berry::IApplication)

public:

  static const QString ERROR_EXCEPTION; // = "error.exception";

  QVariant Start(IApplicationContext* context) override;

  void Stop() override;
};

}

#endif // BERRYERRORAPPLICATION_H
