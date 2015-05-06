/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

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

  QHash<QString,QString> GetParameterValues() const;
};

}

#endif // BERRYPERSPECTIVEPARAMETERVALUES_H
