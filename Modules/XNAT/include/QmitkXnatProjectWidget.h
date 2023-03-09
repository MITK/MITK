/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkXnatProjectWidget_h
#define QmitkXnatProjectWidget_h

// XNATUI
#include <MitkXNATExports.h>
#include <ui_QmitkXnatProjectWidgetControls.h>

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

  QmitkXnatProjectWidget(QWidget *parent = nullptr);
  QmitkXnatProjectWidget(Mode mode, QWidget *parent = nullptr);
  ~QmitkXnatProjectWidget() override;

  void SetProject(ctkXnatProject *project);
  ctkXnatProject *GetProject() const;

protected:
  Ui::QmitkXnatProjectWidgetControls m_Controls;

private:
  void Init();
  Mode m_Mode;
  ctkXnatProject *m_Project;
};

#endif
