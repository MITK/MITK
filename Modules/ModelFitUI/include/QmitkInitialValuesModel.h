/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkInitialValuesModel_h
#define QmitkInitialValuesModel_h

#include <QAbstractTableModel>

#include <mitkSimpleBarrierConstraintChecker.h>
#include <mitkModelTraitsInterface.h>
#include <mitkDataNode.h>
#include <mitkInitialParameterizationDelegateBase.h>

#include <MitkModelFitUIExports.h>


/**
 * \class QmitkInitialValuesModel
 * \brief Qt table model for defining and editing initial parameter values of a model fit.
 *
 * This model manages the initial values used to start a model fitting process. Each parameter
 * can be initialized with either a simple scalar value or an image-based value source. The
 * model provides four columns: parameter name, value type (scalar/image), value, and unit.
 *
 * The type and value columns are editable. When the type is set to "image", the value
 * column displays the name of the selected data node rather than a numeric value.
 *
 * \sa QmitkInitialValuesManagerWidget
 * \sa QmitkInitialValuesDelegate
 * \sa QmitkInitialValuesTypeDelegate
 * \sa mitk::InitialParameterizationDelegateBase
 */
class MITKMODELFITUI_EXPORT QmitkInitialValuesModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  /**
   * \brief Constructs a QmitkInitialValuesModel.
   * \param[in] parent Optional parent QObject.
   */
  QmitkInitialValuesModel(QObject* parent = nullptr);

  /** \brief Destructor. */
  ~QmitkInitialValuesModel() override {};

  /**
   * \brief Sets the parameter names, default values, and units.
   *
   * Triggers a full model reset. The modified flag is cleared.
   *
   * \param[in] names List of parameter names. The index corresponds to the parameter
   *            index in the fitting model.
   * \param[in] values Default initial values for the parameters.
   * \param[in] units Map of parameter names to their unit strings.
   *
   * \throw mitk::Exception if the sizes of names, values, and units do not match.
   */
  void setInitialValues(const mitk::ModelTraitsInterface::ParameterNamesType& names,
                        const mitk::ModelTraitsInterface::ParametersType values,
                        const mitk::ModelTraitsInterface::ParamterUnitMapType units);

  /**
   * \brief Sets the parameter names with all initial values defaulting to zero.
   *
   * Convenience overload that calls setInitialValues() with a zero-filled values vector
   * and empty unit strings.
   *
   * \param[in] names List of parameter names.
   */
  void setInitialValues(const mitk::ModelTraitsInterface::ParameterNamesType& names);

  /**
   * \brief Assigns an image as the initial value source for a specific parameter.
   *
   * \param[in] node Pointer to the data node containing the source image.
   * \param[in] paramIndex Index of the parameter to initialize with the image.
   *            Corresponds to the position in the vector defined by setInitialValues().
   *
   * \pre paramIndex must be within bounds of the initial parameterization vector.
   * \pre node must point to a valid mitk::Image instance.
   *
   * \throw mitk::Exception if node is nullptr or does not contain an image.
   *
   * \note Setting an image for an index overwrites any previously set scalar value
   *       for that parameter.
   */
  void addInitialParameterImage(const mitk::DataNode* node, mitk::ModelTraitsInterface::ParametersType::size_type paramIndex);

  /**
   * \brief Checks whether all initial values are valid.
   *
   * Returns false if any image-based parameter has a null data node pointer.
   *
   * \return True if all initial values are valid, false otherwise.
   */
  bool hasValidInitialValues() const;

  /**
   * \brief Clears all image-based parameter assignments.
   *
   * Triggers a full model reset. After this call, all parameters revert to
   * their scalar initial values.
   */
  void resetInitialParameterImage();

  /**
   * \brief Creates and returns a delegate representing the full initial parameterization.
   *
   * The returned delegate contains both scalar initial values and any image-based
   * parameterizations that have been configured.
   *
   * \return Smart pointer to an InitialParameterizationDelegateBase.
   */
  mitk::InitialParameterizationDelegateBase::Pointer getInitialParametrizationDelegate() const;

  /**
   * \brief Returns the current scalar initial values.
   *
   * \return The parameter values vector.
   *
   * \note This only returns scalar initial values. Image-based parameterizations are not
   *       included. Use getInitialParametrizationDelegate() to retrieve the complete
   *       parameterization.
   */
  mitk::ModelTraitsInterface::ParametersType getInitialValues() const;

  /**
   * \brief Returns item flags for the given model index.
   *
   * Columns 1 (type) and 2 (value) are editable; columns 0 (name) and 3 (unit)
   * are read-only.
   *
   * \param[in] index The model index to query.
   * \return The item flags.
   */
  Qt::ItemFlags flags(const QModelIndex& index) const override;

  /**
   * \brief Returns data for the given index and role.
   *
   * Column 0: parameter name. Column 1: type ("scalar" or "image"). Column 2: the
   * scalar value or the name of the selected image node. Column 3: the parameter unit.
   *
   * \param[in] index The model index to query.
   * \param[in] role The Qt item data role.
   * \return The requested data as QVariant.
   */
  QVariant data(const QModelIndex& index, int role) const override;

  /**
   * \brief Returns header data for the specified section.
   *
   * Horizontal headers are: "Parameters", "Type", "Value", "Unit".
   *
   * \param[in] section The column index.
   * \param[in] orientation The header orientation.
   * \param[in] role The Qt item data role.
   * \return The header data as QVariant.
   */
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  /**
   * \brief Returns the number of parameters (rows).
   * \param[in] parent The parent model index.
   * \return The number of rows. Returns 0 if parent is valid.
   */
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;

  /**
   * \brief Returns the number of columns (always 4).
   * \param[in] parent The parent model index.
   * \return 4 (name, type, value, unit). Returns 0 if parent is valid.
   */
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  /**
   * \brief Sets data for a given index.
   *
   * For column 1 (type), accepts an integer (0 = scalar, 1 = image) and switches
   * the value source accordingly. For column 2 (value), accepts a double for scalar
   * types or a void pointer to a mitk::DataNode for image types.
   *
   * \param[in] index The model index to modify.
   * \param[in] value The new value to set.
   * \param[in] role The Qt item data role (only Qt::EditRole is handled).
   * \return True if data was successfully set, false otherwise.
   */
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

  /**
   * \brief Indicates whether the model content has been modified since the last setInitialValues() call.
   * \return True if modified, false otherwise.
   */
  bool isModified();

private:
  int valueType(const QModelIndex& index) const;

  mitk::ModelTraitsInterface::ParametersType m_Values;
  mitk::ModelTraitsInterface::ParameterNamesType m_ParameterNames;
  mitk::ModelTraitsInterface::ParamterUnitMapType m_ParameterUnits;

  typedef std::map<mitk::ModelTraitsInterface::ParametersType::size_type, mitk::DataNode::ConstPointer> ImageMapType;
  ImageMapType m_ParameterImageMap;

  /** Indicates if the data of the model was modified, since the model was set. */
  bool m_modified;

};

#endif
