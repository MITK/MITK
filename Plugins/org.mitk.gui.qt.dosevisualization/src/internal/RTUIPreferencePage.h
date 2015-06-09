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


#ifndef __RT_UI_PREFERENCE_PAGE_H
#define __RT_UI_PREFERENCE_PAGE_H

#include "berryIQtPreferencePage.h"
#include "berryIPreferences.h"

#include "ui_RTUIPreferencePageControls.h"

class QWidget;

/**
* \class RTUIPreferencePage
* \brief Preference page for general RT visualization settings.
*/
class RTUIPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
    Q_INTERFACES(berry::IPreferencePage)

public:
  RTUIPreferencePage();
  ~RTUIPreferencePage();

  /**
  * \brief Called by framework to initialize this preference page, but currently does nothing.
  * \param workbench The workbench.
  */
  void Init(berry::IWorkbench::Pointer workbench);

  /**
  * \brief Called by framework to create the GUI, and connect signals and slots.
  * \param widget The Qt widget that acts as parent to all GUI components, as this class itself is not derived from QWidget.
  */
  void CreateQtControl(QWidget* widget);

  /**
  * \brief Required by framework to get hold of the GUI.
  * \return QWidget* the top most QWidget for the GUI.
  */
  QWidget* GetQtControl() const;

  /**
  * \see IPreferencePage::PerformOk
  */
  virtual bool PerformOk();

  /**
  * \see IPreferencePage::PerformCancel
  */
  virtual void PerformCancel();

  /**
  * \see IPreferencePage::Update
  */
  virtual void Update();

  public slots:

protected:

  QWidget                 *m_MainControl;
  Ui::RTUIPreferencePageControls* m_Controls;

  berry::IPreferences::Pointer m_PreferencesNode;

};

#endif
