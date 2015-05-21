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

#ifndef QMITKXNATSUBJECTINFOWIDGET_H
#define QMITKXNATSUBJECTINFOWIDGET_H

// XNATUI
#include <ui_QmitkXnatSubjectInfoWidgetControls.h>
#include <MitkXNATExports.h>

// Qt
#include <QWidget>

// CTK XNAT Core
class ctkXnatSubject;

class MITKXNAT_EXPORT QmitkXnatSubjectInfoWidget : public QWidget
{
  Q_OBJECT

public:
  QmitkXnatSubjectInfoWidget(ctkXnatSubject* subject, QWidget* parent = 0);
  ~QmitkXnatSubjectInfoWidget();

  void SetSubject(ctkXnatSubject* subject);
  ctkXnatSubject* GetSubject() const;

protected:
  Ui::QmitkXnatSubjectInfoWidgetControls m_Controls;

private:
  ctkXnatSubject* m_Subject;
};

#endif // QMITKXNATSUBJECTINFOWIDGET_H
