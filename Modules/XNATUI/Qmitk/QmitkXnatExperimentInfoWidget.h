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

#ifndef QMITKXNATEXPERIMENTINFOWIDGET_H
#define QMITKXNATEXPERIMENTINFOWIDGET_H

// XNATUI
#include "ui_QmitkXnatExperimentInfoWidgetControls.h"
#include "MitkXNATUIExports.h"

// Qt
#include <QWidget>

// CTK XNAT Core
class ctkXnatExperiment;

class MITKXNATUI_EXPORT QmitkXnatExperimentInfoWidget : public QWidget
{
  Q_OBJECT

public:
  QmitkXnatExperimentInfoWidget(ctkXnatExperiment* experiment = 0, QWidget* parent = 0);
  ~QmitkXnatExperimentInfoWidget();

  void SetExperiment(ctkXnatExperiment* experiment);
  ctkXnatExperiment* GetExperiment() const;

protected:
  Ui::QmitkXnatExperimentInfoWidgetControls m_Controls;

private:
  ctkXnatExperiment* m_Experiment;
};

#endif // QMITKXNATEXPERIMENTINFOWIDGET_H
