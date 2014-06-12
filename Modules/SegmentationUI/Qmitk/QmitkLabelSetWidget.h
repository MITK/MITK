/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QmitkLabelSetWidget_h
#define QmitkLabelSetWidget_h

#include "MitkSegmentationUIExports.h"

#include <ui_QmitkLabelSetWidgetControls.h>
#include "mitkColorSequenceRainbow.h"
#include "mitkVector.h"


class QmitkDataStorageComboBox;
class QCompleter;

namespace mitk {
  class LabelSetImage;
  class LabelSet;
  class Label;
  class DataStorage;
  class ToolManager;
  class DataNode;
}

class MitkSegmentationUI_EXPORT QmitkLabelSetWidget : public QWidget
{
  Q_OBJECT

public:

  explicit QmitkLabelSetWidget(QWidget* parent = NULL);
  ~QmitkLabelSetWidget();

  void SetDataStorage( mitk::DataStorage* storage );

  void SetOrganColors(const QStringList& organColors);

  void UpdateControls();

  virtual void setEnabled(bool enabled);

  QStringList &GetLabelStringList();


signals:

  /// \brief Send a signal when it was requested to go to a label.
  void goToLabel(const mitk::Point3D&);
  void resetView();

public slots:

  void ResetAllTableWidgetItems();
  void UpdateAllTableWidgetItems();
  void SelectLabelByPixelValue(int pixelValue);

private slots:

  void OnOpacityChanged(int);
  void OnCreateSmoothedSurface(bool);
  void OnCreateDetailedSurface(bool);
  void OnUnlockAllLabels(bool);
  void OnLockAllLabels(bool);
  void OnSetAllLabelsVisible(bool);
  void OnSetAllLabelsInvisible(bool);
  void OnSetOnlyActiveLabelVisible(bool);
  void OnRandomColor(bool);
  void OnEraseLabel(bool);
  void OnRemoveLabel(bool);
  void OnRenameLabel(bool);
  void OnCombineAndCreateMask(bool);
  void OnCreateMasks(bool);
  void OnCombineAndCreateSurface(bool);
  void OnEraseLabels(bool);
  void OnRemoveLabels(bool);
  void OnMergeLabels(bool);
  void OnLockedButtonClicked();
  void OnVisibleButtonClicked();
  void OnColorButtonClicked();

  void OnItemClicked(QTableWidgetItem *item);
  void OnItemDoubleClicked(QTableWidgetItem *item);
  void OnTableViewContextMenuRequested(const QPoint&);
  //void SelectTableWidgetItem(QTableWidgetItem *item);
  void InsertTableWidgetItem(const mitk::Label * label);
  void UpdateTableWidgetItem(QTableWidgetItem *item);


  // reaction to "returnPressed" signal from ...
  void OnSearchLabel();

  // reaction to signal "mergeLabel" from QmitkLabelSetTableWidget
  void OnMergeLabel(bool);

  // reaction to signal "labelListModified" from QmitkLabelSetTableWidget
  void OnLabelListModified(const QStringList& list);

  // reaction to signal "WorkingDataModified" from ToolManager
  void OnToolManagerWorkingDataModified();

  // reaction to the signal "toggleOutline" from QmitkLabelSetTableWidget
  void OnToggleOutline(bool);

  // reaction to the signal "createMask" from QmitkLabelSetTableWidget
  void OnCreateMask(bool);

  // reaction to the signal "createCroppedMask" from QmitkLabelSetTableWidget
  void OnCreateCroppedMask(bool);

  // reaction to the button "Change Label"
  void OnActiveLabelChanged(int pixelValue);

  // reaction to the button "Import Segmentation"
  void OnImportSegmentation();

  // reaction to the button "Import Labeled Image"
  void OnImportLabeledImage();


private:

    enum TableColumns { NAME_COL=0, LOCKED_COL, COLOR_COL, VISIBLE_COL };

    void WaitCursorOn();

    void WaitCursorOff();

    void RestoreOverrideCursor();

    void OnThreadedCalculationDone();

    void InitializeTableWidget();

    int GetPixelValueOfSelectedItem();

    mitk::LabelSetImage * GetWorkingImage();

    mitk::DataNode * GetWorkingNode();

    Ui::QmitkLabelSetWidgetControls m_Controls;

    mitk::ColorSequenceRainbow m_ColorSequenceRainbow;

    QCompleter* m_Completer;

    mitk::DataStorage* m_DataStorage;

    mitk::ToolManager* m_ToolManager;

    QStringList m_OrganColors;

    QStringList m_LabelStringList;

};

#endif
