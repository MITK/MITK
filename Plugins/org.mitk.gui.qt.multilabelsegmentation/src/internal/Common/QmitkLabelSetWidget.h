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

#include <ui_QmitkLabelSetWidgetControls.h>

#include "mitkWeakPointer.h"
#include "mitkDataNode.h"
#include "mitkNodePredicateBase.h"

#include <berryIBerryPreferences.h>

class QmitkDataStorageComboBox;
class QCompleter;

namespace mitk {
  class LabelSetImage;
  class DataStorage;
  class ToolManager;
}

class QmitkLabelSetWidget : public QWidget
{
  Q_OBJECT

public:

  explicit QmitkLabelSetWidget(QWidget* parent = NULL);
  ~QmitkLabelSetWidget();

  void SetActiveLabel(int);

  void SetPreferences( berry::IPreferences::Pointer prefs );

  virtual void setEnabled(bool enabled);

  void SetDataStorage(mitk::DataStorage& storage);

  void OnToolManagerWorkingDataModified();

signals:

  /// \brief Send a signal when it was requested to position the crosshairs on a label.
  void goToLabel(const mitk::Point3D&);

private slots:

  void OnSearchLabel();

  // reaction to signal "labelListModified" from QmitkLabelSetTableWidget
  void OnLabelListModified(const QStringList& list);

  // reaction to the signal "renameLabel" from QmitkLabelSetTableWidget
  void OnRenameLabel(int index, const mitk::Color& color, const std::string& name);

  // reaction to the signal "createSurface" from QmitkLabelSetTableWidget
  void OnCreateSurface(int);

  // reaction to the signal "smoothLabel" from QmitkLabelSetTableWidget
  void OnSmoothLabel(int);

  // reaction to the signal "toggleOutline" from QmitkLabelSetTableWidget
  void OnToggleOutline(bool);

  // reaction to the signal "createMask" from QmitkLabelSetTableWidget
  void OnCreateMask(int);

  // reaction to the signal "combineAndCreateMask" from QmitkLabelSetTableWidget
  void OnCombineAndCreateMask( const QList<QTableWidgetSelectionRange>& ranges );

  // reaction to the signal "combineAndCreateSurface" from QmitkLabelSetTableWidget
  void OnCombineAndCreateSurface( const QList<QTableWidgetSelectionRange>& ranges );

  // reaction to the button "New Label"
  void OnNewLabel();

  // reaction to the button "Add Layer"
  void OnAddLayer();

  // reaction to the button "Delete Layer"
  void OnDeleteLayer();

  // reaction to the button "Previous Layer"
  void OnPreviousLayer();

  // reaction to the button "Next Layer"
  void OnNextLayer();

  // reaction to the button "New Segmentation"
  void OnNewSegmentation();

  // reaction to the button "Delete Segmentation"
  void OnDeleteSegmentation();

  // reaction to the button "Load Segmentation"
  void OnLoadSegmentation();

  // reaction to the button "Import Segmentation"
  void OnImportSegmentation();

  // reaction to the button "Import Labeled Image"
  void OnImportLabeledImage();

  // reaction to the button "Save Segmentation"
  void OnSaveSegmentation();

private:

    Ui::QmitkLabelSetWidgetControls m_Controls;

    QCompleter* m_Completer;

    berry::IPreferences::Pointer m_Preferences;

    mitk::WeakPointer<mitk::DataStorage> m_DataStorage;

    mitk::ToolManager * m_ToolManager;

    // handling of a list of known (organ name, organ color) combination
    // ATTENTION these methods are defined in QmitkSegmentationOrganNamesHandling.cpp
    QStringList GetDefaultOrganColorString();
    void UpdateOrganList(QStringList& organColors, const QString& organname, mitk::Color colorname);
    void AppendToOrganList(QStringList& organColors, const QString& organname, int r, int g, int b);

    void WaitCursorOn();

    void WaitCursorOff();

    void RestoreOverrideCursor();

    void UpdateControls();

    void OnThreadedCalculationDone();

    void SetLastFileOpenPath(const QString& path) const;

    QString GetLastFileOpenPath() const;
};

#endif
