/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkSimpleBarrierManagerWidget_h
#define QmitkSimpleBarrierManagerWidget_h

#include <MitkModelFitUIExports.h>

#include <QWidget>

#include <mitkSimpleBarrierConstraintChecker.h>

/*forward declarations*/
class QmitkSimpleBarrierParametersDelegate;
class QmitkSimpleBarrierTypeDelegate;
class QmitkSimpleBarrierModel;

namespace Ui
{
  class QmitkSimpleBarrierManagerWidget;
}

/**
 * \class QmitkSimpleBarrierManagerWidget
 * \brief Widget for viewing and editing the barrier constraints of a SimpleBarrierConstraintChecker.
 *
 * This widget presents the constraints of a mitk::SimpleBarrierConstraintChecker in a table
 * view. Users can add and remove constraints via buttons or a context menu. Each constraint
 * row shows the affected parameters, constraint type (upper/lower), barrier value, and
 * penalty zone width. Editing is handled through dedicated delegates for parameter selection
 * and type selection.
 *
 * \sa QmitkSimpleBarrierModel
 * \sa QmitkSimpleBarrierTypeDelegate
 * \sa QmitkSimpleBarrierParametersDelegate
 * \sa mitk::SimpleBarrierConstraintChecker
 */
class MITKMODELFITUI_EXPORT QmitkSimpleBarrierManagerWidget : public QWidget
{
  Q_OBJECT

public:
  /**
   * \brief Constructs a QmitkSimpleBarrierManagerWidget.
   * \param[in] parent Optional parent widget.
   */
  QmitkSimpleBarrierManagerWidget(QWidget* parent = nullptr);

  /** \brief Destructor. Deletes the internal model and delegates. */
  ~QmitkSimpleBarrierManagerWidget() override;

signals:
  /**
   * \brief Emitted when a constraint is changed.
   * \param[in] constraint The modified constraint.
   */
  void ConstraintChanged(mitk::SimpleBarrierConstraintChecker::Constraint constraint);

public Q_SLOTS:
  /**
   * \brief Sets the constraint checker and parameter metadata to manage.
   *
   * Configures the internal model with the provided checker, parameter names, and units.
   * The modified flag is reset.
   *
   * \param[in] pChecker Pointer to the SimpleBarrierConstraintChecker to manage.
   *            If nullptr, a new empty checker is created internally.
   * \param[in] names List of all parameter names. The index corresponds to the parameter
   *            index in the fitting model.
   * \param[in] units Map of parameter names to their unit strings.
   */
  void setChecker(mitk::SimpleBarrierConstraintChecker* pChecker,
                  const mitk::ModelTraitsInterface::ParameterNamesType& names,
                  const mitk::ModelTraitsInterface::ParamterUnitMapType& units);

protected Q_SLOTS:
  /**
   * \brief Shows a context menu with options to add or delete constraints.
   * \param[in] pos The position in widget coordinates where the menu was requested.
   */
  void OnShowContextMenuIsoSet(const QPoint& pos);

  /**
   * \brief Adds a new default constraint to the checker.
   * \param[in] checked The checked state of the triggering action (unused).
   */
  void OnAddConstraint(bool checked);

  /**
   * \brief Deletes the currently selected constraint from the checker.
   *
   * If no index is selected, attempts to delete the constraint at position (1,1).
   *
   * \param[in] checked The checked state of the triggering action (unused).
   */
  void OnDelConstraint(bool checked);

protected:

  /**
   * \brief Updates the widget according to its current settings.
   */
  void update();

  mitk::SimpleBarrierConstraintChecker::Pointer m_Checker;
  mitk::ModelTraitsInterface::ParameterNamesType m_ParameterNames;
  mitk::ModelTraitsInterface::ParamterUnitMapType m_ParameterUnits;


  QmitkSimpleBarrierModel* m_InternalModel;
  QmitkSimpleBarrierTypeDelegate* m_TypeDelegate;
  QmitkSimpleBarrierParametersDelegate* m_ParametersDelegate;

  bool m_InternalUpdate;

  std::unique_ptr<Ui::QmitkSimpleBarrierManagerWidget> m_Controls;

};

#endif
