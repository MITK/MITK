/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkGeneralPreferencePage_h
#define QmitkGeneralPreferencePage_h

#include "berryIQtPreferencePage.h"

class QWidget;
class QCheckBox;

class QmitkGeneralPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:

  QmitkGeneralPreferencePage();

  /**
  * @see berry::IPreferencePage::Init(berry::IWorkbench::Pointer workbench)
  */
  void Init(berry::IWorkbench::Pointer workbench) override;

  /**
  * @see berry::IPreferencePage::CreateQtControl(void* parent)
  */
  void CreateQtControl(QWidget* widget) override;

  /**
  * @see berry::IPreferencePage::CreateQtControl()
  */
  QWidget* GetQtControl() const override;

  /**
  * @see berry::IPreferencePage::PerformOk()
  */
  bool PerformOk() override;

  /**
  * @see berry::IPreferencePage::PerformCancel()
  */
  void PerformCancel() override;

   /**
  * @see berry::IPreferencePage::Update()
  */
  void Update() override;

protected:

    QWidget* m_MainControl;

    QCheckBox* m_GlobalReinitOnNodeDelete;
    QCheckBox* m_GlobalReinitOnNodeVisibilityChanged;
};

#endif
