/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkFormsExampleView_h
#define QmitkFormsExampleView_h

#include <QmitkAbstractView.h>
#include <mitkForm.h>

class QmitkFormsExampleView : public QmitkAbstractView
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  QmitkFormsExampleView();
  ~QmitkFormsExampleView() override;

  void CreateQtPartControl(QWidget* parent) override;
  void SetFocus() override;

private:
  mitk::Forms::Form m_Form;
};

#endif
