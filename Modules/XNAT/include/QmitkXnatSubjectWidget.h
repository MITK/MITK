/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkXnatSubjectWidget_h
#define QmitkXnatSubjectWidget_h

#include <MitkXNATExports.h>

// Qt
#include <QWidget>
#include <memory>

// CTK XNAT Core
class ctkXnatSubject;

namespace Ui { class QmitkXnatSubjectWidgetControls; }

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
  std::unique_ptr<Ui::QmitkXnatSubjectWidgetControls> m_Controls;

private:
  void Init();
  Mode m_Mode;
  ctkXnatSubject *m_Subject;
};

#endif
