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


#ifndef QMITKSTDMULTIWIDGETEDITORPREFERENCEPAGE_H_
#define QMITKSTDMULTIWIDGETEDITORPREFERENCEPAGE_H_

#include "berryIQtPreferencePage.h"
#include <org_mitk_gui_qt_common_Export.h>
#include <berryIPreferences.h>

class QWidget;
class QCheckBox;

struct MITK_QT_COMMON QmitkStdMultiWidgetEditorPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:
  QmitkStdMultiWidgetEditorPreferencePage();
  QmitkStdMultiWidgetEditorPreferencePage(const QmitkStdMultiWidgetEditorPreferencePage& other);

  void Init(berry::IWorkbench::Pointer workbench);

  void CreateQtControl(QWidget* widget);

  QWidget* GetQtControl() const;

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
  QWidget* m_MainControl;
  QCheckBox* m_EnableFlexibleZooming;  
  berry::IPreferences::Pointer m_StdMultiWidgetEditorPreferencesNode;
};

#endif /* QMITKDATAMANAGERPREFERENCEPAGE_H_ */
