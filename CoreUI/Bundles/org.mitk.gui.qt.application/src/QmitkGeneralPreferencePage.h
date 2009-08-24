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

#include <QWidget>

#include "cherryIQtPreferencePage.h"
#include "mitkQtAppDll.h"

class QCheckBox;

struct MITK_QT_APP QmitkGeneralPreferencePage : public cherry::IQtPreferencePage
{
  Q_OBJECT

public:
  QmitkGeneralPreferencePage(QWidget* parent = 0, Qt::WindowFlags f = 0);

  ///
  /// Invoked when the OK button was clicked in the preferences dialog
  ///
  virtual bool PerformOk();

  ///
  /// Invoked when the Cancel button was clicked in the preferences dialog
  ///
  virtual void PerformCancel();
protected:
  cherry::IPreferences::WeakPtr m_GeneralPreferencesNode;
  QCheckBox* m_StartMaximized;

};

#endif /* QMITKGENERALPREFERENCEPAGE_H_ */
