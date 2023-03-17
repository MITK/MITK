/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkUSNavigationCombinedSettingsWidget_h
#define QmitkUSNavigationCombinedSettingsWidget_h

#include "QmitkUSNavigationAbstractSettingsWidget.h"
#include "mitkDataNode.h"

namespace Ui {
class QmitkUSNavigationCombinedSettingsWidget;
}

/**
 * \brief Settings widget for the USNavigationMarkerPlacement.
 * This widgets allows for configuring the experiment mode and for changing the
 * application between marker placement and punctuation.
 */
class QmitkUSNavigationCombinedSettingsWidget : public QmitkUSNavigationAbstractSettingsWidget
{
  Q_OBJECT

protected slots:
  void OnApplicationChanged(int);

public:
  explicit QmitkUSNavigationCombinedSettingsWidget(QWidget *parent = nullptr);
  ~QmitkUSNavigationCombinedSettingsWidget() override;
  void OnSetSettingsNode(itk::SmartPointer<mitk::DataNode> settingsNode, bool overwriteValues) override;

protected:
  void OnSaveProcessing() override;
  void OnLoadSettingsProcessing() override;

  QString InteractionNameToFile(const QString& name) const;

private:
  Ui::QmitkUSNavigationCombinedSettingsWidget *ui;
};

#endif
