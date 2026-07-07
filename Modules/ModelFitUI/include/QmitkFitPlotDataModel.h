/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkFitPlotDataModel_h
#define QmitkFitPlotDataModel_h

#include <QAbstractTableModel>

#include <mitkModelFitPlotDataHelper.h>
#include <MitkModelFitUIExports.h>

/**
 * \class QmitkFitPlotDataModel
 * \brief Qt table model for displaying the curve values of a ModelFitPlotData instance.
 *
 * This model presents the numerical data behind model fit plot curves in a tabular form.
 * Column 0 is the sample index, column 1 is the x-axis value, and subsequent columns
 * correspond to individual plot curves (current position, positional, and static plots).
 * Interpolated signal curves are automatically excluded from the column layout.
 *
 * \sa QmitkFitPlotDataWidget
 * \sa mitk::ModelFitPlotData
 */
class MITKMODELFITUI_EXPORT QmitkFitPlotDataModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  /** \brief Vector type holding const pointers to ModelFitInfo instances. */
  using FitVectorType = std::vector<mitk::modelFit::ModelFitInfo::ConstPointer>;

  /**
   * \brief Constructs a QmitkFitPlotDataModel.
   * \param[in] parent Optional parent QObject.
   */
  QmitkFitPlotDataModel(QObject* parent = nullptr);

  /** \brief Destructor. */
  ~QmitkFitPlotDataModel() override {};

  /**
   * \brief Returns a pointer to the currently stored plot data.
   * \return Pointer to the internal ModelFitPlotData instance.
   */
  const mitk::ModelFitPlotData* GetPlotData() const;

  /**
   * \brief Returns the display name for the x-axis.
   * \return Const reference to the x-axis name string.
   */
  const std::string& GetXName() const;

  /**
   * \brief Returns item flags for the given model index (read-only).
   * \param[in] index The model index to query.
   * \return The item flags.
   */
  Qt::ItemFlags flags(const QModelIndex& index) const override;

  /**
   * \brief Returns data for the given index and role.
   *
   * Column 0 returns the row index, column 1 returns the x-axis value, and remaining
   * columns return the y-value of the corresponding curve at the given row.
   *
   * \param[in] index The model index to query.
   * \param[in] role The Qt item data role.
   * \return The requested data as QVariant.
   */
  QVariant data(const QModelIndex& index, int role) const override;

  /**
   * \brief Returns header data for the specified section.
   *
   * Column 0 header is "#", column 1 is the x-axis name, and remaining columns
   * show the curve name optionally annotated with the positional collection name.
   *
   * \param[in] section The column index.
   * \param[in] orientation The header orientation.
   * \param[in] role The Qt item data role.
   * \return The header data as QVariant.
   */
  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

  /**
   * \brief Returns the number of data points (rows) in the plot data.
   *
   * All curves are assumed to have the same number of data points.
   *
   * \param[in] parent The parent model index.
   * \return The number of rows.
   */
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;

  /**
   * \brief Returns the number of columns.
   *
   * Equals 2 (index + x-axis) plus the number of non-interpolated curves across
   * current position, positional, and static plot collections.
   *
   * \param[in] parent The parent model index.
   * \return The number of columns. Returns 0 if parent is valid.
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
   * \brief Sets the plot data to display.
   *
   * Makes a deep copy of the provided data and triggers a full model reset.
   *
   * \param[in] data Pointer to the ModelFitPlotData to copy and display.
   */
  void SetPlotData(const mitk::ModelFitPlotData* data);

  /**
   * \brief Sets the display name for the x-axis column header.
   *
   * Triggers a full model reset.
   *
   * \param[in] xName The name to display in the x-axis column header.
   */
  void SetXName(const std::string& xName);

protected:
  /**
   * \brief Retrieves the curve name and pointer for the given column index.
   *
   * Searches through current position plots, positional plots, and static plots
   * (skipping interpolated signal curves) to find the curve matching the column.
   *
   * \param[in] col The column index (0-based, accounting for index and x-axis columns).
   * \return A pair of the curve name and a pointer to the PlotDataCurve.
   */
  std::pair<std::string, const mitk::PlotDataCurve*> GetCurveByColumn(int col) const;

  /**
   * \brief Determines whether the given curve belongs to a positional collection.
   * \param[in] curve Pointer to the curve to look up.
   * \return A pair where the first element indicates whether the curve was found in a
   *         positional collection, and the second element is the positional collection entry.
   */
  std::pair<bool, mitk::ModelFitPlotData::PositionalCollectionMap::value_type> GetPositionalCurvePoint(const mitk::PlotDataCurve* curve) const;

private:
  mitk::ModelFitPlotData m_PlotData;
  std::string m_XName;

};

#endif
