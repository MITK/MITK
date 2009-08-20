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
* \brief The QmitkRenderWindowMenu is a popup Widget which shows up when the mouse curser enter a QmitkRenderWindow. 
* The Menu Widget is located in the right top corner of each RenderWindow. It includes different settings. For example 
* the layout design can be changed with the setting button. Switching between full-screen mode and layout design can be done
* with the full-screen button. Splitting the Widget horizontal or vertical as well closing the Widget is not implemented yet. 
* The popup Widget can be deactivated with ActivateMenuWidget(false) in QmitkRenderWindow.
*
* \ingroup Renderer
* 
* \sa QmitkRenderWindow
* \sa QmitkStdMultiWidget
* 
*/

class QMITK_EXPORT QmitkRenderWindowMenu : public QWidget  
{
  Q_OBJECT

public:

  QmitkRenderWindowMenu( QWidget* parent = 0, Qt::WFlags f = 0 );
  virtual ~QmitkRenderWindowMenu();

  /*! Return visibility of settings menu. The menu is connected with m_SettingsButton and includes 
  layout direction (transversal, coronal .. ) and layout design (standard layout, 2D images top, 
  3D bottom ... ). */
  bool GetSettingsMenuVisibilty()
  { 
    if( m_Settings == NULL)
      return false;
    else
      return m_Settings->isVisible();  
  }

  /*! Set layout index. Defines layout direction (transversal, coronal, sagital or threeD) of the parent. */
  void SetLayoutIndex( unsigned int layoutIndex );
  
  /*! Return layout direction of parent (transversal, coronal, sagital or threeD) */
  unsigned int GetLayoutIndex()
  {  return m_Layout;  }

  /*! Update list of layout design (standard layout, 2D images top, 3D bottom ..). Set action of current layout design 
  to disable and all other to enable. */
  void UpdateLayoutDesignList( int layoutDesignIndex );

  /*! Move menu widget to correct position (right upper corner). E.g. it is necessary when the full-screen mode
  is activated.*/
  void MoveWidgetToCorrectPos();


protected:

  /*! Create menu widget. The menu contains five QPushButtons (hori-split, verti-split, full-screen, settings and close button)
  and their signal/slot connection for handling.  */
  void CreateMenuWidget();

   
  /*! Create settings menu which contains layout direction and the different layout designs. */
  void CreateSettingsWidget();

  /*! Reimplemented from QWidget. The paint event is a request to repaint all or part of a widget.*/
  void paintEvent(QPaintEvent *event);

  /*! Update list of layout direction (transversal, coronal, sagital or threeD). Set action of currect layout direction
  to disable and all other to enable. Normaly the user can switch here between the different layout direction, but 
  this is not supported yet. */
  void UpdateLayoutList();

  /*! Change Icon of full-screen button depending on full-screen mode. */
  void ChangeFullScreenIcon();


signals:
  
  void ShowCrosshair(bool show);
  void ResetView(); // == "global reinit"
  
  // \brief int parameters are enum from QmitkStdMultiWidget
  void ChangeCrosshairRotationMode(int); 
  void SetCrosshairRotationLinked(bool);

  /*! emit signal, when layout design changed by the setting menu.*/
  void SignalChangeLayoutDesign( int layoutDesign );

protected slots:  

  void OnCrosshairRotationModeSelected(QAction*); 

  /*! slot for horizontal splitting of the current widget. The slot is connected to the clicked() event of m_HoriSplitButton. 
  The method is not implemented yet. */
  void OnHoriSplitButton( bool checked );

  /*! slot for vertical splitting of the current widget. The slot is connected to the clicked() event of m_VertiSplitButton. 
  The method is not implemented yet. */
  void OnVertiSplitButton( bool checked );
  
  /*! slot for activating/deactivating the full-screen mode. The slot is connected to the clicked() event of m_FullScreenButton. 
  Activating the full-screen maximize the current widget, deactivating restore If layout design changed by the settings menu, 
  the full-Screen mode is automatically switch to false. */
  void OnFullScreenButton( bool checked );
 
  /*! Slot for opening setting menu. The slot is connected to the clicked() event of m_SettingsButton. 
  The settings menu includes differen layout directions (transversal, coronal, saggital and 3D) as well all layout design 
  (standard layout, 2D images top, 3D bottom ..)*/
  void OnSettingsButton( bool checked );

  /*! Slot for closing widget. The slot is connected to the clicked() event of m_CloseButton.
  The method is not implemented yet.*/
  void OnCloseButton( bool checked );

  /*! Slot for changing layout diretion to transversal. The slot is conntect to the triggered() signal of m_TransversalAction.
  The method is not implemented yet.*/
  void OnChangeDirectionToTransversal(bool);

  /*! Slot for changing layout diretion to sagittal. The slot is conntect to the triggered() signal of m_SagittalAction.
  The method is not implemented yet.*/
  void OnChangeDirectionToSagittal(bool);

  /*! Slot for changing layout diretion to coronal. The slot is conntect to the triggered() signal of m_CoronalAction.
  The method is not implemented yet.*/
  void OnChangeDirectionToCoronal(bool);

  /*! Slot for changing layout diretion to 3D. The slot is conntect to the triggered() signal of m_ThreeDAction.
  The method is not implemented yet.*/
  void OnChangeDirectionToThreeD(bool);

  /*! Slot for changing layout design to standard layout. The slot is connected to the triggered() signal of m_DefaultLayoutAction. */
  void OnChangeLayoutToDefault(bool);

  /*! Slot for changing layout design to 2D images top, 3D bottom layout. The slot is connected to the triggered() signal of m_2DImagesUpLayoutAction. */
  void OnChangeLayoutTo2DImagesUp(bool);

  /*! Slot for changing layout design to 2D images left, 3D right layout. The slot is connected to the triggered() signal of m_2DImagesLeftLayoutAction. */
  void OnChangeLayoutTo2DImagesLeft(bool);

  /*! Slot for changing layout to Big 3D layout. The slot is connected to the triggered() signal of m_Big3DLayoutAction. */
  void OnChangeLayoutToBig3D(bool);

  /*! Slot for changing layout design to Transversal plane layout. The slot is connected to the triggered() signal of m_Widget1LayoutAction. */
  void OnChangeLayoutToWidget1(bool);

  /*! Slot for changing layout design to Sagittal plane layout. The slot is connected to the triggered() signal of m_Widget2LayoutAction. */
  void OnChangeLayoutToWidget2(bool);

  /*! Slot for changing layout design to Coronal plane layout. The slot is connected to the triggered() signal of m_Widget3LayoutAction. */
  void OnChangeLayoutToWidget3(bool);

  /*! Slot for changing layout design to Coronal top, 3D bottom layout. The slot is connected to the triggered() signal of m_RowWidget3And4LayoutAction. */
  void OnChangeLayoutToRowWidget3And4(bool);

  /*! Slot for changing layout design to Coronal left, 3D right layout. The slot is connected to the triggered() signal of m_ColumnWidget3And4LayoutAction. */
  void OnChangeLayoutToColumnWidget3And4(bool);

  /*! Slot for changing layout design to Sagittal top, Coronal n 3D bottom layout. The slot is connected to the triggered() signal of m_SmallUpperWidget2Big3and4LayoutAction. */
  void OnChangeLayoutToSmallUpperWidget2Big3and4(bool);

  /*! Slot for changing layout design to Transversal n Sagittal left, 3D right layout. The slot is connected to the triggered() signal of m_2x2Dand3DWidgetLayoutAction. */
  void OnChangeLayoutTo2x2Dand3DWidget(bool);

  /*! Slot for changing layout design to Transversal n 3D left, Sagittal right layout. The slot is connected to the triggered() signal of m_Left2Dand3DRight2DLayoutAction. */
  void OnChangeLayoutToLeft2Dand3DRight2D(bool);

public:
  
  /*! enum for layout direction*/
  enum {
    TRANSVERSAL,
    SAGITTAL,
    CORONAL,
    THREE_D
  };


  /*! enum for layout design */
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

  /*! QPushButton for split widget horizontal*/
  QPushButton*        m_CrosshairModeButton;
  QPushButton*        m_HoriSplitButton;
  
  /*! QPushButton for split widget vertical*/
  QPushButton*        m_VertiSplitButton;

  /*! QPushButton for activating/deactivating full-screen mode*/
  QPushButton*        m_FullScreenButton;

  /*! QPushButton for open the settings menu*/
  QPushButton*        m_SettingsButton;

  /*! QPushButton for closing the widget*/
  QPushButton*        m_CloseButton;
  
  /*! QAction for transversal view (layout direction) */
  QAction*            m_TransversalAction;

  /*! QAction for saggital view (layout direction) */
  QAction*            m_SagittalAction;

  /*! QAction for coronal view (layout direction) */
  QAction*            m_CoronalAction;

  /*! QAction for 3D view (layout direction) */
  QAction*            m_ThreeDAction;

  /*! QAction for Default layout design */
  QAction*            m_DefaultLayoutAction;
  
  /*! QAction for 2D images up layout design */
  QAction*            m_2DImagesUpLayoutAction;

  /*! QAction for 2D images left layout design */
  QAction*            m_2DImagesLeftLayoutAction;

  /*! QAction for big 3D layout design */
  QAction*            m_Big3DLayoutAction;

  /*! QAction for big transversal layout design */
  QAction*            m_Widget1LayoutAction;

  /*! QAction for big saggital layout design */
  QAction*            m_Widget2LayoutAction;

  /*! QAction for big coronal layout design */
  QAction*            m_Widget3LayoutAction;

  /*! QAction for coronal top, 3D bottom layout design */
  QAction*            m_RowWidget3And4LayoutAction;

  /*! QAction for coronal left, 3D right layout design */
  QAction*            m_ColumnWidget3And4LayoutAction;

  /*! QAction for sagittal top, coronal n 3D bottom layout design */
  QAction*            m_SmallUpperWidget2Big3and4LayoutAction;

  /*! QAction for transversal n sagittal left, 3D right layout design */
  QAction*            m_2x2Dand3DWidgetLayoutAction;

  /*! QAction for transversal n 3D left, sagittal right layout design*/
  QAction*            m_Left2Dand3DRight2DLayoutAction;


  /*! QMenu containg all layout direction and layout design settings.*/
  QMenu*              m_Settings;

  /*! Index of layout direction. 0: transversal; 1: saggital; 2: coronal; 3: threeD */
  unsigned int        m_Layout;

  /*! Index of layout design. 0: LAYOUT_DEFAULT; 1: LAYOUT_2DIMAGEUP; 2: LAYOUT_2DIMAGELEFT; 3: LAYOUT_BIG3D
  4: LAYOUT_TRANSVERSAL; 5: LAYOUT_SAGITTAL; 6: LAYOUT_CORONAL; 7: LAYOUT_2X2DAND3DWIDGET; 8: LAYOUT_ROWWIDGET3AND4;
  9: LAYOUT_COLUMNWIDGET3AND4; 10: LAYOUT_ROWWIDGETSMALL3ANDBIG4; 11: LAYOUT_SMALLUPPERWIDGET2BIGAND4; 12: LAYOUT_LEFT2DAND3DRIGHT2D */
  unsigned int        m_LayoutDesign;

  /*! Store index of old layout design. It is used e.g. for the full-screen mode, when deactivating the mode the former layout design will restore.*/
  unsigned int        m_OldLayoutDesign;

  /*! Flag if full-screen mode is activated or deactivated. */
  bool                m_FullScreenMode;
};

#endif // QmitkRenderWindowMenu_H
