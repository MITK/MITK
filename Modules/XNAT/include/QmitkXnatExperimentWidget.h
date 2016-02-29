/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological rmatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QMITKXNATEXPERIMENTWIDGET_H
#define QMITKXNATEXPERIMENTWIDGET_H

// XNATUI
#include <ui_QmitkXnatExperimentWidgetControls.h>
#include <MitkXNATExports.h>

// Qt
#include <QWidget>

// CTK XNAT Core
class ctkXnatExperiment;

class MITKXNAT_EXPORT QmitkXnatExperimentWidget : public QWidget
{
  Q_OBJECT

public:

  enum Mode
  {
    INFO,
    CREATE
  };

  QmitkXnatExperimentWidget(QWidget* parent = nullptr);
  QmitkXnatExperimentWidget(Mode mode, QWidget* parent = nullptr);
  ~QmitkXnatExperimentWidget();

  void SetExperiment(ctkXnatExperiment* experiment);
  ctkXnatExperiment* GetExperiment() const;

protected:
  Ui::QmitkXnatExperimentWidgetControls m_Controls;

private:
  void Init();
  Mode m_Mode;
  ctkXnatExperiment* m_Experiment;
};

#endif // QMITKXNATEXPERIMENTWIDGET_H
