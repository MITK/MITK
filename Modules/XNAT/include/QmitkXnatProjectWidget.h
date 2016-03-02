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

#ifndef QMITKXNATPROJECTWIDGET_H
#define QMITKXNATPROJECTWIDGET_H

// XNATUI
#include <ui_QmitkXnatProjectWidgetControls.h>
#include <MitkXNATExports.h>

// Qt
#include <QWidget>

// CTK XNAT Core
class ctkXnatProject;

class MITKXNAT_EXPORT QmitkXnatProjectWidget : public QWidget
{
  Q_OBJECT

public:

  enum Mode
  {
    INFO,
    CREATE
  };

  QmitkXnatProjectWidget(QWidget* parent = nullptr);
  QmitkXnatProjectWidget(Mode mode, QWidget* parent = nullptr);
  ~QmitkXnatProjectWidget();

  void SetProject(ctkXnatProject* project);
  ctkXnatProject* GetProject() const;

protected:
  Ui::QmitkXnatProjectWidgetControls m_Controls;

private:

  void Init();
  Mode m_Mode;
  ctkXnatProject* m_Project;
};

#endif // QMITKXNATPROJECTWIDGET_H
