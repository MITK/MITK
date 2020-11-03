/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKXNATSUBJECTWIDGET_H
#define QMITKXNATSUBJECTWIDGET_H

#include <MitkXNATExports.h>
#include <ui_QmitkXnatSubjectWidgetControls.h>

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

  QmitkXnatSubjectWidget(QWidget *parent = nullptr);
  QmitkXnatSubjectWidget(Mode mode, QWidget *parent = nullptr);
  ~QmitkXnatSubjectWidget() override;

  void SetSubject(ctkXnatSubject *subject);
  ctkXnatSubject *GetSubject() const;

protected:
  Ui::QmitkXnatSubjectWidgetControls m_Controls;

private:
  void Init();
  Mode m_Mode;
  ctkXnatSubject *m_Subject;
};

#endif // QMITKXNATSUBJECTWIDGET_H
