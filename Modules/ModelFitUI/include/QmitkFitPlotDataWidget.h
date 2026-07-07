/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkFitPlotDataWidget_h
#define QmitkFitPlotDataWidget_h

#include <mitkModelFitPlotDataHelper.h>

#include <MitkModelFitUIExports.h>

#include <QWidget>

#include <memory>

class QmitkFitPlotDataModel;

namespace Ui
{
  class QmitkFitPlotDataWidget;
}

/**
 * \class QmitkFitPlotDataWidget
 * \brief Widget that displays model fit plot data in a table and supports CSV export.
 *
 * This widget wraps a QmitkFitPlotDataModel in a table view, presenting the numerical
 * curve data of a ModelFitPlotData instance. It provides buttons to copy the table contents
 * to the system clipboard or to export them to a CSV file.
 *
 * \sa QmitkFitPlotDataModel
 * \sa mitk::ModelFitPlotData
 */
class MITKMODELFITUI_EXPORT QmitkFitPlotDataWidget : public QWidget
{
  Q_OBJECT

public:
  /** \brief Vector type holding const pointers to ModelFitInfo instances. */
  using FitVectorType = std::vector<mitk::modelFit::ModelFitInfo::ConstPointer>;

  /**
   * \brief Constructs a QmitkFitPlotDataWidget.
   * \param[in] parent Optional parent widget.
   */
  QmitkFitPlotDataWidget(QWidget* parent = nullptr);

  /** \brief Destructor. */
  ~QmitkFitPlotDataWidget() override;

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

public Q_SLOTS:

  /**
   * \brief Sets the plot data to display in the table.
   * \param[in] data Pointer to the ModelFitPlotData to display.
   */
  void SetPlotData(const mitk::ModelFitPlotData* data);

  /**
   * \brief Sets the display name for the x-axis column header.
   * \param[in] xName The name for the x-axis.
   */
  void SetXName(const std::string& xName);

protected Q_SLOTS:
  /**
   * \brief Exports the table data to a user-selected CSV file.
   *
   * Opens a file dialog for the user to choose a destination. Displays an error
   * message if the file cannot be created or opened.
   */
  void OnExportClicked() const;

  /**
   * \brief Copies the table contents as CSV text to the system clipboard.
   */
  void OnClipboardResultsButtonClicked() const;

protected:
  /**
   * \brief Serializes the internal model data to a CSV-formatted string.
   * \return The model content as a CSV string.
   */
  std::string StreamModelToString() const;

  std::unique_ptr<Ui::QmitkFitPlotDataWidget> m_Controls;
  QmitkFitPlotDataModel * m_InternalModel;
};

#endif
