/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAlgorithmListModel_h
#define QmitkAlgorithmListModel_h

#include <QAbstractTableModel>
#include <QStringList>

// MITK
#include <MitkMatchPointRegistrationUIExports.h>

// MatchPoint
#include <mapDeploymentDLLDirectoryBrowser.h>

/**
 * \class QmitkAlgorithmListModel
 * \brief Qt table model representing a list of MatchPoint registration algorithm DLL handles.
 *
 * This model wraps a list of MatchPoint algorithm DLLInfo objects and exposes them
 * as a four-column table suitable for use in any QAbstractItemView. The columns are:
 * - Column 0: Algorithm name
 * - Column 1: Algorithm namespace
 * - Column 2: Algorithm version
 * - Column 3: Keywords (semicolon-separated)
 *
 * The Qt::UserRole returns the row index of the item, which can be used to
 * retrieve the corresponding DLLInfo from the original list.
 *
 * \sa QmitkAlgorithmProfileViewer, QmitkAlgorithmSettingsConfig
 */
class MITKMATCHPOINTREGISTRATIONUI_EXPORT QmitkAlgorithmListModel /** \cond */ : public QAbstractTableModel /** \endcond */
{
  Q_OBJECT

public:
  /**
   * \brief Constructs the algorithm list model.
   * \param[in] parent Optional parent QObject for Qt memory management.
   */
  QmitkAlgorithmListModel(QObject *parent = nullptr);

  /** \brief Destructor. */
  ~QmitkAlgorithmListModel() override{};

  /**
   * \brief Sets the list of algorithm DLL info objects to be displayed.
   *
   * Replaces the current model content with the provided list and triggers
   * a full model reset.
   *
   * \param[in] algList List of MatchPoint DLLInfo smart pointers describing the available algorithms.
   */
  void SetAlgorithms(::map::deployment::DLLDirectoryBrowser::DLLInfoListType algList);

  /**
   * \brief Returns item flags for the given model index.
   * \param[in] index The model index to query.
   * \return Default item flags from QAbstractItemModel.
   */
  Qt::ItemFlags flags(const QModelIndex &index) const override;

  /**
   * \brief Returns data for the given model index and role.
   *
   * For Qt::DisplayRole, returns the algorithm name (column 0), namespace (column 1),
   * version (column 2), or keywords (column 3). For Qt::UserRole, returns the row index.
   *
   * \param[in] index The model index to query.
   * \param[in] role The data role (e.g., Qt::DisplayRole, Qt::UserRole).
   * \return A QVariant containing the requested data, or an invalid QVariant if the index is invalid.
   */
  QVariant data(const QModelIndex &index, int role) const override;

  /**
   * \brief Returns header data for the given section, orientation, and role.
   *
   * For horizontal headers with Qt::DisplayRole, returns "Name", "Namespace", "Version",
   * or "Keywords" for sections 0--3.
   *
   * \param[in] section The column index.
   * \param[in] orientation The header orientation (only Qt::Horizontal is handled).
   * \param[in] role The data role.
   * \return A QVariant containing the header label, or an invalid QVariant.
   */
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  /**
   * \brief Returns the number of rows (algorithms) in the model.
   * \param[in] parent The parent index. Must be invalid for a flat table model.
   * \return The number of algorithms, or 0 if the parent is valid.
   */
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

  /**
   * \brief Returns the number of columns in the model.
   * \param[in] parent The parent index. Must be invalid for a flat table model.
   * \return Always 4 (Name, Namespace, Version, Keywords), or 0 if the parent is valid.
   */
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

private:
  ::map::deployment::DLLDirectoryBrowser::DLLInfoListType m_AlgList;
};

#endif
