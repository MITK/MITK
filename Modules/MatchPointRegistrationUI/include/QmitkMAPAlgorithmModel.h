/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMAPAlgorithmModel_h
#define QmitkMAPAlgorithmModel_h

#include <QAbstractTableModel>
#include <QStringList>

// MITK
#include <MitkMatchPointRegistrationUIExports.h>

// MatchPoint
#include <mapMetaPropertyAlgorithmInterface.h>
#include <mapRegistrationAlgorithmBase.h>

/**
 * \class QmitkMAPAlgorithmModel
 * \brief Qt table model exposing the meta-properties of a MatchPoint registration algorithm.
 *
 * This model wraps the MetaPropertyAlgorithmInterface of a MatchPoint algorithm and
 * presents its configurable properties as a two-column table (Property / Value). An
 * algorithm can be set as data source; the model retrieves all information through
 * the MetaPropertyInterface. Editable properties (writable meta-properties) can be
 * modified directly through the model, and changes are propagated back into the algorithm.
 *
 * Supported meta-property types for display and editing: bool, int, unsigned int, long,
 * unsigned long, float, double, and map::core::String. Unsupported types are shown with
 * an error message.
 *
 * \note The model stores only a raw pointer to the MetaPropertyAlgorithmInterface.
 *   You must call SetAlgorithm(nullptr) or set a new algorithm before the previously
 *   set algorithm is destroyed.
 *
 * \sa QmitkAlgorithmSettingsConfig, QmitkMapPropertyDelegate
 */
class MITKMATCHPOINTREGISTRATIONUI_EXPORT QmitkMAPAlgorithmModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  /**
   * \brief Constructs the algorithm model.
   * \param[in] parent Optional parent QObject for Qt memory management.
   */
  QmitkMAPAlgorithmModel(QObject *parent = nullptr);

  /** \brief Destructor. */
  ~QmitkMAPAlgorithmModel() override{};

  /**
   * \brief Sets the algorithm whose meta-properties should be displayed.
   *
   * Attempts to cast the algorithm to MetaPropertyAlgorithmInterface and delegates
   * to the overload accepting that interface.
   *
   * \param[in] pAlgorithm Pointer to the registration algorithm. May be \c nullptr to clear.
   */
  void SetAlgorithm(map::algorithm::RegistrationAlgorithmBase *pAlgorithm);

  /**
   * \brief Sets the meta-property interface directly as data source.
   *
   * Resets the model and refreshes the property list from the given interface.
   *
   * \param[in] pMetaInterface Pointer to the meta-property interface. May be \c nullptr to clear.
   */
  void SetAlgorithm(map::algorithm::facet::MetaPropertyAlgorithmInterface *pMetaInterface);

  /**
   * \brief Returns item flags for the given model index.
   *
   * Items in the value column (column 1) are editable if the corresponding meta-property
   * is writable and its type is supported.
   *
   * \param[in] index The model index to query.
   * \return Item flags including Qt::ItemIsEditable for writable properties.
   */
  Qt::ItemFlags flags(const QModelIndex &index) const override;

  /**
   * \brief Returns data for the given model index and role.
   *
   * Column 0 returns the property name (Qt::DisplayRole). Column 1 returns the property
   * value for Qt::DisplayRole and Qt::EditRole (if the property is readable/writable).
   *
   * \param[in] index The model index to query.
   * \param[in] role The data role.
   * \return A QVariant containing the requested data, or an invalid QVariant.
   */
  QVariant data(const QModelIndex &index, int role) const override;

  /**
   * \brief Returns header data for the table columns.
   *
   * Provides "Property" for column 0 and "Value" for column 1 (Qt::DisplayRole,
   * Qt::Horizontal).
   *
   * \param[in] section The column index.
   * \param[in] orientation The header orientation.
   * \param[in] role The data role.
   * \return A QVariant containing the header label, or an invalid QVariant.
   */
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  /**
   * \brief Returns the number of rows (meta-properties) in the model.
   * \param[in] parent The parent index. Must be invalid for a flat table model.
   * \return The number of meta-properties, or 0 if the parent is valid.
   */
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

  /**
   * \brief Returns the number of columns in the model.
   * \param[in] parent The parent index. Must be invalid for a flat table model.
   * \return Always 2 (Property, Value), or 0 if the parent is valid.
   */
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  /**
   * \brief Sets a property value at the given index.
   *
   * Only operates on column 1 with Qt::EditRole. Attempts to cast the QVariant value
   * to the matching meta-property type and writes it back to the algorithm. If successful,
   * the model is reset to reflect the change.
   *
   * \param[in] index The model index (must be in column 1).
   * \param[in] value The new value to set.
   * \param[in] role Must be Qt::EditRole.
   * \return True if the value was successfully set, false otherwise.
   */
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

private:
  void UpdateMetaProperties() const;

  /** Method uses m_pMetaInterface to retrieve the MetaProperty and unwraps it into an
   * suitable QVariant depending on the passed QT role. If the MetaProperty type is not supported, the QVariant is
   * invalid.
   */
  QVariant GetPropertyValue(const map::algorithm::MetaPropertyInfo *pInfo, int role) const;

  template <typename TValueType>
  bool CheckCastAndSetProp(const map::algorithm::MetaPropertyInfo *pInfo, const QVariant &value);

  bool SetPropertyValue(const map::algorithm::MetaPropertyInfo *pInfo, const QVariant &value);

  map::algorithm::facet::MetaPropertyAlgorithmInterface *m_pMetaInterface;
  mutable map::algorithm::facet::MetaPropertyAlgorithmInterface::MetaPropertyVectorType m_MetaProperties;
};

#endif
