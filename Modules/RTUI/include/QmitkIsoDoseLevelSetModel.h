/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkIsoDoseLevelSetModel_h
#define QmitkIsoDoseLevelSetModel_h

#include <QAbstractTableModel>

#include <mitkIsoDoseLevelCollections.h>

#include <MitkRTUIExports.h>


/**
 * \class QmitkIsoDoseLevelSetModel
 * \brief Table model for viewing and editing the contents of an mitk::IsoDoseLevelSet.
 *
 * This model exposes an IsoDoseLevelSet as a 4-column table with columns for:
 *   - Column 0: Color (Qt::EditRole returns QColor)
 *   - Column 1: Dose value (absolute in Gy or relative in %)
 *   - Column 2: Iso line visibility (boolean)
 *   - Column 3: Color wash visibility (boolean)
 *
 * Use the specialized delegates QmitkDoseColorDelegate, QmitkDoseValueDelegate,
 * and QmitkDoseVisualStyleDelegate for proper rendering and editing in views.
 *
 * \sa QmitkDoseColorDelegate
 * \sa QmitkDoseValueDelegate
 * \sa QmitkDoseVisualStyleDelegate
 * \sa mitk::IsoDoseLevelSet
 */
class MITKRTUI_EXPORT QmitkIsoDoseLevelSetModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  /**
   * \brief Constructor.
   * \param[in] parent Optional parent QObject.
   */
  explicit QmitkIsoDoseLevelSetModel(QObject *parent = nullptr);

  /** \brief Destructor. */
  ~QmitkIsoDoseLevelSetModel() override {};

  /**
   * \brief Sets the iso dose level set managed by this model and resets the modified flag.
   * \param[in] pSet The IsoDoseLevelSet to display and edit. If nullptr, no action is taken.
   */
  void setIsoDoseLevelSet(mitk::IsoDoseLevelSet *pSet);

  /**
   * \brief Returns item flags for the given index.
   * \param[in] index The model index to query.
   * \return The Qt::ItemFlags for the given index.
   */
  Qt::ItemFlags flags(const QModelIndex &index) const override;

  /**
   * \brief Returns data for the given index and role.
   * \param[in] index The model index to query.
   * \param[in] role The Qt role for the data request.
   * \return The requested data as QVariant.
   */
  QVariant data(const QModelIndex &index, int role) const override;

  /**
   * \brief Returns header data for the given section, orientation, and role.
   * \param[in] section The column section index.
   * \param[in] orientation The header orientation (only horizontal is supported).
   * \param[in] role The Qt role for the data request.
   * \return The header text as QVariant.
   */
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  /**
   * \brief Returns the number of iso dose levels in the set.
   * \param[in] parent The parent index (must be invalid for table models).
   * \return The number of rows (iso dose levels).
   */
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

  /**
   * \brief Returns the number of columns (always 4).
   * \param[in] parent The parent index.
   * \return 4 (color, dose value, iso line visibility, color wash visibility).
   */
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  /**
   * \brief Sets data at the given index for the given role.
   *
   * Modifies the underlying IsoDoseLevelSet by cloning the affected level,
   * updating the changed field, and reinserting it (which re-sorts the set).
   *
   * \param[in] index The model index to modify.
   * \param[in] value The new value as QVariant.
   * \param[in] role The Qt role (only Qt::EditRole is handled).
   * \return True if the data was successfully set.
   */
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

  /**
   * \brief Returns whether dose values are displayed as absolute values.
   * \return True if absolute dose display mode is active.
   */
  bool getShowAbsoluteDose() const;

  /**
   * \brief Returns the current reference dose.
   * \return The reference dose in Gy.
   */
  mitk::DoseValueAbs getReferenceDose() const;

  /**
   * \brief Returns whether the model allows editing only visibility columns.
   * \return True if dose value and color columns are read-only.
   */
  bool getVisibilityEditOnly() const;

  /**
   * \brief Sets the iso line visibility for all levels.
   * \param[in] activate If true, all iso lines are made visible; if false, all are hidden.
   */
  void switchVisibilityIsoLines(bool activate);

  /**
   * \brief Sets the color wash visibility for all levels.
   * \param[in] activate If true, all color washes are made visible; if false, all are hidden.
   */
  void switchVisibilityColorWash(bool activate);

  /** \brief Inverts the iso line visibility of all levels. */
  void invertVisibilityIsoLines();

  /** \brief Inverts the color wash visibility of all levels. */
  void invertVisibilityColorWash();

  /** \brief Swaps the iso line and color wash visibility for all levels. */
  void swapVisibility();

  /**
   * \brief Adds a new iso dose level at the end of the set.
   *
   * The new level has a dose value slightly above the current maximum
   * (or 0.01 if the set is empty), default color, and both visibility flags set to true.
   */
  void addLevel();

  /**
   * \brief Deletes the iso dose level at the given model index.
   * \param[in] index The model index of the level to delete. Invalid indices are ignored.
   */
  void deleteLevel(const QModelIndex &index);

  /**
   * \brief Checks whether the model content has been modified since the last setIsoDoseLevelSet() call.
   * \return True if modifications have been made.
   */
  bool isModified();

  public Q_SLOTS:
    /**
     * \brief Sets the reference dose used for absolute/relative conversion.
     * \param[in] newReferenceDose The new reference dose in Gy. Must be positive.
     * \throw mitk::Exception if newReferenceDose is zero or negative.
     */
    void setReferenceDose(double newReferenceDose);

    /**
     * \brief Sets whether dose values should be displayed as absolute (Gy) or relative (%).
     * \param[in] showAbsoluteDose If true, values are shown in Gy; otherwise in percent.
     */
    void setShowAbsoluteDose(bool showAbsoluteDose);

    /**
     * \brief Sets whether the model allows editing only the visibility columns.
     * \param[in] onlyVisibility If true, dose value and color columns become read-only.
     */
    void setVisibilityEditOnly(bool onlyVisibility);

private:
  mitk::IsoDoseLevelSet::Pointer m_DoseSet;

  bool m_showAbsoluteDose;
  bool m_visibilityEditOnly;

  mitk::DoseValueAbs m_referenceDose;

  /** Indicates if the data of the model was modified, since the model was set. */
  bool m_modified;

};

#endif
