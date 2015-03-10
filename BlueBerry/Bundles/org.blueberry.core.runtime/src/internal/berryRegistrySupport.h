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

#ifndef BERRYREGISTRYSUPPORT_H
#define BERRYREGISTRYSUPPORT_H

#include <berrySmartPointer.h>

class QTranslator;

namespace berry {

struct IStatus;

/**
 * Simple implementation of the registry support functionality.
 * The logging output is done with the BERRY_LOG macros (for both specific and generic logs)
 * in the following format:
 *
 * [Error|Warning|Log]: Main error message
 * [Error|Warning|Log]: Child error message 1
 *   ...
 * [Error|Warning|Log]: Child error message N
 *
 * The translation routine assumes that keys are prefixed with '%'. If no resource
 * bundle is present, the key itself (without leading '%') is returned. There is
 * no decoding for the leading '%%'.
 */
class RegistrySupport
{

public:

  static QString Translate(const QString& key, QTranslator* resources);

  static void Log(const SmartPointer<IStatus>& status, const QString& prefix);
};

}

#endif // BERRYREGISTRYSUPPORT_H
