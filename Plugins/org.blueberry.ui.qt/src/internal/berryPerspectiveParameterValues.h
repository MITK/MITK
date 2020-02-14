/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYPERSPECTIVEPARAMETERVALUES_H
#define BERRYPERSPECTIVEPARAMETERVALUES_H

#include <berryIParameterValues.h>

namespace berry {

/**
 * Provides the parameter values for the show perspective command.
 */
class PerspectiveParameterValues : public QObject, public IParameterValues
{
  Q_OBJECT

public:

  QHash<QString,QString> GetParameterValues() const override;
};

}

#endif // BERRYPERSPECTIVEPARAMETERVALUES_H
