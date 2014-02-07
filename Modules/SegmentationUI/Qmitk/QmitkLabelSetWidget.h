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

#include "SegmentationUIExports.h"

#include <ui_QmitkLabelSetWidgetControls.h>

#include "mitkWeakPointer.h"
#include "mitkDataNode.h"

class QmitkDataStorageComboBox;
class QCompleter;

namespace mitk {
  class LabelSetImage;
  class DataStorage;
  class ToolManager;
}

class SegmentationUI_EXPORT QmitkLabelSetWidget : public QWidget
{
  Q_OBJECT

public:

  explicit QmitkLabelSetWidget(QWidget* parent = NULL);
  ~QmitkLabelSetWidget();

  void SetActiveLabel(int);

  void SetDataStorage( mitk::DataStorage& storage );

  void SetOrganColors(const QStringList& organColors);

  void UpdateControls();

  virtual void setEnabled(bool enabled);

signals:

  /// \brief Send a signal when it was requested to go to a label.
  void goToLabel(const mitk::Point3D&);

private slots:

  // reaction to "returnPressed" signal from ...
  void OnSearchLabel();

  // reaction to signal "mergeLabel" from QmitkLabelSetTableWidget
  void OnMergeLabel(int);

  // reaction to signal "labelListModified" from QmitkLabelSetTableWidget
  void OnLabelListModified(const QStringList& list);

  // reaction to the signal "renameLabel" from QmitkLabelSetTableWidget
  void OnRenameLabel(int index, const mitk::Color& color, const std::string& name);

  // reaction to the signal "createSurface" from QmitkLabelSetTableWidget
  void OnCreateSurface(int index, bool smooth);

  // reaction to the signal "toggleOutline" from QmitkLabelSetTableWidget
  void OnToggleOutline(bool);

  // reaction to the signal "createMask" from QmitkLabelSetTableWidget
  void OnCreateMask(int);

  // reaction to the signal "createCroppedMask" from QmitkLabelSetTableWidget
  void OnCreateCroppedMask(int);

  // reaction to the signal "combineAndCreateMask" from QmitkLabelSetTableWidget
  void OnCombineAndCreateMask( const QList<QTableWidgetSelectionRange>& ranges );

  // reaction to the signal "combineAndCreateSurface" from QmitkLabelSetTableWidget
  void OnCombineAndCreateSurface( const QList<QTableWidgetSelectionRange>& ranges );

  // reaction to the button "Change Label"
  void OnActiveLabelChanged(int);

  // reaction to the button "Add Layer"
  void OnAddLayer();

  // reaction to the button "Delete Layer"
  void OnDeleteLayer();

  // reaction to the button "Previous Layer"
  void OnPreviousLayer();

  // reaction to the button "Next Layer"
  void OnNextLayer();

  // reaction to the combobox change "Change Layer"
  void OnChangeLayer(int);

  // reaction to the button "Deactive Active Tool"
  void OnDeactivateActiveTool();

  // reaction to the button "Lock exterior"
  void OnLockExteriorToggled(bool);

  // reaction to the button "Import Segmentation"
  void OnImportSegmentation();

  // reaction to the button "Import Labeled Image"
  void OnImportLabeledImage();

private:

    void WaitCursorOn();

    void WaitCursorOff();

    void RestoreOverrideCursor();

    void OnThreadedCalculationDone();

    Ui::QmitkLabelSetWidgetControls m_Controls;

    QCompleter* m_Completer;

    mitk::WeakPointer<mitk::DataStorage> m_DataStorage;

    mitk::ToolManager* m_ToolManager;

    QStringList m_OrganColors;
};

#endif
