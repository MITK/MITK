/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 20:04:59 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 17180 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkRenderWindowMenu_h
#define QmitkRenderWindowMenu_h


#include "mitkCommon.h"

#include <QWidget>
#include <QEvent>
#include <QPushButton>
#include <QMenuBar>
#include <QAction>


/**
* \brief MITK implementation of the QVTKWidget
* \ingroup Renderer
*/

class QMITK_EXPORT QmitkRenderWindowMenu : public QWidget  
{
  Q_OBJECT

public:

  QmitkRenderWindowMenu( QWidget* parent = 0, Qt::WFlags f = 0 );
  virtual ~QmitkRenderWindowMenu();

  bool GetSettingsMenuVisibilty()
  { 
    if( m_Settings == NULL)
      return false;
    else
      return m_Settings->isVisible();  
  }


protected:

  /** */
  void CreateMenuWidget();

  /** */
  void CreateMenuBar();

  /** */
  void CreateSettingsWidget();

  /** */
  void paintEvent(QPaintEvent *event);


protected slots:  

  /// \brief
  void OnHoriSplitButton( bool checked );

  /// \brief
  void OnVertiSplitButton( bool checked );
  
  /// \brief
  void OnFullScreenButton( bool checked );

  /// \brief
  void OnSettingsButton( bool checked );

  /// \brief
  void OnCloseButton( bool checked );

protected:

  /** */
  QPushButton*        m_HoriSplitButton;
  QPushButton*        m_VertiSplitButton;
  QPushButton*        m_FullScreenButton;
  QPushButton*        m_SettingsButton;
  QPushButton*        m_CloseButton;


  /** */
  QMenuBar*           m_MenuBar; 

  /** */
  QMenu*              m_Settings;


};

#endif // QmitkRenderWindowMenu_H
