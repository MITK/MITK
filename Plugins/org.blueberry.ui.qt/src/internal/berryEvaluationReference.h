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

  berryObjectMacro(berry::EvaluationReference)

  /**
   * @param expression
   */
  EvaluationReference(const SmartPointer<Expression>& expression,
                      IPropertyChangeListener *listener,
                      const QString& property);

  /*
   * @see IEvaluationReference#GetListener()
   */
  IPropertyChangeListener* GetListener() const;

  QString GetProperty() const;

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
