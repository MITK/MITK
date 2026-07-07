/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef QmitkInspectionPositionWidget_h
#define QmitkInspectionPositionWidget_h

#include <mitkModelFitInfo.h>
#include <mitkPointSet.h>

#include <MitkModelFitUIExports.h>

#include <QWidget>
#include <memory>

namespace Ui
{
  class QmitkInspectionPositionWidget;
}

class QmitkFitParameterModel;

/**
 * \class QmitkInspectionPositionWidget
 * \brief Widget for managing world coordinate positions used to inspect model fit results.
 *
 * This widget displays the current cross-hair position and maintains a list of bookmark
 * positions. Users can add the current position to the bookmark list via a button. The
 * bookmark points are stored in a mitk::PointSet associated with a data node. Changes to
 * the bookmark list are signaled via PositionBookmarksChanged().
 *
 * \sa QmitkFitParameterWidget
 * \sa QmitkFitParameterModel
 */
class MITKMODELFITUI_EXPORT QmitkInspectionPositionWidget : public QWidget
{
  Q_OBJECT

public:
  /** \brief Vector type holding const pointers to ModelFitInfo instances. */
  using FitVectorType = std::vector<mitk::modelFit::ModelFitInfo::ConstPointer>;

  /**
   * \brief Constructs a QmitkInspectionPositionWidget.
   * \param[in] parent Optional parent widget.
   */
  QmitkInspectionPositionWidget(QWidget* parent = nullptr);

  /** \brief Destructor. */
  ~QmitkInspectionPositionWidget() override;

  /**
   * \brief Returns the current world coordinate position.
   * \return The current 3D position.
   */
  mitk::Point3D GetCurrentPosition() const;

  /**
   * \brief Returns the point set containing all bookmark positions.
   * \return Pointer to the bookmark PointSet, or nullptr if no bookmark node is set.
   */
  const mitk::PointSet* GetPositionBookmarks() const;

  /**
   * \brief Assigns a data node containing a point set for position bookmarks.
   *
   * The widget observes the point set within this node. The "Add" button is enabled
   * only when a valid node is set.
   *
   * \param[in] newNode Pointer to the data node holding the bookmark PointSet.
   *                    Pass nullptr to disable bookmark functionality.
   */
  void SetPositionBookmarkNode(mitk::DataNode *newNode);

  /**
   * \brief Returns the data node currently assigned for position bookmarks.
   * \return Pointer to the bookmark data node, or nullptr if none is assigned.
   */
  mitk::DataNode *GetPositionBookmarkNode();

public Q_SLOTS:

  /**
   * \brief Updates the displayed current position.
   * \param[in] currentPos The new 3D world position to display.
   */
  void SetCurrentPosition(const mitk::Point3D& currentPos);

  /**
   * \brief Slot called when the internal point list widget reports changes.
   *
   * Emits PositionBookmarksChanged().
   */
  void OnPointListChanged();

  /**
   * \brief Adds the current position to the bookmark point set.
   *
   * Inserts the current position into the PointSet and emits PositionBookmarksChanged().
   * Does nothing if no bookmark PointSet is available.
   */
  void OnAddCurrentPositionClicked();

Q_SIGNALS:
  /** \brief Emitted whenever the position bookmark list is modified. */
  void PositionBookmarksChanged();

protected:
  std::unique_ptr<Ui::QmitkInspectionPositionWidget> m_Controls;
  mitk::Point3D m_CurrentPosition;
};

#endif
