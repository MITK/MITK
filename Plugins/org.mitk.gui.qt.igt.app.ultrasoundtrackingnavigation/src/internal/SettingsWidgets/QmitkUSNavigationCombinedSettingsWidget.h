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
#ifndef QMITKUSNAVIGATIONCOMBINEDSETTINGSWIDGET_H
#define QMITKUSNAVIGATIONCOMBINEDSETTINGSWIDGET_H

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
  explicit QmitkUSNavigationCombinedSettingsWidget(QWidget *parent = 0);
  ~QmitkUSNavigationCombinedSettingsWidget();
  virtual void OnSetSettingsNode(itk::SmartPointer<mitk::DataNode> settingsNode, bool overwriteValues);

protected:
  virtual void OnSaveProcessing();
  virtual void OnLoadSettingsProcessing();

  QString InteractionNameToFile(const QString& name) const;

private:
  Ui::QmitkUSNavigationCombinedSettingsWidget *ui;
};

#endif // QMITKUSNAVIGATIONCOMBINEDSETTINGSWIDGET_H
