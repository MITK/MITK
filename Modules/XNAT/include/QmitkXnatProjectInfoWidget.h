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

#ifndef QMITKXNATPROJECTINFOWIDGET_H
#define QMITKXNATPROJECTINFOWIDGET_H

// XNATUI
#include <ui_QmitkXnatProjectInfoWidgetControls.h>
#include <MitkXNATExports.h>

// Qt
#include <QWidget>

// CTK XNAT Core
class ctkXnatProject;

class MITKXNAT_EXPORT QmitkXnatProjectInfoWidget : public QWidget
{
  Q_OBJECT

public:
  QmitkXnatProjectInfoWidget(ctkXnatProject* project, QWidget* parent = 0);
  ~QmitkXnatProjectInfoWidget();

  ctkXnatProject* GetProject() const;

protected:
  Ui::QmitkXnatProjectInfoWidgetControls m_Controls;

private:
  ctkXnatProject* m_Project;
};

#endif // QMITKXNATPROJECTINFOWIDGET_H
