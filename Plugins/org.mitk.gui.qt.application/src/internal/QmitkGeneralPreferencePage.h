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


#ifndef QMITKGENERALPREFERENCEPAGE_H_
#define QMITKGENERALPREFERENCEPAGE_H_

#include "berryIQtPreferencePage.h"
#include <berryIPreferences.h>

class QWidget;

class QmitkGeneralPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:

  /**
  * Default constructor
  */
  QmitkGeneralPreferencePage();

  /**
  * @see berry::IPreferencePage::Init(berry::IWorkbench::Pointer workbench)
  */
  void Init(berry::IWorkbench::Pointer workbench);

  /**
  * @see berry::IPreferencePage::CreateQtControl(void* parent)
  */
  void CreateQtControl(QWidget* widget);

  /**
  * @see berry::IPreferencePage::CreateQtControl()
  */
  QWidget* GetQtControl() const;

  /**
  * @see berry::IPreferencePage::PerformOk()
  */
  virtual bool PerformOk();

  /**
  * @see berry::IPreferencePage::PerformCancel()
  */
  virtual void PerformCancel();

   /**
  * @see berry::IPreferencePage::Update()
  */
  virtual void Update();

protected:

  QWidget* m_MainControl;

};

#endif /* QMITKGENERALPREFERENCEPAGE_H_ */
