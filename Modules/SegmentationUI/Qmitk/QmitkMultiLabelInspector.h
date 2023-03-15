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

namespace Ui
{
  class QmitkMultiLabelInspector;
}

/*
* @brief This is an inspector that offers a simple list view on a data storage.
*/
class MITKSEGMENTATIONUI_EXPORT QmitkMultiLabelInspector : public QWidget
{
  Q_OBJECT

public:
  QmitkMultiLabelInspector(QWidget* parent = nullptr);
  ~QmitkMultiLabelInspector();

  bool GetMultiSelectionMode() const;

  bool GetAllowVisibilityModification() const;
  bool GetAllowLockModification() const;
  bool GetAllowLabelModification() const;

  using LabelValueType = mitk::LabelSetImage::LabelValueType;
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

  void GoToLabel(LabelValueType label, const mitk::Point3D&) const;

public Q_SLOTS:

  /**
  * @brief Transform a list label values into the new selection of the inspector.
  * @param selectedNodes A list of selected label values.
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

  /**
  * @brief Sets the segmentation that will be used /monitored by the widget.
  *
  * @param segmentation      A pointer to the segmentation to set.
  */
  void SetMultiLabelSegmentation(mitk::LabelSetImage* segmentation);

  void SetMultiSelectionMode(bool multiMode);

  void SetAllowVisibilityModification(bool vmod);
  void SetAllowLockModification(bool lmod);
  void SetAllowLabelModification(bool labelMod);

  void SetDefaultLabelNaming(bool defaultLabelNaming);

  /** Adds an instance of the same label/class like the label instance
  * returned by GetCurrentLabel() to the segmentation. This new label
  * instance is returned by the function. If the inspector has no current label,
  * no new instance will be generated and nullptr will be returned.
  * @remark The new label instance is a clone of current label instance. Therefore
  * all properties but the LabelValue will be the same.
  * @pre AllowLabeModification must be set to true.*/
  mitk::Label* AddNewLabelInstance();

  /** Adds a new label to the segmentation. Depending on the settings the name of
  * the label will be either default generated or the rename delegate will be used.
  * @pre AllowLabeModification must be set to true.*/
  mitk::Label* AddNewLabel();

  /** Removes the first currently selected label of the segmentation. If no label is selected
  * nothing will happen.
  * @pre AllowLabeModification must be set to true.*/
  void RemoveLabel();

  /** Adds a new group with a new label to segmentation.
  * @pre AllowLabeModification must be set to true.*/
  mitk::Label* AddNewGroup();

  /** Removes the group of the first currently selected label of the segmentation. If no label is selected
  * nothing will happen.
  * @pre AllowLabeModification must be set to true.*/
  void RemoveGroup();


protected:
  void Initialize();
  void OnModelReset();

  QmitkMultiLabelTreeModel* m_Model;
  mitk::LabelSetImage::Pointer m_Segmentation;

  LabelValueVectorType m_LastValidSelectedLabels;
  QStyledItemDelegate* m_LockItemDelegate;
  QStyledItemDelegate* m_ColorItemDelegate;
  QStyledItemDelegate* m_VisibilityItemDelegate;

  Ui::QmitkMultiLabelInspector* m_Controls;

  LabelValueVectorType GetSelectedLabelsFromSelectionModel() const;
  void UpdateSelectionModel(const LabelValueVectorType& selectedLabels);

  /** Helper that returns the label object (if multiple labels are selected the first).*/
  mitk::Label* GetFirstSelectedLabelObject() const;

  mitk::Label* AddNewLabelInternal(const mitk::LabelSetImage::SpatialGroupIndexType& containingGroup);


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
  void OnChangeModelSelection(const QItemSelection& selected, const QItemSelection& deselected);

  void OnContextMenuRequested(const QPoint&);

  void OnAddLabelInstance(bool);
  void OnRemoveLabels(bool);
  void OnEraseLabels(bool);
  void OnMergeLabels(bool);

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

  void PrepareGoToLabel(LabelValueType labelID) const;

private:
  bool m_ShowVisibility = true;
  bool m_ShowLock = true;
  bool m_ShowOther = false;

  /** Indicates if the context menu allows changes in visiblity.
      Visiblity includes also color*/
  bool m_AllowVisibilityModification = true;
  bool m_AllowLockModification = true;
  bool m_AllowLabelModification = false;

  bool m_DefaultLabelNaming = true;

  bool m_ModelManipulationOngoing = false;
};

#endif // QMITKDATASTORAGELISTINSPECTOR_H
