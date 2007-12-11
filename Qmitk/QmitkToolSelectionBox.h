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

#include "qbuttongroup.h"

#include "mitkToolManager.h"

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
class QmitkToolSelectionBox : public QButtonGroup
{
  Q_OBJECT

  public:
    
    QmitkToolSelectionBox(QWidget* parent = 0, const char* name = 0);
    virtual ~QmitkToolSelectionBox();

    mitk::ToolManager* GetToolManager();
    void SetToolManager(mitk::ToolManager&); // no NULL pointer allowed here, a manager is required

    void OnToolManagerToolModified(const itk::EventObject&);
    void OnToolManagerReferenceDataModified(const itk::EventObject&);
    void OnToolManagerWorkingDataModified(const itk::EventObject&);

    void SetAlwaysEnabled(bool alwaysEnabled);

  signals:

    /// Whenever a tool is activated. id is the index of the active tool. Counting starts at 0, -1 indicates "no tool selected"
    /// This signal is also emitted, when the whole QmitkToolSelectionBox get disabled. Then it will claim ToolSelected(-1)
    /// When it is enabled again, there will be another ToolSelected event with the tool that is currently selected
    void ToolSelected(int id);

  public slots:

    virtual void setEnabled( bool );
  
  protected slots:

    void toolButtonClicked(int id);
    void SetGUIEnabledAccordingToToolManagerState();
  
  protected:

    void RecreateButtons();

    mitk::ToolManager::Pointer m_ToolManager;

    bool m_SelfCall;
    
    unsigned long m_ToolSelectedObserverTag;
    unsigned long m_ToolReferenceDataChangedObserverTag;
    unsigned long m_ToolWorkingDataChangedObserverTag;

    bool m_Enabled;

    bool m_AlwaysEnabledOverride;
    
};

#endif

