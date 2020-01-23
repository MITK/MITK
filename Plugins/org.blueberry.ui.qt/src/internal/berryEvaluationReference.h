/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYEVALUATIONREFERENCE_H
#define BERRYEVALUATIONREFERENCE_H

#include "berryEvaluationResultCache.h"
#include <berryIEvaluationReference.h>

namespace berry {

class EvaluationReference : public EvaluationResultCache, public IEvaluationReference
{

private:

  IPropertyChangeListener* listener;
  QString property;
  bool postingChanges;

public:

  berryObjectMacro(berry::EvaluationReference);

  /**
   * @param expression
   */
  EvaluationReference(const SmartPointer<Expression>& expression,
                      IPropertyChangeListener *listener,
                      const QString& property);

  /*
   * @see IEvaluationReference#GetListener()
   */
  IPropertyChangeListener* GetListener() const override;

  QString GetProperty() const override;

  /*
   * @see IEvaluationReference#SetFlopping(bool)
   */
  void SetPostingChanges(bool evaluationEnabled);

  /*
   * @see IEvaluationReference#IsFlopping()
   */
  bool IsPostingChanges() const;
};

}

#endif // BERRYEVALUATIONREFERENCE_H
