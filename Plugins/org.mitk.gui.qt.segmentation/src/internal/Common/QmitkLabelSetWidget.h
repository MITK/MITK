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

#include <berryIBerryPreferences.h>

class QmitkDataStorageComboBox;
class QCompleter;

namespace mitk {
  class LabelSetImage;
  class DataStorage;
}

class QmitkLabelSetWidget : public QWidget
{
  Q_OBJECT

public:

  explicit QmitkLabelSetWidget(QWidget* parent = NULL);
  ~QmitkLabelSetWidget();

  void SetActiveLabel(int);

  void SetPreferences( berry::IPreferences::Pointer prefs );

  void SetDataStorage(mitk::DataStorage& storage);

  void OnToolManagerWorkingDataModified();

  void OnToolManagerReferenceDataModified();

signals:

  /// \brief Send a signal when it was requested to position the crosshairs on a label.
  void goToLabel(const mitk::Point3D&);

private slots:

  void OnSearchLabel();

  // reaction to signal "labelListModified" from QmitkLabelSetTableWidget
  void OnLabelListModified(const QStringList& list);

  // reaction to the signal "renameLabel" from QmitkLabelSetTableWidget
  void OnRenameLabel(int index, const mitk::Color& color, const std::string& name);

  // reaction to the signal "newlabel" from QmitkLabelSetTableWidget
  void OnNewLabel();

  // reaction to the signal "createSurface" from QmitkLabelSetTableWidget
  void OnCreateSurface(int);

  // reaction to the signal "createMask" from QmitkLabelSetTableWidget
  void OnCreateMask(int);

  // reaction to the signal "combineAndCreateMask" from QmitkLabelSetTableWidget
  void OnCombineAndCreateMask( const QList<QTableWidgetSelectionRange>& ranges );

  // reaction to the signal "combineAndCreateSurface" from QmitkLabelSetTableWidget
  void OnCombineAndCreateSurface( const QList<QTableWidgetSelectionRange>& ranges );

  // reaction to the button "New segmentation session"
  void OnNewLabelSet();

  // reaction to the button "Load segmentation session"
  void OnLoadLabelSet();

  // reaction to the button "Import segmentation session"
  void OnImportLabelSet();

  // reaction to the button "Save Segmentation"
  void OnSaveLabelSet();

private:

    Ui::QmitkLabelSetWidgetControls m_Controls;

    QCompleter* m_Completer;

    berry::IPreferences::Pointer m_Preferences;

    mitk::WeakPointer<mitk::DataStorage> m_DataStorage;

    mitk::DataNode::Pointer m_ReferenceNode;
    mitk::DataNode::Pointer m_WorkingNode;

    // handling of a list of known (organ name, organ color) combination
    // ATTENTION these methods are defined in QmitkSegmentationOrganNamesHandling.cpp
    QStringList GetDefaultOrganColorString();
    void UpdateOrganList(QStringList& organColors, const QString& organname, mitk::Color colorname);
    void AppendToOrganList(QStringList& organColors, const QString& organname, int r, int g, int b);

    void WaitCursorOn();

    void BusyCursorOn();

    void WaitCursorOff();

    void BusyCursorOff();

    void RestoreOverrideCursor();

};

#endif
