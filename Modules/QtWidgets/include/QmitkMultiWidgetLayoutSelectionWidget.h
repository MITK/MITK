/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKMULTIWIDGETLAYOUTSELECTIONWIDGET_H
#define QMITKMULTIWIDGETLAYOUTSELECTIONWIDGET_H

#include "MitkQtWidgetsExports.h"

#include "ui_QmitkMultiWidgetLayoutSelectionWidget.h"

// qt
#include "QWidget"

/**
* @brief
*
*
*/
class MITKQTWIDGETS_EXPORT QmitkMultiWidgetLayoutSelectionWidget : public QWidget
{
  Q_OBJECT

public:

  QmitkMultiWidgetLayoutSelectionWidget(QWidget* parent = nullptr);

Q_SIGNALS:

  void LayoutSet(int row, int column);
  void SaveLayout(std::string filename);
  void CustomLayoutLoad(std::string filename);
  void PresetLayoutLoad(std::string filename);

private Q_SLOTS:

  void OnTableItemSelectionChanged();
  void OnSetLayoutButtonClicked();
  void OnSaveLayoutButtonClicked();
  void OnLoadLayoutButtonClicked();
  void OnLayoutPresetSelected(int index);

private:

  void Init();


  Ui::QmitkMultiWidgetLayoutSelectionWidget ui;
  std::map<int, std::string> m_PresetNameMap;

};

#endif // QMITKMULTIWIDGETLAYOUTSELECTIONWIDGET_H
