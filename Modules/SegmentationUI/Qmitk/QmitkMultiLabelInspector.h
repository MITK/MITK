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
#include <QmitkMultiLabelTreeModel.h>
#include "ui_QmitkMultiLabelInspectorControls.h"

class QmitkMultiLabelTreeModel;
class QStyledItemDelegate;

/*
* @brief This is an inspector that offers a simple list view on a data storage.
*/
class MITKSEGMENTATIONUI_EXPORT QmitkMultiLabelInspector : public QWidget
{
  Q_OBJECT

public:
  QmitkMultiLabelInspector(QWidget* parent = nullptr);

  bool GetMultiSelectionMode() const;

  bool GetAllowVisibilityModification() const;
  bool GetAllowLockModification() const;

  using LabelValueVectorType = mitk::LabelSetImage::LabelValueVectorType;

  /**
  * @brief Retrieve the currently selected labels (equals the last CurrentSelectionChanged values).
  */
  LabelValueVectorType GetSelectedLabels() const;

  /** Returns the label that currently has the focus in the tree view (indicated by QTreeView::currentIndex,
  thus the mouse is over it and it has a dashed border line).
  The current label must not equal the seleted label(s). If the mouse
  is not over a label (instance item), the method will return a null pointer.*/
  mitk::Label* GetCurrentLabel() const;

Q_SIGNALS:
  /**
  * @brief A signal that will be emitted if the selected labels change.
  *
  * @param labels A list of label values that are now selected.
  */
  void CurrentSelectionChanged(LabelValueVectorType labels) const;

  void GoToLabel(mitk::LabelSetImage::LabelValueType label, const mitk::Point3D&) const;

public Q_SLOTS:

  /**
  * @brief Transform a list label values into a model selection and set this as a new selection of the view
  *
  * @param selectedNodes A list of data nodes that should be newly selected.
  */
  void SetSelectedLabels(const LabelValueVectorType& selectedLabels);
  void SetSelectedLabel(mitk::LabelSetImage::LabelValueType selectedLabel);

  /**
  * @brief Sets the segmentation that will be used /monitored by the widget.
  *
  * @param segmentation      A pointer to the segmentation to set.
  */
  void SetMultiLabelSegmentation(mitk::LabelSetImage* segmentation);

  void SetMultiSelectionMode(bool multiMode);

  void SetAllowVisibilityModification(bool vmod);
  void SetAllowLockModification(bool lmod);

protected:
  void Initialize();
  void OnModelReset();

  QmitkMultiLabelTreeModel* m_Model;
  mitk::LabelSetImage::Pointer m_Segmentation;

  LabelValueVectorType m_LastValidSelectedLabels;
  QStyledItemDelegate* m_LockItemDelegate;
  QStyledItemDelegate* m_ColorItemDelegate;
  QStyledItemDelegate* m_VisibilityItemDelegate;

  Ui_QmitkMultiLabelInspector m_Controls;

  LabelValueVectorType GetSelectedLabelsFromSelectionModel() const;
  void UpdateSelectionModel(const LabelValueVectorType& selectedLabels);

  bool m_ShowVisibility = true;
  bool m_ShowLock = true;
  bool m_ShowOther = false;

  /** Indicates if the context menu allows changes in visiblity.
      Visiblity includes also color*/
  bool m_AllowVisibilityModification = true;
  bool m_AllowLockModification = true;
  bool m_AllowLabelModification = false;

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

  void OnContextMenuRequested(const QPoint&);

  void OnRemoveLabels(bool);
  void OnEraseLabels(bool);
  void OnMergeLabels(bool);

  void OnRemoveLabel(bool);
  void OnRenameLabel(bool);
  void OnEraseLabel(bool);

  void OnUnlockAllLabels(bool);
  void OnLockAllLabels(bool);

  void OnSetAllLabelsVisible(bool);
  void OnSetAllLabelsInvisible(bool);
  void OnSetOnlyActiveLabelVisible(bool);

  void OnItemDoubleClicked(const QModelIndex& index);

  void WaitCursorOn() const;
  void WaitCursorOff() const;
  void RestoreOverrideCursor() const;

  void PrepareGoToLabel(mitk::Label::PixelType labelID) const;

};

#endif // QMITKDATASTORAGELISTINSPECTOR_H
