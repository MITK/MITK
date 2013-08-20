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

#include <berryIBerryPreferences.h>

class QmitkDataStorageComboBox;
class QCompleter;

namespace mitk {
  class LabelSetImage;
}

class QmitkLabelSetWidget : public QWidget
{
  Q_OBJECT

public:

  explicit QmitkLabelSetWidget(QWidget* parent = NULL);
  ~QmitkLabelSetWidget();

  void SetActiveLabel(int);

  /// \brief Set the LabelSetImage the widget should listen to.
  void SetActiveLabelSetImage(mitk::LabelSetImage*);

  void SetPreferences( berry::IPreferences::Pointer prefs );

signals:

  /// \brief Send a signal when it was requested to go to a label.
  void goToLabel(const mitk::Point3D&);

private slots:

  void OnSearchLabel();

  // reaction to signal "labelListModified" from QmitkLabelSetTableWidget
  void OnLabelListModified(const QStringList& list);

  // reaction to the signal "renameLabel" from QmitkLabelSetTableWidget
  void OnRenameLabel(int);

  // reaction to the signal "newlabel" from QmitkLabelSetTableWidget
  void OnNewLabel();

  // reaction to the signal "createSurface" from QmitkLabelSetTableWidget
  void OnCreateSurface(int);

private:

    Ui::QmitkLabelSetWidgetControls m_Controls;

    QCompleter* m_Completer;

    berry::IPreferences::Pointer m_Preferences;

     // handling of a list of known (organ name, organ color) combination
  // ATTENTION these methods are defined in QmitkSegmentationOrganNamesHandling.cpp
  QStringList GetDefaultOrganColorString();
  void UpdateOrganList(QStringList& organColors, const QString& organname, mitk::Color colorname);
  void AppendToOrganList(QStringList& organColors, const QString& organname, int r, int g, int b);

};

#endif
