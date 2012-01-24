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

#include "berryIQtPreferencePage.h"
#include "org_mitk_gui_qt_segmentation_Export.h"
#include <berryIPreferences.h>

class QWidget;
class QCheckBox;
class QRadioButton;
class QDoubleSpinBox;

class MITK_QT_SEGMENTATION QmitkSegmentationPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
  Q_INTERFACES(berry::IPreferencePage)

public:

  QmitkSegmentationPreferencePage();
  QmitkSegmentationPreferencePage(const QmitkSegmentationPreferencePage& other)
  {
    Q_UNUSED(other)
    throw std::runtime_error("Copy constructor not implemented");
  }
  ~QmitkSegmentationPreferencePage();

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

protected slots:

  void OnVolumeRenderingCheckboxChecked(int);
  void OnSmoothingCheckboxChecked(int);

protected:

  QWidget* m_MainControl;
  QRadioButton* m_RadioOutline;
  QRadioButton* m_RadioOverlay;
  QCheckBox* m_VolumeRenderingCheckBox;
  QCheckBox* m_SmoothingCheckBox;
  QDoubleSpinBox* m_SmoothingSpinBox;
  QDoubleSpinBox* m_DecimationSpinBox;
  QDoubleSpinBox* m_ClosingSpinBox;

  bool m_Initializing;

  berry::IPreferences::Pointer m_SegmentationPreferencesNode;
};

#endif /* QMITKDATAMANAGERPREFERENCEPAGE_H_ */

