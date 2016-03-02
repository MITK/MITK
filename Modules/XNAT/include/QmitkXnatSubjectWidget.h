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

#ifndef QMITKXNATSUBJECTWIDGET_H
#define QMITKXNATSUBJECTWIDGET_H

#include <ui_QmitkXnatSubjectWidgetControls.h>
#include <MitkXNATExports.h>

// Qt
#include <QWidget>

// CTK XNAT Core
class ctkXnatSubject;

class MITKXNAT_EXPORT QmitkXnatSubjectWidget : public QWidget
{
  Q_OBJECT

public:

  enum Mode
  {
    INFO,
    CREATE
  };

  QmitkXnatSubjectWidget(QWidget* parent = nullptr);
  QmitkXnatSubjectWidget(Mode mode, QWidget* parent = nullptr);
  ~QmitkXnatSubjectWidget();

  void SetSubject(ctkXnatSubject* subject);
  ctkXnatSubject* GetSubject() const;

protected:
  Ui::QmitkXnatSubjectWidgetControls m_Controls;

private:
  void Init();
  Mode m_Mode;
  ctkXnatSubject* m_Subject;
};

#endif // QMITKXNATSUBJECTWIDGET_H
