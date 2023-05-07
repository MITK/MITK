/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMultiWidgetLayoutSelectionWidget_h
#define QmitkMultiWidgetLayoutSelectionWidget_h

#include "MitkQtWidgetsExports.h"

#include "ui_QmitkMultiWidgetLayoutSelectionWidget.h"

#include <nlohmann/json.hpp>

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

  // needs to be connected via Qt::DirectConnection (usually default), to ensure the stream pointers validity
  void SaveLayout(std::ostream* outStream);

  void LoadLayout(const nlohmann::json* jsonData);

private Q_SLOTS:

  void OnTableItemSelectionChanged();
  void OnSetLayoutButtonClicked();
  void OnSaveLayoutButtonClicked();
  void OnLoadLayoutButtonClicked();
  void OnLayoutPresetSelected(int index);

private:

  void Init();


  Ui::QmitkMultiWidgetLayoutSelectionWidget ui;
  std::map<int, nlohmann::json> m_PresetMap;

};

#endif
