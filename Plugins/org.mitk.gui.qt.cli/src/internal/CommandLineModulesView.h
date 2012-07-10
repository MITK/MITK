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

#ifndef CommandLineModulesView_h
#define CommandLineModulesView_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_CommandLineModulesViewControls.h"

namespace berry
{
  class IBerryPreferences;
}

/*!
 * \class CommandLineModulesView
 * \brief Provides basic GUI interface to the CTK command line modules.
 *
 * The intention of this class is that it is an intermediate integration point
 * for the CTK command line modules. Ideally, an MITK base application would
 * load command line modules dynamically at runtime, and promote them to the
 * main window menu bar to appear as fully fledged views. However, currently
 * this is not supported, and so this view provides a simplified interface
 * in order to facilitate testing, debugging, and understanding of use-cases.
 *
 * \sa QmitkAbstractView
 * \ingroup org_mitk_gui_qt_cli_internal
 */
class CommandLineModulesView : public QmitkAbstractView
{  
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT
  
public:

  static const std::string VIEW_ID;

  CommandLineModulesView();
  virtual ~CommandLineModulesView();

  /**
   * \brief Main method, called by framework to create the GUI at the right time.
   * \param parent The parent QWidget, as this class itself is not a QWidget subclass.
   */
  virtual void CreateQtPartControl(QWidget *parent);

protected slots:
  
protected:

  /**
   * \brief Called by framework to set the focus on the right widget
   * when this view has focus, but currently does nothing.
   */
  virtual void SetFocus();

private:

  /**
   * \brief Called by the framework to indicate that the preferences have changed.
   * \param prefs not used.
   */
  virtual void OnPreferencesChanged(const berry::IBerryPreferences* prefs);

  /**
   * Called on startup and by OnPreferencesChanged to load the preferences into member variables.
   */
  void RetrievePreferenceValues();

  QString m_TemporaryDirectoryName;
  QString m_ModulesDirectoryName;
  Ui::CommandLineModulesViewControls m_Controls;
};

#endif // CommandLineModulesView_h

