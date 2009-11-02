/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
 Language:  C++
 Date:      $Date$
 Version:   $Revision: 16224 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef QmitkSegmentationPreferencePage_h_included
#define QmitkSegmentationPreferencePage_h_included

#include "cherryIQtPreferencePage.h"
#include "mitkQtSegmentationDll.h"
#include <cherryIPreferences.h>

class QWidget;
class QCheckBox;
class QRadioButton;

class MITK_QT_SEGMENTATION QmitkSegmentationPreferencePage : public cherry::IQtPreferencePage
{

public:

  QmitkSegmentationPreferencePage();

  void Init(cherry::IWorkbench::Pointer workbench);

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
  QRadioButton* m_RadioOutline;
  QRadioButton* m_RadioOverlay;
  QCheckBox* m_VolumeRenderingCheckBox;

  cherry::IPreferences::Pointer m_SegmentationPreferencesNode;
};

#endif /* QMITKDATAMANAGERPREFERENCEPAGE_H_ */

