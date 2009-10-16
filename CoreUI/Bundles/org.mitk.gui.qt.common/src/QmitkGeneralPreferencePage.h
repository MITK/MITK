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


#ifndef QMITKGENERALPREFERENCEPAGE_H_
#define QMITKGENERALPREFERENCEPAGE_H_

#ifdef __MINGW32__
// We need to inlclude winbase.h here in order to declare
// atomic intrinsics like InterlockedIncrement correctly.
// Otherwhise, they would be declared wrong within qatomic_windows.h .
#include <windows.h>
#endif


#include "cherryIQtPreferencePage.h"
#include "mitkQtCommonDll.h"

//class QCheckBox;

struct MITK_QT_COMMON QmitkGeneralPreferencePage : public cherry::IQtPreferencePage
{

public:

  QmitkGeneralPreferencePage();

  /**
   * \see IPreferencePage::Init(IWorkbench::Pointer)
   */
  virtual void Init(cherry::IWorkbench::Pointer workbench);

  /**
   * \see IQtPreferencePage::CreateQtControl(Qwidget*)
   */
  virtual void CreateQtControl(QWidget* parent);

  /**
   * \see IQtPreferencePage::GetQtControl()
   */
  virtual QWidget* GetQtControl() const;

  ///
  /// \see IPreferencePage::PerformOk()
  ///
  virtual bool PerformOk();

  ///
  /// \see IPreferencePage::PerformCancel()
  ///
  virtual void PerformCancel();

  ///
  /// \see IPreferencePage::Update()
  ///
  virtual void Update();
protected:
  cherry::IPreferences::WeakPtr m_GeneralPreferencesNode;
  //QCheckBox* m_StartMaximized;

  QWidget* m_MainControl;

};

#endif /* QMITKGENERALPREFERENCEPAGE_H_ */
