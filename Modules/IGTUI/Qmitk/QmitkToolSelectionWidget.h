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

Widget with combobox for selection of a tool and button for triggering an action.

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
  \brief This method sets the list of available tools to the combobox.
  */
  void SetToolNames( const QStringList& toolNames );

  /*!
  \brief This method starts the timer if it is not already active.
  */
  void AddToolName( const QString& toolName);

 
  /*!
  \brief This method starts the timer if it is not already active. 
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
  \brief This method sets the text for the use tool checkbox.
  */
  void SetCheckboxtText( const QString& text);
  
  /*!
  \brief This method returns whether the use tool checkbox is checked or not. 
  */
  bool IsSelectedToolActivated();

signals:
  void SignalUseTool(int index, bool use);
  void SignalSelectedToolChanged(int index);

public slots:
    void EnableWidget();
    void DisableWidget();


    protected slots:
      void CheckBoxToggled(bool checked);


protected:
  void CreateConnections();
  void CreateQtPartControl( QWidget *parent );
  Ui::QmitkToolSelectionWidgetControls* m_Controls;  ///< gui widgets

};
#endif // _QmitkToolSelectionWidget_H_INCLUDED

