/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKMULTILABELSEGMENTATIONINSPECTOR_H
#define QMITKMULTILABELSEGMENTATIONINSPECTOR_H

#include <MitkSegmentationUIExports.h>
#include <mitkWeakPointer.h>

#include <QWidget>
#include <QmitkMultiLabelSegmentationTreeModel.h>
#include "ui_QmitkMultiLabelSegmentationInspector.h"

class QmitkMultiLabelSegmentationTreeModel;
class QStyledItemDelegate;

/*
* @brief This is an inspector that offers a simple list view on a data storage.
*/
class MITKSEGMENTATIONUI_EXPORT QmitkMultiLabelSegmentationInspector : public QWidget
{
  Q_OBJECT

public:
  QmitkMultiLabelSegmentationInspector(QWidget* parent = nullptr);

  using SelectionMode = QAbstractItemView::SelectionMode;
  void SetSelectionMode(SelectionMode mode);
  SelectionMode GetSelectionMode() const;

  /**
  * @brief Sets the segmentation that will be used /monitored by the widget.
  *
  * @param segmentation      A pointer to the segmentation to set.
  */
  void SetMultiLabelSegmentation(mitk::LabelSetImage* segmentation);

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
  void CurrentSelectionChanged(LabelValueVectorType labels);

public Q_SLOTS:

  /**
  * @brief Transform a list label values into a model selection and set this as a new selection of the view
  *
  * @param selectedNodes A list of data nodes that should be newly selected.
  */
  void SetSelectedLabels(LabelValueVectorType selectedLabels);
  void SetSelectedLabel(mitk::LabelSetImage::LabelValueType selectedLabel);

protected:
  void Initialize();
  void OnModelReset();

  QmitkMultiLabelSegmentationTreeModel* m_Model;
  mitk::WeakPointer<mitk::LabelSetImage> m_Segmentation;

  QStyledItemDelegate* m_LockItemDelegate;
  QStyledItemDelegate* m_ColorItemDelegate;
  QStyledItemDelegate* m_VisibilityItemDelegate;

  Ui_QmitkMultiLabelSegmentationInspector m_Controls;

private Q_SLOTS:
  /**
  * @brief Transform a labels selection into a data node list and emit the 'CurrentSelectionChanged'-signal.
  *
  *   The function adds the selected nodes from the original selection that could not be modified, if
  *   'm_SelectOnlyVisibleNodes' is false.
  *   This slot is internally connected to the 'selectionChanged'-signal of the selection model of the private member item view.
  *
  * @param selected	The newly selected items.
  * @param deselected	The newly deselected items.
  */
  void ChangeModelSelection(const QItemSelection& selected, const QItemSelection& deselected);

};

#endif // QMITKDATASTORAGELISTINSPECTOR_H
