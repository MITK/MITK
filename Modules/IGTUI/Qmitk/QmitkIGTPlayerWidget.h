/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:14:45 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 1.12 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkIGTPlayerWidget_H
#define QmitkIGTPlayerWidget_H

//QT headers
#include <QWidget>

//mitk headers
#include "MitkIGTUIExports.h"
#include "mitkNavigationTool.h"
#include "mitkNavigationDataPlayer.h"
#include <mitkTimeStamp.h>



//ui header
#include "ui_QmitkIGTPlayerWidgetControls.h"

/** Documentation:
*   \brief GUI to access the IGT Player. 
*   User must specify the file name where the input xml-file is located. The NavigationDatas from the xml-file can be 
*   played in normal mode or in PointSet mode. 
*   
*   In normal mode the player updates the NavigationDatas every 100ms (can be changed in SetUpdateRate()) and returns
*   them when GetNavigationDatas() is called.
*   In PointSet mode the player generates a PointSet with all NavigationDatas from the xml-file. So the playback is
*   performed on this ND PointSet.
*
*   \ingroup IGTUI
*/
class MitkIGTUI_EXPORT QmitkIGTPlayerWidget : public QWidget
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  /*!  
  \brief default constructor  
  */ 
  QmitkIGTPlayerWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);    

  /*!  
  \brief default deconstructor  
  */ 
  ~QmitkIGTPlayerWidget();

  /*!  
  \brief Sets the player for this player widget 
  */ 
  void SetPlayer(mitk::NavigationDataPlayer::Pointer player);

  /*!  
  \brief Returns the playing timer of this widget  
  */ 
  QTimer* GetPlayingTimer();

  /*!  
  \brief Returns the current playback NavigationDatas from the xml-file
  */ 
  const std::vector<mitk::NavigationData::Pointer> GetNavigationDatas();

  /*!  
  \brief Sets the widget's look for the normal playback
  */ 
  void SetWidgetViewToNormalPlayback();

  /*!  
  \brief Sets the widget's look for the PointSet playback
  */ 
  void SetWidgetViewToPointSetPlayback();


  /*!  
  \brief Sets the update rate of this widget's playing timer
  */ 
  void SetUpdateRate(unsigned int msecs);


  /*!
  \brief Returns the number of different tools from the current playing stream.
  *
  * Retuns 0 if playback file is invalid.
  */
  unsigned int GetNumberOfTools();

  /*!  
  \brief Stops the playback
  */ 
  void StopPlaying();

 

signals:
  /*!  
  \brief This signal is emitted when the player starts the playback
  */ 
  void PlayingStarted();
  /*!  
  \brief This signal is emitted when the player resumes after a pause
  */ 
  void PlayingResumed();
  /*!  
  \brief This signal is emitted when the player stops
  */ 
  void PlayingStopped();
  /*!  
  \brief This signal is emitted when the player is paused
  */ 
  void PlayingPaused();
  /*!  
  \brief This signal is emitted when the player reaches the end of the playback
  */ 
  void PlayingEnded();

  /*!  
  \brief This signal is emitted every time the player updated the NavigationDatas
  */ 
  void PlayerUpdated();

  
  void InputFileChanged();




  protected slots:   
    /*!  
    \brief Starts or pauses the playback
    */ 
    void OnPlayButtonClicked(bool toggled);    
    /*!  
    \brief Updates the playback data
    */ 
    void OnPlaying();   
    /*!  
    \brief Stops the playback
    */  
    void OnStopPlaying();
    /*!  
    \brief Updates the input filename
    */ 
    void SetInputFileName(const QString& inputFileName);
    /*!  
    \brief Opens file open dialog for searching the input file
    */ 
    void OnSelectPressed();
    /*!  
    \brief Stops the playback
    */ 
    void OnGoToEnd();
    /*!  
    \brief Stops the playback and resets the player to the beginning
    */ 
    void OnGoToBegin();
    /*!  
    \brief Switches between the normal playback view and the PointSet playback view
    */ 
    void OnChangeWidgetView(bool pointSetPlaybackView);



protected:

  /// \brief Creation of the connections
  virtual void CreateConnections();


  virtual void CreateQtPartControl(QWidget *parent);

  /*!  
  \brief Checks if an imput file with the set filename exists
  */ 
  bool CheckInputFileValid();

  /*!  
  \brief Sets all LCD numbers to 0
  */ 
  void ResetLCDNumbers();

  Ui::QmitkIGTPlayerWidgetControls* m_Controls;

  mitk::NavigationDataPlayer::Pointer m_Player; ///< plays NDs from a XML file

  QString m_CmpFilename; ///< filename of the input file
  QTimer* m_PlayingTimer; ///< update timer

  mitk::NavigationData::TimeStampType m_StartTime; ///< start time of playback needed for time display


};
#endif