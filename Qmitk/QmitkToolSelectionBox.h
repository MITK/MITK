/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef QmitkQmitkToolSelectionBox_h_Included
#define QmitkQmitkToolSelectionBox_h_Included

#include "QmitkToolGUIArea.h"

#include "mitkToolManager.h"

#include <qbuttongroup.h>

#include <map>

class QmitkToolGUI;

/**
  \brief Display the tool selection state of a mitk::ToolManager

  \sa mitk::ToolManager

  \ingroup Reliver
  \ingroup Widgets

  There is a separate page describing the general design of QmitkSliceBasedSegmentation: \ref QmitkSliceBasedSegmentationTechnicalPage

  This widget graphically displays the active tool of a mitk::ToolManager as a set
  of toggle buttons. Each button show the identification of a Tool (icon and name).
  When a button's toggle state is "down", the tool is activated. When a different button
  is clicked, the active tool is switched. When you click an already active button, the
  associated tool is deactivated with no replacement, which means that no tool is active 
  then.

  When this widget is enabled/disabled it (normally) also enables/disables the tools. There
  could be cases where two QmitkToolSelectionBox widgets are associated to the same ToolManager, 
  but if this happens, please look deeply into the code.

  Last contributor: $Author$
*/
class QMITK_EXPORT QmitkToolSelectionBox : public QButtonGroup
{
  Q_OBJECT

  public:
 
    enum EnabledMode { EnabledWithReferenceAndWorkingData, EnabledWithReferenceData, EnabledWithWorkingData, AlwaysEnabled };
    
    QmitkToolSelectionBox(QWidget* parent = 0, const char* name = 0);
    virtual ~QmitkToolSelectionBox();

    mitk::ToolManager* GetToolManager();
    void SetToolManager(mitk::ToolManager&); // no NULL pointer allowed here, a manager is required
    
    /**
      You may specify a list of tool "groups" that should be displayed in this widget. Every Tool can report its group
      as a string. This method will try to find the tool's group inside the supplied string \param toolGroups. If there is a match,
      the tool is displayed. Effectively, you can provide a human readable list like "default, lymphnodevolumetry, oldERISstuff".
    */
    void SetDisplayedToolGroups(const std::string& toolGroups = 0);

    void OnToolManagerToolModified();
    void OnToolManagerReferenceDataModified();
    void OnToolManagerWorkingDataModified();

    void OnToolErrorMessage(std::string s);
    void OnGeneralToolMessage(std::string s);

  signals:

    /// Whenever a tool is activated. id is the index of the active tool. Counting starts at 0, -1 indicates "no tool selected"
    /// This signal is also emitted, when the whole QmitkToolSelectionBox get disabled. Then it will claim ToolSelected(-1)
    /// When it is enabled again, there will be another ToolSelected event with the tool that is currently selected
    void ToolSelected(int id);

  public slots:

    virtual void setEnabled( bool );
    virtual void SetEnabledMode(EnabledMode mode);

    virtual void SetLayoutColumns(int);
    virtual void SetShowNames(bool);
    virtual void SetGenerateAccelerators(bool);

    virtual void SetToolGUIArea( QWidget* parentWidget );
  
  protected slots:

    void toolButtonClicked(int id);
    void SetGUIEnabledAccordingToToolManagerState();
  
  protected:

    void RecreateButtons();
    void SetOrUnsetButtonForActiveTool();

    mitk::ToolManager::Pointer m_ToolManager;

    bool m_SelfCall;
    
    bool m_Enabled;

    std::string m_DisplayedGroups;

    /// stores relationship between button IDs of the Qt widget and tool IDs of ToolManager
    std::map<int,int> m_ButtonIDForToolID;
    /// stores relationship between button IDs of the Qt widget and tool IDs of ToolManager
    std::map<int,int> m_ToolIDForButtonID;

    int  m_LayoutColumns;
    bool m_ShowNames;
    bool m_GenerateAccelerators;

    QWidget* m_ToolGUIWidget;
    QmitkToolGUI* m_LastToolGUI;

    EnabledMode m_EnabledMode;
};

#endif

