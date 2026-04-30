/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkFitParameterModel_h
#define QmitkFitParameterModel_h

#include <QAbstractTableModel>

#include <mitkModelFitInfo.h>
#include <mitkPointSet.h>

#include <MitkModelFitUIExports.h>


/**
 * \class QmitkFitParameterModel
 * \brief Qt table model for displaying parameter values of model fit results at various world positions.
 *
 * This model presents the parameter values of one or more ModelFitInfo instances evaluated at
 * the current world coordinate position and optional bookmark positions. When a single fit is
 * provided, the model displays a flat list of parameters. When multiple fits are present, the
 * model uses a tree hierarchy where the first level represents fits and the second level
 * represents the parameters of each fit.
 *
 * Columns are organized as: parameter name, parameter type, value at the current position,
 * and one additional column per bookmark position.
 *
 * \sa QmitkFitParameterWidget
 * \sa mitk::modelFit::ModelFitInfo
 */
class MITKMODELFITUI_EXPORT QmitkFitParameterModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  /** \brief Vector type holding const pointers to ModelFitInfo instances. */
  using FitVectorType = std::vector<mitk::modelFit::ModelFitInfo::ConstPointer>;

  /**
   * \brief Constructs a QmitkFitParameterModel.
   * \param[in] parent Optional parent QObject.
   */
  QmitkFitParameterModel(QObject* parent = nullptr);

  /** \brief Destructor. */
  ~QmitkFitParameterModel() override {};

  /**
   * \brief Returns the currently set model fit info instances.
   * \return Const reference to the vector of ModelFitInfo pointers.
   */
  const FitVectorType& getFits() const;

  /**
   * \brief Returns the current world coordinate position used for value lookup.
   * \return The current 3D position.
   */
  mitk::Point3D getCurrentPosition() const;

  /**
   * \brief Returns the point set used as position bookmarks.
   * \return Pointer to the position bookmark PointSet, or nullptr if none is set.
   */
  const mitk::PointSet* getPositionBookmarks() const;

  /**
   * \brief Returns item flags for the given model index.
   * \param[in] index The model index to query.
   * \return The item flags (read-only by default).
   */
  Qt::ItemFlags flags(const QModelIndex& index) const override;

  /**
   * \brief Returns data for the given index and role.
   *
   * Provides parameter names (column 0), parameter types (column 1), values at the current
   * position (column 2), and values at bookmark positions (columns 3+). Static parameters
   * are displayed with comma-separated values. When multiple fits are set, the top-level
   * rows show fit name and UID.
   *
   * \param[in] index The model index to query.
   * \param[in] role The Qt item data role.
   * \return The requested data as QVariant, or an invalid QVariant if unavailable.
   */
  QVariant data(const QModelIndex& index, int role) const override;

  /**
   * \brief Returns header data for the specified section, orientation, and role.
   *
   * Horizontal headers are: "Name", "Type", "Value", and positional headers for each bookmark
   * showing coordinates.
   *
   * \param[in] section The column index.
   * \param[in] orientation The header orientation (horizontal or vertical).
   * \param[in] role The Qt item data role.
   * \return The header data as QVariant.
   */
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  /**
   * \brief Returns the number of rows in the model.
   *
   * For a single fit, returns the number of parameters plus static parameters.
   * For multiple fits, the top-level row count equals the number of fits, and child
   * row counts equal the parameter counts of each respective fit.
   *
   * \param[in] parent The parent model index.
   * \return The row count.
   */
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;

  /**
   * \brief Returns the number of columns in the model.
   *
   * Always returns 3 (name, type, current value) plus the number of bookmark positions.
   *
   * \param[in] parent The parent model index.
   * \return The column count.
   */
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  /**
   * \brief Sets data for a given index (currently always returns false; model is read-only).
   * \param[in] index The model index.
   * \param[in] value The value to set.
   * \param[in] role The Qt item data role.
   * \return Always false.
   */
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

public Q_SLOTS:

  /**
   * \brief Sets the model fit info instances to display.
   *
   * Triggers a full model reset.
   *
   * \param[in] fits Vector of ModelFitInfo pointers to display.
   */
  void setFits(const FitVectorType& fits);

  /**
   * \brief Sets the current world coordinate position for parameter value lookup.
   *
   * Triggers a full model reset so that values are re-read at the new position.
   *
   * \param[in] currentPos The new 3D world position.
   */
  void setCurrentPosition(const mitk::Point3D& currentPos);

  /**
   * \brief Sets the point set used as position bookmarks.
   *
   * Each bookmark adds an additional column to the model showing parameter values
   * at that position. Triggers a full model reset.
   *
   * \param[in] bookmarks Pointer to a PointSet of bookmark positions, or nullptr to clear.
   */
  void setPositionBookmarks(const mitk::PointSet* bookmarks);


protected:

  std::size_t getBookmarksCount() const;

private:

  bool hasSingleFit() const;

  FitVectorType m_Fits;
  mitk::PointSet::ConstPointer m_Bookmarks;
  mitk::Point3D m_CurrentPos;

};

#endif
