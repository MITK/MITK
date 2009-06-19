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

  void SetLayoutIndex( unsigned int layoutIndex );
  
  unsigned int GetLayoutIndex()
  {  return m_Layout;  }

  /** */
  void UpdateLayoutDesignList( int layoutDesignIndex );

protected:

  /** */
  void CreateMenuWidget();

  /** */
  void CreateMenuBar();

  /** */
  void CreateSettingsWidget();

  /** */
  void paintEvent(QPaintEvent *event);

  /** */
  void UpdateLayoutList();

signals:
  
  /** */
  void SignalChangeLayoutDesign( int layoutDesign );

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


  /// \brief change Layout slots
  void OnChangeDirectionToTransversal(bool);
  void OnChangeDirectionToSagittal(bool);
  void OnChangeDirectionToCoronal(bool);
  void OnChangeDirectionToThreeD(bool);

  /// \brief change Layout slots
  void OnChangeLayoutToDefault(bool);
  void OnChangeLayoutTo2DImagesUp(bool);
  void OnChangeLayoutTo2DImagesLeft(bool);
  void OnChangeLayoutToBig3D(bool);
  void OnChangeLayoutToWidget1(bool);
  void OnChangeLayoutToWidget2(bool);
  void OnChangeLayoutToWidget3(bool);
  void OnChangeLayoutToRowWidget3And4(bool);
  void OnChangeLayoutToColumnWidget3And4(bool);
  void OnChangeLayoutToSmallUpperWidget2Big3and4(bool);
  void OnChangeLayoutTo2x2Dand3DWidget(bool);
  void OnChangeLayoutToLeft2Dand3DRight2D(bool);

public:
  
  //enum for layout direction
  enum {
    TRANSVERSAL,
    SAGITTAL,
    CORONAL,
    THREE_D
  };

  //enum for layout design
  enum {
    LAYOUT_DEFAULT,
    LAYOUT_2DIMAGEUP,
    LAYOUT_2DIMAGELEFT,
    LAYOUT_BIG3D,
    LAYOUT_TRANSVERSAL,
    LAYOUT_SAGITTAL,
    LAYOUT_CORONAL,
    LAYOUT_2X2DAND3DWIDGET,
    LAYOUT_ROWWIDGET3AND4,
    LAYOUT_COLUMNWIDGET3AND4,
    LAYOUT_ROWWIDGETSMALL3ANDBIG4, //not in use in this class, but we need it here to synchronize with the SdtMultiWidget.
    LAYOUT_SMALLUPPERWIDGET2BIGAND4,
    LAYOUT_LEFT2DAND3DRIGHT2D
  };


protected:

  /** */
  QPushButton*        m_HoriSplitButton;
  QPushButton*        m_VertiSplitButton;
  QPushButton*        m_FullScreenButton;
  QPushButton*        m_SettingsButton;
  QPushButton*        m_CloseButton;

  /** WidgetLayout */
  QAction* m_TransversalAction;
  QAction* m_SagittalAction;
  QAction* m_CoronalAction;
  QAction* m_ThreeDAction;

  /** Layouts */
  QAction* m_DefaultLayoutAction;
  QAction* m_2DImagesUpLayoutAction;
  QAction* m_2DImagesLeftLayoutAction;
  QAction* m_Big3DLayoutAction;
  QAction* m_Widget1LayoutAction;
  QAction* m_Widget2LayoutAction;
  QAction* m_Widget3LayoutAction;
  QAction* m_RowWidget3And4LayoutAction;
  QAction* m_ColumnWidget3And4LayoutAction;
  QAction* m_SmallUpperWidget2Big3and4LayoutAction;
  QAction* m_2x2Dand3DWidgetLayoutAction;
  QAction* m_Left2Dand3DRight2DLayoutAction;


  /** */
  QMenuBar*           m_MenuBar; 

  /** */
  QMenu*              m_Settings;

  /** */
  unsigned int        m_Layout;

  /** */
  unsigned int        m_LayoutDesign;

  /** */
  unsigned int        m_OldLayoutDesign;

  /** */
  bool                m_FullScreenMode;

};

#endif // QmitkRenderWindowMenu_H
