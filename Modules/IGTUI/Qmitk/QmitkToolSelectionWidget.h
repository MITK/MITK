/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-03-21 19:27:37 +0100 (Sa, 21 Mrz 2009) $
Version:   $Revision: 16719 $ 

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _QmitkToolSelectionWidget_H_INCLUDED
#define _QmitkToolSelectionWidget_H_INCLUDED

#include "ui_QmitkToolSelectionWidgetControls.h"
#include "MitkIGTUIExports.h"

#include <QStringList>


/*!
\brief QmitkToolSelectionWidget

Widget for tool selection in an IGT Plugin. Provides a combobx which can be filled with the tool names ( SetToolNames() or AddToolName() ) of a tracking source and a checkbox 
whose text can be set with AddCheckBoxText(). Toggeling of the checkbox should be used to activate or inactivate a specific action for the selected tool in the IGT Plugin.

*/
class MitkIGTUI_EXPORT QmitkToolSelectionWidget : public QWidget
{  
  Q_OBJECT // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
public: 


  /*!  
  \brief default constructor  
  */ 
  QmitkToolSelectionWidget( QWidget* parent );

  /*!  
  \brief default destructor  
  */ 
  virtual ~QmitkToolSelectionWidget();

  /*!
  \brief This method returns the current selected index from the tool combobox.
  */
  int GetCurrentSelectedIndex();

  /*!
  \brief This method sets the list with names of the available tools to the combobox. This method should be used after the initilization of the tracking source. For correct use make sure that the tool names are in the same order as the tools from the tracking source.
  */
  void SetToolNames( const QStringList& toolNames );

  /*!
  \brief This method adds a single tool name at the end of the tool combobox. This method should be used after a tool has been added manually to the tracking source.
  */
  void AddToolName( const QString& toolName);

 
  /*!
  \brief This method changes the tool name in the combobox at the given position.
  */
  void ChangeToolName( int index, const QString& toolName );

  /*!
  \brief This method removes a single tool name from the combobox by name.
  */
  void RemoveToolName( const QString& toolName );

  /*!
  \brief This method removes a single tool name from the combobox by index.
  */
  void RemoveToolName( int index );

  /*!
  \brief This method clears all tool names from the combobox.
  */
  void ClearToolNames();

  /*!
  \brief This method sets the text of the use tool checkbox.
  */
  void SetCheckboxtText( const QString& text);
  
  /*!
  \brief This method returns whether the use tool checkbox is checked or not. 
  */
  bool IsSelectedToolActivated();

signals:
   /*!
  \brief This signal is emitted when the checkbox is toggled. It provides the current selected tool id and whether it has been selected or deselected.
  */
  void SignalUseTool(int index, bool use);
   /*!
  \brief This signal is emitted when a different tool is selected in the combo box.
  */
  void SignalSelectedToolChanged(int index);

public slots:
   /*!
  \brief Enables this widget.
  */
    void EnableWidget();
     /*!
  \brief Disables this widget.
  */
    void DisableWidget();


    protected slots:
      /*!
  \brief Slot which emits the SingalUseTool() after providing it with the tool id. 
  */
      void CheckBoxToggled(bool checked);


protected:
/*!
  \brief Creates this widget's signal slot connections.
  */
  void CreateConnections();
  void CreateQtPartControl( QWidget *parent );
  Ui::QmitkToolSelectionWidgetControls* m_Controls;  ///< gui widgets

};
#endif // _QmitkToolSelectionWidget_H_INCLUDED

