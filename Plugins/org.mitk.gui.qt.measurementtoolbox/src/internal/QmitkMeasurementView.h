/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMeasurementView_h
#define QmitkMeasurementView_h

#include <QmitkAbstractView.h>

#include <mitkIRenderWindowPartListener.h>

#include <memory>
#include <vector>

class QTreeWidgetItem;
struct QmitkMeasurementViewData;

namespace mitk
{
  class PlanarFigure;
  class TimeGeometry;
}

/**
 * \brief Places planar figures on a reference image and shows their measurements.
 *
 * Each tool places one figure, which is added below the reference image. Figures
 * that are not helper objects can be edited while the view is open.
 */
class QmitkMeasurementView : public QmitkAbstractView, public mitk::IRenderWindowPartListener
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkMeasurementView();
  ~QmitkMeasurementView() override;

  void CreateQtPartControl(QWidget* parent) override;
  void SetFocus() override {}

  /** Lets a press on a value row of the measurement tree act on its figure row. */
  bool eventFilter(QObject* watched, QEvent* event) override;

private:
  void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;
  void NodeAdded(const mitk::DataNode* node) override;
  void NodeChanged(const mitk::DataNode* node) override;
  void NodeRemoved(const mitk::DataNode* node) override;

  void RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartDeactivated(mitk::IRenderWindowPart* renderWindowPart) override;
  void RenderWindowPartInputChanged(mitk::IRenderWindowPart* renderWindowPart) override;

  void CreateToolButtons();
  void CreateConnections();

  void OnCurrentSelectionChanged(QList<mitk::DataNode::Pointer> nodes);
  void OnAlignViewsClicked();
  void OnToolClicked(int toolIndex);
  void OnFixedSizeToggled();
  void OnFixedSizeValueChanged();
  void OnColorButtonClicked();
  void OnAppearanceChanged();
  void OnApplyAppearanceClicked();

  /** Updates the controls that depend on the selected figures. */
  void OnSelectedFiguresChanged();

  /** The complete figures in the selection that can be edited in this view. */
  std::vector<mitk::DataNode*> GetSelectedFigures() const;
  void OnCopyToClipboard();

  /** Takes the appearance of new figures from the preferences, or else from the defaults of the mapper. */
  void LoadAppearance();
  void SaveAppearance() const;
  void ApplyAppearance(mitk::DataNode* node) const;
  void UpdateColorButton();
  void UpdateLineWidthLabel();
  float GetLineWidth() const;

  /**
   * Shows the alignment overlay and enables the drawing tools according to the
   * reference image selection and the alignment of the render windows. Cancels
   * a pending placement when the views are not aligned.
   */
  void UpdateDrawingControls();

  /** Shows the options of the active tool only. */
  void UpdateToolOptions();

  /** Shows how to place the pending figure, how to edit the selected one, or how to start. */
  void UpdateHint();

  /** Updates the measurement tree on the next turn of the event loop, once for a burst of changes. */
  void ScheduleMeasurementTreeUpdate();

  /** Lists the complete figures of the reference image, keeping their rows, expansion and selection. */
  void UpdateMeasurementTree();

  /** Selects the rows of the selected figures. */
  void UpdateMeasurementTreeSelection();

  /** Names what the copy button copies: the selected rows, or all of them. */
  void UpdateCopyButton();

  void OnMeasurementTreeSelectionChanged();
  void OnMeasurementTreeItemActivated(const QTreeWidgetItem* item);

  /** Moves the views to the figure, and turns the closest view into its plane if no view shows it. */
  void ShowFigure(const mitk::DataNode* node);

  std::vector<mitk::DataNode*> GetMeasuredFigures() const;

  /**
   * Forwards the interaction reference geometry to the render window part, if there
   * is one, and swallows the exception thrown for a time point the geometry does not
   * cover. Passing nullptr lifts the requirement.
   */
  void SetInteractionReferenceGeometry(const mitk::TimeGeometry* geometry);

  void StartPlacement(int toolIndex);
  void CancelPlacement();

  /** Resets the tools after a placement ended, was canceled or its figure got removed. */
  void ResetPlacement();

  /** Whether the figures of the tool are presets of the fixed size, placed by a single click. */
  bool IsPlacedWithFixedSize(int toolIndex) const;
  void ApplyFixedSize(mitk::PlanarFigure* figure, int toolIndex) const;
  void AddFigureToDataStorage(mitk::PlanarFigure* figure, const std::string& baseName);
  void SelectNode(mitk::DataNode* node);
  void SelectNodes(const QList<mitk::DataNode::Pointer>& nodes);

  void AddAllInteractors();
  mitk::DataStorage::SetOfObjects::ConstPointer GetAllPlanarFigures() const;

  std::unique_ptr<QmitkMeasurementViewData> d;
};

#endif
