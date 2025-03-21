/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMultiLabelSelectionWidget_h
#define QmitkMultiLabelSelectionWidget_h

#include <MitkSegmentationUIExports.h>
#include <mitkWeakPointer.h>
#include <mitkLabelSetImage.h>
#include <mitkDataNode.h>
#include <mitkLabelHighlightGuard.h>

#include <QWidget>

class QmitkMultiLabelInspectorPopup;
class QmitkMultiLabelInspector;

namespace Ui
{
  class QmitkMultiLabelSelectionWidget;
}

/*
* @brief This is a selection widget that allows to select one or multiple labels of a multi label segmentation..
*/
class MITKSEGMENTATIONUI_EXPORT QmitkMultiLabelSelectionWidget : public QWidget
{
  Q_OBJECT

public:
  QmitkMultiLabelSelectionWidget(QWidget* parent = nullptr);
  ~QmitkMultiLabelSelectionWidget();

  bool GetMultiSelectionMode() const;

  bool GetHighlightingActivated() const;
  mitk::LabelSetImage* GetMultiLabelSegmentation() const;
  mitk::DataNode* GetMultiLabelNode() const;
  QString GetEmptyInfo() const;

  /**
  * Convenience function that creates a binary mask that represents the selected label in the segmentation.
  * If no segmentation is defined or no label is selected, a null ptr is returned.
  * @pre Currently the function is only implemented for MultiSelectionMode==false;
  */
  mitk::Image::Pointer CreateSelectedLabelMask() const;

  using LabelValueType = mitk::LabelSetImage::LabelValueType;
  using LabelValueVectorType = mitk::LabelSetImage::LabelValueVectorType;

  /**
  * @brief Retrieve the currently selected labels (equals the last CurrentSelectionChanged values).
  */
  LabelValueVectorType GetSelectedLabels() const;

Q_SIGNALS:
  /**
  * @brief A signal that will be emitted if the selected labels change.
  *
  * @param labels A list of label values that are now selected.
  */
  void CurrentSelectionChanged(LabelValueVectorType labels) const;

  /** @brief Signal is emitted, if the segmentation is changed that is observed by the inspector.*/
  void SegmentationChanged() const;

public Q_SLOTS:

  /**
  * @brief Transform a list of label values into the new selection of the inspector.
  * @param selectedLabels A list of selected label values.
  * @remark Using this method to select labels will not trigger the CurrentSelectionChanged signal. Observers
  * should regard that to avoid signal loops.
  */
  void SetSelectedLabels(const LabelValueVectorType& selectedLabels);
  /**
  * @brief The passed label will be used as new selection in the widget
  * @param selectedLabel Value of the selected label.
  * @remark Using this method to select labels will not trigger the CurrentSelectionChanged signal. Observers
  * should regard that to avoid signal loops.
  */
  void SetSelectedLabel(mitk::LabelSetImage::LabelValueType selectedLabel);

  /** @brief Sets the segmentation that will be used and monitored by the widget.
  * @param segmentation      A pointer to the segmentation to set.
  * @remark You cannot set the segmentation directly if a segmentation node is
  * also set. Reset the node (nullptr) if you want to change to direct segmentation
  * setting.
  * @pre Segmentation node is nullptr.
  */
  void SetMultiLabelSegmentation(mitk::LabelSetImage* segmentation);

  /**
  * @brief Sets the segmentation node that will be used /monitored by the widget.
  *
  * @param node A pointer to the segmentation node.
  * @remark If not set some features (e.g. highlighting in render windows) of the inspectors
  * are not active.
  * @remark Currently it is also needed to circumvent the fact that
  * modification of data does not directly trigger modification of the
  * node (see T27307).
  */
  void SetMultiLabelNode(mitk::DataNode* node);

  void SetMultiSelectionMode(bool multiMode);

  void SetHighlightingActivated(bool visiblityMod);

  /** Set the info text that should be displayed if no label is selected.
   *  The string can contain HTML code, if desired.
   */
  void SetEmptyInfo(QString info);

protected:
  void resizeEvent(QResizeEvent* event) override;

private slots:
  void ShowPopup();
  void HidePopup();
  void OnSelectionFinished();

private:
  void PositionPopup();

  std::unique_ptr<Ui::QmitkMultiLabelSelectionWidget> m_Controls;
  std::unique_ptr<QmitkMultiLabelInspectorPopup> m_Popup;

};

/**
 * @brief This class is the pop-up used by QmitkMultiLabelSelectionWidget
 * for selection labels
 */
class QmitkMultiLabelInspectorPopup : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkMultiLabelInspectorPopup(QWidget* parent = nullptr);
  ~QmitkMultiLabelInspectorPopup();

  QmitkMultiLabelInspector* GetInspector() const;
  using LabelValueVectorType = QmitkMultiLabelSelectionWidget::LabelValueVectorType;

signals:
  /**
   * @brief Signal emitted when selection is finished
   */
  void SelectionFinished();

private slots:
  void OnPopupLabelsChanged(const LabelValueVectorType& selectedLabels);

private:
  std::unique_ptr<QmitkMultiLabelInspector> m_Inspector;
};

#endif
