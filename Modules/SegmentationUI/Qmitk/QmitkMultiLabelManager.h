/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMultiLabelManager_h
#define QmitkMultiLabelManager_h

#include <MitkSegmentationUIExports.h>

#include <mitkLabelSetImage.h>
#include <mitkDataNode.h>
#include <mitkNumericTypes.h>
#include <mitkITKEventObserverGuard.h>

#include <QWidget>

class QmitkDataStorageComboBox;
class QCompleter;

namespace Ui
{
  class QmitkMultiLabelManagerControls;
}

namespace mitk
{
  class DataStorage;
}

class MITKSEGMENTATIONUI_EXPORT QmitkMultiLabelManager : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkMultiLabelManager(QWidget *parent = nullptr);
  ~QmitkMultiLabelManager() override;


  using LabelValueVectorType = mitk::LabelSetImage::LabelValueVectorType;

  /**
  * @brief Retrieve the currently selected labels (equals the last CurrentSelectionChanged values).
  */
  LabelValueVectorType GetSelectedLabels() const;

  mitk::LabelSetImage* GetMultiLabelSegmentation() const;
  mitk::DataNode* GetMultiLabelNode() const;

Q_SIGNALS:
  /**
  * @brief A signal that will be emitted if the selected labels change.
  *
  * @param labels A list of label values that are now selected.
  */
  void CurrentSelectionChanged(const LabelValueVectorType& labels);

  /**
  * @brief A signal that will be emitted if the user has requested to "go to" a certain label.
  *
  * Going to a label would be e.g. to focus the render windows on the centroid of the label.
  * @param label The label that should be focused.
  * @param point in World coordinate that should be focused.
  */
  void GoToLabel(mitk::LabelSetImage::LabelValueType label, const mitk::Point3D& point) const;

  /** @brief Signal that is emitted, if a label should be (re)named and default
  * label naming is deactivated.
  *
  * The instance for which a new name is requested is passed with the signal.
  * @param label Pointer to the instance that needs a (new) name.
  * @param [in] rename Indicating if the signal was triggered for naming or renaming a label.
  * @param [out] canceled Indicating if the request was canceled by the used.
  */
  void LabelRenameRequested(mitk::Label* label, bool rename, bool& canceled) const;

public Q_SLOTS:

  /**
  * @brief Transform a list label values into a model selection and set this as a new selection of the view
  *
  * @param selectedLabels A list of data nodes that should be newly selected.
  */
  void SetSelectedLabels(const LabelValueVectorType& selectedLabels);

  /**
  * @brief Selects the passed label instance and sets a new selection of the view
  *
  * @param selectedLabel Value of the label instance that should be selected.
  */
  void SetSelectedLabel(mitk::LabelSetImage::LabelValueType selectedLabel);

  /**
  * @brief Sets the segmentation that will be used /monitored by the widget.
  *
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
  * @remark If not set some features of the manager are not active
  */
  void SetMultiLabelNode(mitk::DataNode* node);

  void SetDataStorage(mitk::DataStorage *storage);

  void UpdateControls();

  virtual void setEnabled(bool enabled);

  QStringList &GetLabelStringList();

  void SetDefaultLabelNaming(bool defaultLabelNaming);

private Q_SLOTS:

  // LabelSet dependent

  void OnRenameLabelShortcutActivated();

  // reaction to "returnPressed" signal from ...
  void OnSearchLabel();
  // reaction to the change of labels. If multiple labels are selected, it is ignored.
  void OnSelectedLabelChanged(const LabelValueVectorType& labels);

  // LabelSetImage Dependent
  void OnCreateDetailedSurface(bool);
  void OnCreateSmoothedSurface(bool);
  // reaction to the signal "createMask" from QmitkLabelSetTableWidget
  void OnCreateMask(bool);
  // reaction to the signal "createCroppedMask" from QmitkLabelSetTableWidget
  void OnCreateCroppedMask(bool);

  void OnSavePreset();
  void OnLoadPreset();

  void OnGoToLabel(mitk::LabelSetImage::LabelValueType label, const mitk::Point3D& position) const;
  void OnLabelRenameRequested(mitk::Label* label, bool rename, bool& canceled) const;
  void OnModelUpdated();
  void OnSegmentationChanged();

private:
  enum TableColumns
  {
    NAME_COL = 0,
    LOCKED_COL,
    COLOR_COL,
    VISIBLE_COL
  };

  void WaitCursorOn();

  void WaitCursorOff();

  void RestoreOverrideCursor();

  void OnThreadedCalculationDone();

  void AddSegmentationObserver();
  void RemoveSegmentationObserver();

  void OnLabelEvent(mitk::LabelSetImage::LabelValueType labelValue);
  void OnGroupEvent(mitk::LabelSetImage::GroupIndexType groupIndex);

  Ui::QmitkMultiLabelManagerControls* m_Controls;

  QCompleter *m_Completer;

  QStringList m_OrganColors;

  QStringList m_LabelStringList;

  bool m_ProcessingManualSelection;

  mitk::DataStorage* m_DataStorage;

  mitk::ITKEventObserverGuard m_LabelAddedObserver;
  mitk::ITKEventObserverGuard m_LabelModifiedObserver;
  mitk::ITKEventObserverGuard m_LabelRemovedObserver;
  mitk::ITKEventObserverGuard m_GroupAddedObserver;
  mitk::ITKEventObserverGuard m_GroupModifiedObserver;
  mitk::ITKEventObserverGuard m_GroupRemovedObserver;
};

#endif
