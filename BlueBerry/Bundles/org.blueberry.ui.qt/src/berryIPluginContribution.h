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


#ifndef BERRYIPLUGINCONTRIBUTION_H
#define BERRYIPLUGINCONTRIBUTION_H

#include <berryMacros.h>
#include <berryObject.h>

namespace berry {

/**
 * An interface that descriptor classes may implement in addition to their
 * descriptor interface. This indicates that they may or may not originate from
 * a plugin contribution. This is useful in various activity filtering
 * scenarios.
 */
struct IPluginContribution : public virtual Object
{
  berryObjectMacro(berry::IPluginContribution)

  /**
   * @return the local id of the contribution. Must not be <code>null</code>.
   *         This should correspond to the extension-specific identifier for
   *         a given contribution.
   */
  virtual QString GetLocalId() const = 0;

  /**
   * @return the id of the originating plugin. Can be <code>null</code> if
   *         this contribution did not originate from a plugin.
   */
  virtual QString GetPluginId() const = 0;
};

}

#endif // BERRYIPLUGINCONTRIBUTION_H
