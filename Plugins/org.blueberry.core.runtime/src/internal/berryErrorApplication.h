/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
