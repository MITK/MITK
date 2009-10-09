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


#ifndef QMITKDATAMANAGERPREFERENCEPAGE_H_
#define QMITKDATAMANAGERPREFERENCEPAGE_H_

#include "cherryIQtPreferencePage.h"
#include "mitkQtDataManagerDll.h"

#include <QWidget>

struct MITK_QT_DATAMANAGER QmitkDataManagerPreferencePage : public cherry::IQtPreferencePage
{
  Q_OBJECT

public:
  QmitkDataManagerPreferencePage(QWidget* parent = 0, Qt::WindowFlags f = 0);

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

};

#endif /* QMITKDATAMANAGERPREFERENCEPAGE_H_ */
