/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkXnatExperimentWidget_h
#define QmitkXnatExperimentWidget_h

// XNATUI
#include <MitkXNATExports.h>
#include <ui_QmitkXnatExperimentWidgetControls.h>

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

  QmitkXnatExperimentWidget(QWidget *parent = nullptr);
  QmitkXnatExperimentWidget(Mode mode, QWidget *parent = nullptr);
  ~QmitkXnatExperimentWidget() override;

  void SetExperiment(ctkXnatExperiment *experiment);
  ctkXnatExperiment *GetExperiment() const;

protected:
  Ui::QmitkXnatExperimentWidgetControls m_Controls;

private:
  void Init();
  Mode m_Mode;
  ctkXnatExperiment *m_Experiment;
};

#endif
