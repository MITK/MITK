/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkSimpleBarrierManagerWidget_h
#define QmitkSimpleBarrierManagerWidget_h

#include "MitkModelFitUIExports.h"

#include "ui_QmitkSimpleBarrierManagerWidget.h"
#include <QWidget>

#include "mitkSimpleBarrierConstraintChecker.h"

/*forward declarations*/
class QmitkSimpleBarrierParametersDelegate;
class QmitkSimpleBarrierTypeDelegate;
class QmitkSimpleBarrierModel;

/**
* \class QmitkSimpleBarrierManagerWidget
* \brief Widget that allows to edit the constraints of SimpleBarrierConstraintChecker.
*/
class MITKMODELFITUI_EXPORT QmitkSimpleBarrierManagerWidget : public QWidget
{
  Q_OBJECT

public:
  QmitkSimpleBarrierManagerWidget(QWidget* parent = nullptr);
  ~QmitkSimpleBarrierManagerWidget() override;

signals:
  void ConstraintChanged(mitk::SimpleBarrierConstraintChecker::Constraint constraint);

public Q_SLOTS:
  /** Sets the data handled by the model and resets the modified flag
   @param pChecker Pointer to the checker instance that should be managed.
   @param names List of all possible parameter names. It is assumed that the
   index of the list equals the parameter index in the respective fitting model.*/
  void setChecker(mitk::SimpleBarrierConstraintChecker* pChecker,
                  const mitk::ModelTraitsInterface::ParameterNamesType& names);

protected Q_SLOTS:
  void OnShowContextMenuIsoSet(const QPoint& pos);
  void OnAddConstraint(bool checked);
  void OnDelConstraint(bool checked);

protected:

  /**
  * \brief Updates the widget according to its current settings.
  */
  void update();

  mitk::SimpleBarrierConstraintChecker::Pointer m_Checker;
  mitk::ModelTraitsInterface::ParameterNamesType m_ParameterNames;

  QmitkSimpleBarrierModel* m_InternalModel;
  QmitkSimpleBarrierTypeDelegate* m_TypeDelegate;
  QmitkSimpleBarrierParametersDelegate* m_ParametersDelegate;

  bool m_InternalUpdate;

  Ui::QmitkSimpleBarrierManagerWidget m_Controls;

};

#endif
