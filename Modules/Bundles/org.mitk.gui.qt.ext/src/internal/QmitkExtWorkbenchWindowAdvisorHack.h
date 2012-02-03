/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <QObject>

class ctkPluginContext;
class QmitkPreferencesDialog;

class QmitkExtWorkbenchWindowAdvisorHack : public QObject
{
  Q_OBJECT

  public slots:

    void onUndo();
    void onRedo();
    void onImageNavigator();
    void onEditPreferences();
    void onQuit();

    void onResetPerspective();
    void onClosePerspective();
    void onNewWindow();
    void onIntro();
    
    /**
     * @brief This slot is called if the user klicks the menu item "help->context help" or presses F1.
     * The help page is shown in a workbench editor.
     */
    void onHelp();

    void onHelpContents();
    
    /**
     * @brief This slot is called if the user clicks in help menu the about button
     */
    void onAbout();

  public:

    QmitkExtWorkbenchWindowAdvisorHack();
    ~QmitkExtWorkbenchWindowAdvisorHack();

    static QmitkExtWorkbenchWindowAdvisorHack* undohack;
};
