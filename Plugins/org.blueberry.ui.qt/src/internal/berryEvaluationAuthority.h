/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYEVALUATIONAUTHORITY_H
#define BERRYEVALUATIONAUTHORITY_H

#include "berryExpressionAuthority.h"

#include <berryIPropertyChangeListener.h>

#include <QHash>

namespace berry {

struct IEvaluationReference;

class EvaluationReference;
class Expression;
class Shell;

class EvaluationAuthority : public ExpressionAuthority
{

public:

  berryObjectMacro(berry::EvaluationAuthority);

private:

  static const QString COMPONENT;

  typedef QHash<SmartPointer<Expression>, QSet<SmartPointer<EvaluationReference> > > ExprToEvalsMapType;

  /**
   * A bucket sort of the evaluation references based on source priority. Each
   * reference will appear only once per set, but may appear in multiple sets.
   * If no references are defined for a particular priority level, then the
   * array at that index will only contain <code>null</code>.
   */
  QHash<QString,ExprToEvalsMapType> cachesBySourceName;
  IPropertyChangeListener::Events serviceListeners;
  MessageExceptionHandler<EvaluationAuthority> serviceExceptionHandler;
  int notifying;

  // private final Map cachesByExpression = new HashMap();

  QStringList GetNames(const SmartPointer<IEvaluationReference>& ref) const;

  /**
   * This will evaluate all refs with the same expression.
   *
   * @param refs
   */
  void RefsWithSameExpression(const QList<SmartPointer<EvaluationReference> >& refs);

  /**
   * @param sourceNames
   */
  void StartSourceChange(const QStringList& sourceNames);

  /**
   * @param sourceNames
   */
  void EndSourceChange(const QStringList& sourceNames);

  /**
   * @param ref
   * @param oldValue
   * @param newValue
   */
  void FirePropertyChange(const SmartPointer<IEvaluationReference>& ref,
                          Object::Pointer oldValue, Object::Pointer newValue);

  void FireServiceChange(const QString& property, Object::Pointer oldValue, Object::Pointer newValue);

  void ServiceChangeException(const std::exception& exc);

  Object::Pointer ValueOf(bool result);

protected:

  /**
   * Returns the currently active shell.
   *
   * @return The currently active shell; may be <code>null</code>.
   */
  SmartPointer<const Shell> GetActiveShell() const;

public:

  EvaluationAuthority();

  /*
   * @see ExpressionAuthority#SourceChanged(int)
   */
  void SourceChanged(int sourcePriority) override;

  /*
   * @see ExpressionAuthority#SourceChanged(QString)
   */
  void SourceChanged(const QStringList& sourceNames) override;

  using ExpressionAuthority::SourceChanged;

  void AddEvaluationListener(const SmartPointer<IEvaluationReference>& ref);

  /**
   * @param ref
   */
  void RemoveEvaluationListener(const SmartPointer<IEvaluationReference>& ref);

  /**
   * @param listener
   */
  void AddServiceListener(IPropertyChangeListener* listener);

  /**
   * @param listener
   */
  void RemoveServiceListener(IPropertyChangeListener* listener);

};

}

#endif // BERRYEVALUATIONAUTHORITY_H
