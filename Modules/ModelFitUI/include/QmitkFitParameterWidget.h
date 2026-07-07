/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkFitParameterWidget_h
#define QmitkFitParameterWidget_h

#include <mitkModelFitInfo.h>
#include <mitkPointSet.h>

#include <MitkModelFitUIExports.h>

#include <QWidget>
#include <memory>

namespace Ui
{
  class QmitkFitParameterWidget;
}

class QmitkFitParameterModel;

/**
 * \class QmitkFitParameterWidget
 * \brief Widget that displays model fit parameter values at multiple world coordinate positions.
 *
 * This widget presents the parameters of all configured ModelFitInfo instances in a table view,
 * evaluated at the current world position and any additional bookmark positions. It provides
 * buttons to export the displayed data as CSV to the system clipboard or to a file.
 *
 * Internally, it delegates to a QmitkFitParameterModel for data handling.
 *
 * \sa QmitkFitParameterModel
 * \sa mitk::modelFit::ModelFitInfo
 */
class MITKMODELFITUI_EXPORT QmitkFitParameterWidget : public QWidget
{
  Q_OBJECT

public:
  /** \brief Vector type holding const pointers to ModelFitInfo instances. */
  using FitVectorType = std::vector<mitk::modelFit::ModelFitInfo::ConstPointer>;

  /**
   * \brief Constructs a QmitkFitParameterWidget.
   * \param[in] parent Optional parent widget.
   */
  QmitkFitParameterWidget(QWidget* parent = nullptr);

  /** \brief Destructor. */
  ~QmitkFitParameterWidget() override;

  /**
   * \brief Returns the currently configured model fit info instances.
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
   * \return Pointer to the bookmark PointSet, or nullptr if none is set.
   */
  const mitk::PointSet* getPositionBookmarks() const;

public Q_SLOTS:

  /**
   * \brief Sets the model fit info instances to display.
   * \param[in] fits Vector of ModelFitInfo pointers.
   */
  void setFits(const FitVectorType& fits);

  /**
   * \brief Sets the current world coordinate position for parameter value lookup.
   * \param[in] currentPos The new 3D world position.
   */
  void setCurrentPosition(const mitk::Point3D& currentPos);

  /**
   * \brief Sets the point set used as position bookmarks.
   * \param[in] bookmarks Pointer to a PointSet of bookmark positions, or nullptr to clear.
   */
  void setPositionBookmarks(const mitk::PointSet* bookmarks);

protected Q_SLOTS:
  /**
   * \brief Exports the parameter table to a user-selected CSV file.
   *
   * Opens a file dialog for the user to choose a destination. If the file cannot
   * be created or opened, an error message is shown.
   */
  void OnExportClicked() const;

  /**
   * \brief Copies the parameter table contents as CSV text to the system clipboard.
   */
  void OnClipboardResultsButtonClicked() const;

protected:
  /**
   * \brief Serializes the internal model data to a CSV-formatted string.
   * \return The model content as a CSV string.
   */
  std::string streamModelToString() const;

  QmitkFitParameterModel * m_InternalModel;

  std::unique_ptr<Ui::QmitkFitParameterWidget> m_Controls;

};

/**
 * \brief Sanitizes a string for safe use in CSV export.
 *
 * Replaces commas, newlines, and carriage returns with spaces to prevent
 * malformed CSV output.
 *
 * \param[in] str The input string to sanitize.
 * \return A sanitized copy of the string.
 *
 * \note This function is declared in the header so it can be reused by other ModelFitUI widgets.
 */
std::string SanatizeString(std::string str);

#endif
