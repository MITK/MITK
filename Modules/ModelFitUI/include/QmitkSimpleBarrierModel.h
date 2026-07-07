/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSimpleBarrierModel_h
#define QmitkSimpleBarrierModel_h

#include <QAbstractTableModel>

#include <mitkSimpleBarrierConstraintChecker.h>
#include <mitkModelTraitsInterface.h>

#include <MitkModelFitUIExports.h>


/**
 * \class QmitkSimpleBarrierModel
 * \brief Qt table model for viewing and editing the constraints of a SimpleBarrierConstraintChecker.
 *
 * This model adapts a mitk::SimpleBarrierConstraintChecker for use with a Qt table view.
 * Each row represents a single constraint, with columns for: affected parameters, constraint
 * type (upper/lower), barrier threshold value, penalty zone width, and parameter units.
 * All columns except the unit column are editable.
 *
 * \sa QmitkSimpleBarrierManagerWidget
 * \sa QmitkSimpleBarrierTypeDelegate
 * \sa QmitkSimpleBarrierParametersDelegate
 * \sa mitk::SimpleBarrierConstraintChecker
 */
class MITKMODELFITUI_EXPORT QmitkSimpleBarrierModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  /**
   * \brief Constructs a QmitkSimpleBarrierModel.
   *
   * Creates an empty internal SimpleBarrierConstraintChecker.
   *
   * \param[in] parent Optional parent QObject.
   */
  QmitkSimpleBarrierModel(QObject* parent = nullptr);

  /** \brief Destructor. */
  ~QmitkSimpleBarrierModel() override {};

  /**
   * \brief Sets the constraint checker and parameter metadata managed by this model.
   *
   * If the provided checker differs from the current one, the model is reset. The
   * modified flag is cleared. If pChecker is nullptr, a new empty checker is created.
   *
   * \param[in] pChecker Pointer to the SimpleBarrierConstraintChecker to manage.
   * \param[in] names List of parameter names. The index corresponds to the parameter
   *            index in the fitting model.
   * \param[in] units Map of parameter names to their unit strings.
   */
  void setChecker(mitk::SimpleBarrierConstraintChecker* pChecker,
                  const mitk::ModelTraitsInterface::ParameterNamesType& names,
                  const mitk::ModelTraitsInterface::ParamterUnitMapType units);

  /**
   * \brief Returns item flags for the given model index.
   *
   * Columns 0 through 3 (parameters, type, value, width) are editable.
   * Column 4 (unit) is read-only.
   *
   * \param[in] index The model index to query.
   * \return The item flags.
   */
  Qt::ItemFlags flags(const QModelIndex& index) const override;

  /**
   * \brief Returns data for the given index and role.
   *
   * Column 0: affected parameter names (display: selected names; edit: all available names).
   * Column 1: constraint type ("upper" or "lower"). Column 2: barrier threshold value.
   * Column 3: penalty zone width. Column 4: units of the affected parameters.
   *
   * \param[in] index The model index to query.
   * \param[in] role The Qt item data role.
   * \return The requested data as QVariant.
   */
  QVariant data(const QModelIndex& index, int role) const override;

  /**
   * \brief Returns header data for the specified section.
   *
   * Horizontal headers are: "Parameters", "Type", "Value", "Width", "Unit".
   *
   * \param[in] section The column index.
   * \param[in] orientation The header orientation.
   * \param[in] role The Qt item data role.
   * \return The header data as QVariant.
   */
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  /**
   * \brief Returns the number of constraints (rows).
   * \param[in] parent The parent model index.
   * \return The number of constraints. Returns 0 if parent is valid.
   */
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;

  /**
   * \brief Returns the number of columns (always 5).
   * \param[in] parent The parent model index.
   * \return 5 (parameters, type, value, width, unit). Returns 0 if parent is valid.
   */
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  /**
   * \brief Sets data for a given index.
   *
   * Column 0: expects a QStringList of selected parameter names. Column 1: expects an
   * integer (0 = lower, 1 = upper). Column 2: expects a double barrier value. Column 3:
   * expects a double width value.
   *
   * \param[in] index The model index to modify.
   * \param[in] value The new value to set.
   * \param[in] role The Qt item data role (only Qt::EditRole is handled).
   * \return True if data was successfully modified, false otherwise.
   */
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

  /**
   * \brief Adds a new default lower barrier constraint for parameter 0 with barrier value 0.
   *
   * Triggers a full model reset and sets the modified flag.
   */
  void addConstraint();

  /**
   * \brief Deletes the constraint at the specified model index.
   *
   * Does nothing if the index is invalid or out of range. Triggers a full model
   * reset and sets the modified flag.
   *
   * \param[in] index The model index of the constraint to delete.
   */
  void deleteConstraint(const QModelIndex& index);

  /**
   * \brief Indicates whether the model content has been modified since the last setChecker() call.
   * \return True if modified, false otherwise.
   */
  bool isModified();

private:
  mitk::SimpleBarrierConstraintChecker::Pointer m_Checker;
  mitk::ModelTraitsInterface::ParameterNamesType m_ParameterNames;
  mitk::ModelTraitsInterface::ParamterUnitMapType m_ParameterUnits;

  /** Indicates if the data of the model was modified, since the model was set. */
  bool m_modified;

};

#endif
