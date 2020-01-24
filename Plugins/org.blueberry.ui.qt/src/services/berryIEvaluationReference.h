/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYIEVALUATIONREFERENCE_H
#define BERRYIEVALUATIONREFERENCE_H

#include "internal/berryIEvaluationResultCache.h"

namespace berry {

struct IPropertyChangeListener;

/**
 * A token representing a core expression and property change listener currently
 * working in the <code>IEvaluationService</code>.
 *
 * @noimplement This interface is not intended to be implemented by clients.
 * @noextend This interface is not intended to be extended by clients.
 */
struct IEvaluationReference : public virtual IEvaluationResultCache
{

  berryObjectMacro(berry::IEvaluationReference);

  /**
   * The property change listener associated with the evaluated expression.
   *
   * @return the listener for updates.
   */
  virtual IPropertyChangeListener* GetListener() const = 0;

  /**
   * The property used in change notifications.
   *
   * @return the property name.
   */
  virtual QString GetProperty() const = 0;

};

}

#endif // BERRYIEVALUATIONREFERENCE_H
