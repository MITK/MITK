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

#ifndef _QmitkUpdateTimerWidget_H_INCLUDED
#define _QmitkUpdateTimerWidget_H_INCLUDED

#include "ui_QmitkUpdateTimerWidgetControls.h"
#include "MitkIGTUIExports.h"


/*!
\brief QmitkUpdateTimerWidget

Widget for setting up and controlling an update timer in an IGT-Pipeline.

*/
class MitkIGTUI_EXPORT QmitkUpdateTimerWidget : public QWidget
{
  Q_OBJECT // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
public:


  enum WidgetButtons {

      StartButton,
      StopButton
  };

  /*!
  \brief default constructor
  */
  QmitkUpdateTimerWidget( QWidget* parent );

  /*!
  \brief default destructor
  */
  virtual ~QmitkUpdateTimerWidget();

  /*!
  \brief This method returns the timer's timeout interval in msec.
  */
  unsigned int GetTimerInterval();

  /*!
  \brief This method sets the timer's timeout interval in msec.
  */
  void SetTimerInterval( unsigned int msec );

  /*!
  \brief This method starts the timer if it is not already active.
  */
  void StartTimer();

  /*!
  \brief This method stops the timer if it is active at the moment.
  */
  void StopTimer();

  /*!
  \brief This method returns this object's timer.
  */
  QTimer* GetUpdateTimer();

  /*!
  \brief This method sets the given QString for the purpose of this update timer e.g. if "Navigation" is given, the start and stop button will be labeled "Start Navigation" and "Stop Navigation". Furthermore the purpose description is used for the timer status label: "Navigation started ... " in this case.
  */
  void SetPurposeLabelText( QString text );

  /*!
  \brief This method hides the framerate settings spinbox and her labels in the view.
  */
  void HideFramerateSettings( bool hidden );


  /*!
  \brief This method sets the icon for a specific button of the widget.
  */
  void SetIcon( WidgetButtons button, const QIcon& icon );




signals:
  void Started();
  void Stopped();

public slots:
    void EnableWidget();
    void DisableWidget();

  protected slots:

    /*!
    \brief This method is called when the start button is pressed. It starts the timer using StartTimer().
    */
    void OnStartTimer();

    /*!
    \brief This method is called when the stop button is pressed. It stops the timer using StopTimer().
    */
    void OnStopTimer();

    /*!
    \brief This method is called when the value in the spinbox is changed. It updates the timer interval using SetTimerInterval( ).
    */
    void OnChangeTimerInterval( int interval );

protected:
  void CreateConnections();
  void CreateQtPartControl( QWidget *parent );
  Ui::QmitkUpdateTimerWidgetControls* m_Controls;  ///< gui widgets


private:
  /*!
  \brief This object's update timer realized by a QTimer
  */
  QTimer* m_UpdateTimer;

  /*!
  \brief This method is used to set up the update rate spinbox, min and max range are set and also the step size
  */
  void SetupUpdateRateSB( int min, int max, int step );

  /*!
  \brief This method is used to set the actual framerate (in Hz) as the framerate label text
  */
  void SetFrameRateLabel();

};
#endif // _QmitkUpdateTimerWidget_H_INCLUDED

