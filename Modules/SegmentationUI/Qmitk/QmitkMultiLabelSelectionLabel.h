/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMultiLabelSelectionLabel_h
#define QmitkMultiLabelSelectionLabel_h

#include <MitkSegmentationUIExports.h>
#include <mitkWeakPointer.h>
#include <mitkLabelSetImage.h>
#include <mitkDataNode.h>
#include <mitkLabelHighlightGuard.h>

#include <QWidget>

namespace Ui
{
  class QmitkMultiLabelSelectionLabel;
}

/*
* @brief This is a widget that behaves like a QLabel widgets and displays the selected labels of
* a segmentation with name and color.
* It also allows label highlighting in render windows if the node is set and
* mouse hovers over the widget.
*/
class MITKSEGMENTATIONUI_EXPORT QmitkMultiLabelSelectionLabel : public QWidget
{
  Q_OBJECT

public:
  QmitkMultiLabelSelectionLabel(QWidget* parent = nullptr);
  ~QmitkMultiLabelSelectionLabel();

  bool GetHighlightingActivated() const;
  mitk::LabelSetImage* GetMultiLabelSegmentation() const;
  mitk::DataNode* GetMultiLabelNode() const;
  QString GetEmptyInfo() const;

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
  * @remark If not set, highlighting in render windows will not work.
  */
  void SetMultiLabelNode(mitk::DataNode* node);

  void SetHighlightingActivated(bool visiblityMod);

  /** Set the info text that should be displayed if no label is selected.
   *  The string can contain HTML code, if desired.
   */
  void SetEmptyInfo(QString info);

protected:
  void Initialize();
  void UpdateWidget(const LabelValueVectorType& selectedLabels);

  mitk::LabelSetImage::Pointer m_Segmentation;

  LabelValueVectorType m_LastValidSelectedLabels;

  std::unique_ptr<Ui::QmitkMultiLabelSelectionLabel> m_Controls;

private Q_SLOTS:
  void enterEvent(QEnterEvent* event) override;
  void leaveEvent(QEvent* event) override;

private:
  bool m_HighlightingActivated = true;
  mitk::DataNode::Pointer m_SegmentationNode;
  unsigned long m_SegmentationNodeDataMTime;
  mitk::ITKEventObserverGuard m_SegmentationObserver;
  mitk::LabelHighlightGuard m_LabelHighlightGuard;
  QString m_EmptyInfo;
};

#endif
