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


#ifndef __QMITK_NODE_SELECTION_PREFERENCE_PAGE_H
#define __QMITK_NODE_SELECTION_PREFERENCE_PAGE_H

#include "berryIQtPreferencePage.h"
#include "berryIPreferences.h"

#include "mitkDataStorageInspectorGenerator.h"

#include "ui_QmitkNodeSelectionPreferencePage.h"

class QWidget;

/**
* \class QmitkNodeSelectionPreferencePage
* \brief Preference page for general node selection settings.
*/
class QmitkNodeSelectionPreferencePage : public QObject, public berry::IQtPreferencePage
{
  Q_OBJECT
    Q_INTERFACES(berry::IPreferencePage)

public:
  QmitkNodeSelectionPreferencePage();
  ~QmitkNodeSelectionPreferencePage();

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

protected slots:

  void UpdateWidgets();
  void MoveDown();
  void MoveUp();

protected:

  QWidget                 *m_MainControl;
  Ui::QmitkNodeSelectionPreferencePage* m_Controls;

  mitk::DataStorageInspectorGenerator::ProviderMapType m_Providers;

};

#endif
