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

#ifndef _QMITKTOFRECORDERWIDGET_H_INCLUDED
#define _QMITKTOFRECORDERWIDGET_H_INCLUDED

#include <MitkToFUIExports.h>
#include <ui_QmitkToFRecorderWidgetControls.h>

//QT headers
#include <QWidget>
#include <QString>
#include <QDialog>
#include <QFileDialog>

//itk headers
#include "itkCommand.h"

//mitk headers
#include <mitkToFImageGrabber.h>
#include <mitkToFImageRecorder.h>

class QmitkStdMultiWidget;

struct QFileDialogArgs;
class QFileIconProvider;
class QFileDialogPrivate;

/**
* @brief Widget allowing to play / record ToF data
*
* @ingroup ToFUI
*/
class MitkToFUI_EXPORT QmitkToFRecorderWidget :public QWidget
{

  //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkToFRecorderWidget(QWidget* p = 0, Qt::WindowFlags f1 = 0);
    virtual ~QmitkToFRecorderWidget();

    /* @brief This method is part of the widget an needs not to be called seperately. */
    virtual void CreateQtPartControl(QWidget *parent);
    /* @brief This method is part of the widget an needs not to be called seperately. (Creation of the connections of main and control widget.)*/
    virtual void CreateConnections();

    /*!
    \brief Set the parameters used for this widget
    \param ToFImageGrabber image grabber providing images from a ToF device
    \param tofImageRecorder image recorder allowing to record ToF images
    */
    void SetParameter(mitk::ToFImageGrabber* ToFImageGrabber,  mitk::ToFImageRecorder* toFImageRecorder);
    /*!
    \brief resets the GUI elements to the initial state. Play button: enabled, Stop button: disabled, Recording box: disabled
    */
    void ResetGUIToInitial();

  signals:
    /*!
    \brief signal emitted when "Play" button is pressed
    */
    void ToFCameraStarted();
    /*!
    \brief signal emitted when "Stop" button is pressed
    */
    void ToFCameraStopped();
    /*!
    \brief signal emitted when recording is started
    */
    void RecordingStarted();
    /*!
    \brief signal emitted AbortEvent() in ToFImageRecorder is observed
    */
    void RecordingStopped();

  public slots:
    /*!
    \brief slot invoking to start the camera.
    Calls StartCamera() and emits ToFCameraStarted signal
    */
    void OnPlay();
    /*!
    \brief slot invoking to stop the camera and the recorder.
    Calls StopCamera() and StopRecorder and emits ToFCameraStarted signal. Resets GUI to initial state.
    */
    void OnStop();
    /*!
    \brief slot invoking to start the recording
    After letting the user chose a file location for the record, m_ImageRecorder->StartRecording() is inoved.
    */
    void OnStartRecorder();
    /*!
    \brief slot resetting the GUI elements of the recording box
    */
    void OnRecordingStopped();
    /*!
    \brief slot activating/deactivating "number of frames" spin box dependent on recording mode (PerFrame / Infinite)
    */
    void OnChangeRecordModeComboBox(int index);

  protected:

    /*!
    \brief starts the camera by calling ToFImageGrabber::StartCamera()
    */
    void StartCamera();
    /*!
    \brief stops the camera by calling ToFImageGrabber::StopCamera()
    */
    void StopCamera();
    /*!
    \brief stops the recording by calling ToFImageRecorder::StopRecording()
    */
    void StopRecorder();
    /*!
    \brief emits RecordingStopped signal.
    */
    void StopRecordingCallback();
    /*!
    \brief adapted version of QFileDialog::getSaveFileName()
    The user is now asked to choose which images he wants to save
    (Distance and/or Intensity and/or Amplitude image) and which type the saved
    image should have (3D, 2D+t).
    */
    static QString getSaveFileName(mitk::ToFImageWriter::ToFImageType& tofImageType,
                                   bool& distanceImageSelected,
                                   bool& amplitudeImageSelected,
                                   bool& intensityImageSelected,
                                   bool& rgbImageSelected,
                                   bool& rawDataSelected,
                                   QWidget *parent = 0,
                                   const QString &caption = QString(),
                                   const QString &dir = QString(),
                                   const QString &filter = QString(),
                                   QString *selectedFilter = 0,
                                   QFileDialog::Options options = 0
                                   );
    /*!
    \brief method creating a filename from the given information
    \param dir directory to save the file
    \param baseFilename base file name entered by the user
    \param modulationFreq modulation frequency of the camera
    \param integrationTime integration time of the camera
    \param numOfFrames number of frames recorded
    \param extension file extension
    \param imageType type of image (DistanceImage, IntensityImage, AmplitudeImage)
    \return dir+"/"+baseFilename+"_MF"+modulationFreq+"_IT"+integrationTime+"_"+numOfFrames+"Images"+imageType+extension
    */
    std::string prepareFilename(std::string dir,
                                std::string baseFilename,
                                std::string modulationFreq,
                                std::string integrationTime,
                                std::string numOfFrames,
                                std::string extension,
                                std::string imageType);

    Ui::QmitkToFRecorderWidgetControls* m_Controls; ///< member holding the UI elements of this widget

    mitk::ToFImageGrabber::Pointer m_ToFImageGrabber; ///< member holding the ToFImageGrabber for acquiring ToF images
    mitk::ToFImageRecorder::Pointer m_ToFImageRecorder; ///< member holding the recorder for ToF images

    mitk::ToFImageRecorder::RecordMode m_RecordMode; ///< member holding the RecordMode of the recorder (PerFrame / Infinite)

    typedef itk::SimpleMemberCommand<QmitkToFRecorderWidget> CommandType;
    CommandType::Pointer m_StopRecordingCommand; ///< itkCommand for abort of recording

  private:

};

#endif // _QMITKTOFRECORDERWIDGET_H_INCLUDED
