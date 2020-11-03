/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYEVALUATIONSERVICE_H
#define BERRYEVALUATIONSERVICE_H

#include <berryIEvaluationService.h>

namespace berry {

class EvaluationAuthority;

class EvaluationService : public IEvaluationService
{

private:

  QScopedPointer<EvaluationAuthority, QScopedPointerObjectDeleter> evaluationAuthority;

public:

  berryObjectMacro(berry::EvaluationService);

  EvaluationService();

  ~EvaluationService() override;

  /*
   * @see IEvaluationService#AddEvaluationListener(Expression, IPropertyChangeListener, QString)
   */
  SmartPointer<IEvaluationReference> AddEvaluationListener(const SmartPointer<Expression>& expression,
                                                           IPropertyChangeListener* listener,
                                                           const QString& property) override;

  /*
   * @see IEvaluationService#AddEvaluationReference(IEvaluationReference)
   */
  void AddEvaluationReference(const SmartPointer<IEvaluationReference>& ref) override;

  /*
   * @see IEvaluationService#RemoveEvaluationListener(IEvaluationReference)
   */
  void RemoveEvaluationListener(const SmartPointer<IEvaluationReference>& ref) override;

  /*
   * @see IServiceWithSources#AddSourceProvider(ISourceProvider)
   */
  void AddSourceProvider(const SmartPointer<ISourceProvider>& provider) override;

  /*
   * @see IServiceWithSources#RemoveSourceProvider(ISourceProvider)
   */
  void RemoveSourceProvider(const SmartPointer<ISourceProvider>& provider) override;

  /*
   * @see IDisposable#Dispose()
   */
  void Dispose() override;

  /*
   * @see IEvaluationService#GetCurrentState()
   */
  SmartPointer<IEvaluationContext> GetCurrentState() const override;

  /*
   * @see IEvaluationService#AddServiceListener(IPropertyChangeListener)
   */
  void AddServiceListener(IPropertyChangeListener *listener) override;

  /*
   * @see IEvaluationService#removeServiceListener(IPropertyChangeListener)
   */
  void RemoveServiceListener(IPropertyChangeListener* listener) override;

  /*
   * @see IEvaluationService#RequestEvaluation(QString)
   */
  void RequestEvaluation(const QString& propertyName) override;

  //void UpdateShellKludge();

};

}

#endif // BERRYEVALUATIONSERVICE_H
