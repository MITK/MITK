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

#ifndef PAImageProcessing_h
#define PAImageProcessing_h

#include <mitkPhotoacousticImage.h>

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>
#include <qthread.h>

#include "ui_PAImageProcessingControls.h"

#include "mitkPhotoacousticBeamformingFilter.h"
#include "mitkPhotoacousticBeamformingSettings.h"

Q_DECLARE_METATYPE(mitk::Image::Pointer)
Q_DECLARE_METATYPE(std::string)

/*!
* \brief Plugin implementing an interface for the Photoacoustic Algorithms Module
*
*  Beamforming, Image processing as B-Mode filtering, cropping, resampling, as well as batch processing can be performed using this plugin.
*/

class PAImageProcessing : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    PAImageProcessing();

  protected slots:

    void UpperSliceBoundChanged();
    void LowerSliceBoundChanged();
    void SliceBoundsEnabled();
    
    void UseResampling();
    void UseLogfilter();
    void SetResampling();
    void UseImageSpacing();
    void UpdateImageInfo();

    /** \brief Method called when the beamforming thread finishes; 
    *  it adds the image to a new data node and registers it to the worbench's data storage
    */
    void HandleBeamformingResults(mitk::Image::Pointer image);
    /** \brief Beamforming is being performed in a separate thread to keep the workbench from freezing.
    */
    void StartBeamformingThread();

    /** \brief Method called when the B-mode filter thread finishes;
    *  it adds the image to a new data node and registers it to the worbench's data storage
    */
    void HandleBmodeResults(mitk::Image::Pointer image);
    /** \brief B-mode filtering is being performed in a separate thread to keep the workbench from freezing.
    */
    void StartBmodeThread();

    /** \brief Method called when the Cropping thread finishes;
    *  it adds the image to a new data node and registers it to the worbench's data storage
    */
    void HandleCropResults(mitk::Image::Pointer image);
    /** \brief Cropping is being performed in a separate thread to keep the workbench from freezing.
    */
    void StartCropThread();

    /** \brief Method called when the bandpass thread finishes;
    *  it adds the image to a new data node and registers it to the worbench's data storage
    */
    void HandleBandpassResults(mitk::Image::Pointer image);
    /** \brief Bandpassing is being performed in a separate thread to keep the workbench from freezing.
    */
    void StartBandpassThread();

    void UpdateProgress(int progress, std::string progressInfo);
    void PAMessageBox(std::string message);

    void BatchProcessing();
    void UpdateSaveBoxes();

    void ChangedSOSBandpass();
    void ChangedSOSBeamforming();

  protected:
    virtual void CreateQtPartControl(QWidget *parent) override;

    virtual void SetFocus() override;

    /** \brief called by QmitkFunctionality when DataManager's selection has changed.
    *  On a change some parameters are internally updated to calculate bounds for GUI elements as the slice selector for beamforming or
    *  the bandpass filter settings.
    */
    virtual void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
                                     const QList<mitk::DataNode::Pointer>& nodes ) override;

    /** \brief Instance of the GUI controls
    */
    Ui::PAImageProcessingControls m_Controls;

    float m_ResampleSpacing;
    bool m_UseLogfilter;
    std::string m_OldNodeName;

    /** \brief The settings set which is used for beamforming, updated through this class.
    */
    mitk::BeamformingSettings BFconfig;

    /** \brief Method for updating the BFconfig by using a selected image and the GUI configuration.
    */
    void UpdateBFSettings(mitk::Image::Pointer image);

    void EnableControls();
    void DisableControls();

    /** \brief Class through which the filters are called.
    */
    mitk::PhotoacousticImage::Pointer m_FilterBank;
};

class BeamformingThread : public QThread
{
  Q_OBJECT
    void run() Q_DECL_OVERRIDE;

  signals:
    void result(mitk::Image::Pointer);
    void updateProgress(int, std::string);
    void message(std::string);

  public:
    void setConfig(mitk::BeamformingSettings BFconfig);
    void setInputImage(mitk::Image::Pointer image);
    void setFilterBank(mitk::PhotoacousticImage::Pointer filterBank)
    {
      m_FilterBank = filterBank;
    }


  protected:
    mitk::BeamformingSettings m_BFconfig;
    mitk::Image::Pointer m_InputImage;
    int m_Cutoff;

    mitk::PhotoacousticImage::Pointer m_FilterBank;
};

class BmodeThread : public QThread
{
  Q_OBJECT
    void run() Q_DECL_OVERRIDE;

  signals:
    void result(mitk::Image::Pointer);

  public:
    enum BModeMethod { ShapeDetection, Abs };

    void setConfig(bool useLogfilter, double resampleSpacing, mitk::PhotoacousticImage::BModeMethod method, bool useGPU);
    void setInputImage(mitk::Image::Pointer image);
    void setFilterBank(mitk::PhotoacousticImage::Pointer filterBank)
    {
      m_FilterBank = filterBank;
    }


  protected:
    mitk::Image::Pointer m_InputImage;

    mitk::PhotoacousticImage::BModeMethod m_Method;
    bool m_UseLogfilter;
    double m_ResampleSpacing;
    bool m_UseGPU;

    mitk::PhotoacousticImage::Pointer m_FilterBank;
};

class CropThread : public QThread
{
  Q_OBJECT
    void run() Q_DECL_OVERRIDE;

signals:
  void result(mitk::Image::Pointer);

public:
  void setConfig(unsigned int CutAbove, unsigned int CutBelow, unsigned int CutSliceFirst, unsigned int CutSliceLast);
  void setInputImage(mitk::Image::Pointer image);
  void setFilterBank(mitk::PhotoacousticImage::Pointer filterBank)
  {
    m_FilterBank = filterBank;
  }

protected:
  mitk::Image::Pointer m_InputImage;

  unsigned int m_CutAbove;
  unsigned int m_CutBelow;
  unsigned int m_CutSliceLast;
  unsigned int m_CutSliceFirst;

  mitk::PhotoacousticImage::Pointer m_FilterBank;
};


class BandpassThread : public QThread
{
  Q_OBJECT
    void run() Q_DECL_OVERRIDE;

signals:
  void result(mitk::Image::Pointer);

public:
  void setConfig(float BPHighPass, float BPLowPass, float TukeyAlpha, float recordTime);
  void setInputImage(mitk::Image::Pointer image);
  void setFilterBank(mitk::PhotoacousticImage::Pointer filterBank)
  {
    m_FilterBank = filterBank;
  }

protected:
  mitk::Image::Pointer m_InputImage;

  float m_BPHighPass;
  float m_BPLowPass;
  float m_TukeyAlpha;
  float m_RecordTime;

  mitk::PhotoacousticImage::Pointer m_FilterBank;
};

#endif // PAImageProcessing_h
