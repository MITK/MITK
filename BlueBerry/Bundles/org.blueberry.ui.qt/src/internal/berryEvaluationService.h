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


#ifndef BERRYEVALUATIONSERVICE_H
#define BERRYEVALUATIONSERVICE_H

#include <berryIEvaluationService.h>

namespace berry {

class EvaluationAuthority;

class EvaluationService : public IEvaluationService
{

private:

  SmartPointer<EvaluationAuthority> evaluationAuthority;

public:

  berryObjectMacro(berry::EvaluationService)

  EvaluationService();

  ~EvaluationService();

  /*
   * @see IEvaluationService#AddEvaluationListener(Expression, IPropertyChangeListener, QString)
   */
  SmartPointer<IEvaluationReference> AddEvaluationListener(const SmartPointer<Expression>& expression,
                                                           const SmartPointer<IPropertyChangeListener>& listener,
                                                           const QString& property);

  /*
   * @see IEvaluationService#AddEvaluationReference(IEvaluationReference)
   */
  void AddEvaluationReference(const SmartPointer<IEvaluationReference>& ref);

  /*
   * @see IEvaluationService#RemoveEvaluationListener(IEvaluationReference)
   */
  void RemoveEvaluationListener(const SmartPointer<IEvaluationReference>& ref);

  /*
   * @see IServiceWithSources#AddSourceProvider(ISourceProvider)
   */
  void AddSourceProvider(SmartPointer<ISourceProvider> provider);

  /*
   * @see IServiceWithSources#RemoveSourceProvider(ISourceProvider)
   */
  void RemoveSourceProvider(SmartPointer<ISourceProvider> provider);

  /*
   * @see IDisposable#Dispose()
   */
  void Dispose();

  /*
   * @see IEvaluationService#GetCurrentState()
   */
  SmartPointer<IEvaluationContext> GetCurrentState() const;

  /*
   * @see IEvaluationService#AddServiceListener(IPropertyChangeListener)
   */
  void AddServiceListener(const SmartPointer<IPropertyChangeListener>& listener);

  /*
   * @see IEvaluationService#removeServiceListener(IPropertyChangeListener)
   */
  void RemoveServiceListener(const SmartPointer<IPropertyChangeListener>& listener);

  /*
   * @see IEvaluationService#RequestEvaluation(QString)
   */
  void RequestEvaluation(const QString& propertyName);

  //void UpdateShellKludge();

};

}

#endif // BERRYEVALUATIONSERVICE_H
