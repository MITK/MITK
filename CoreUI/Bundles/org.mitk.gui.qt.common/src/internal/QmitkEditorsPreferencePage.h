/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-02-10 14:14:32 +0100 (Di, 10 Feb 2009) $
Version:   $Revision: 16224 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef QMITKEDITORSPREFERENCEPAGE_H_
#define QMITKEDITORSPREFERENCEPAGE_H_

#include "berryIQtPreferencePage.h"
#include <berryIPreferences.h>

class QWidget;

class QmitkEditorsPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:

  /**
  * Default constructor
  */
  QmitkEditorsPreferencePage();

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

#endif /* QMITKEDITORSPREFERENCEPAGE_H_ */
