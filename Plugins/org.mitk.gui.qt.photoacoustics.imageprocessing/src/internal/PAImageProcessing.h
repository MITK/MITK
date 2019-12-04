/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef PAImageProcessing_h
#define PAImageProcessing_h

#include <mitkPhotoacousticFilterService.h>

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>
#include <qthread.h>

#include "ui_PAImageProcessingControls.h"

#include "mitkBeamformingFilter.h"
#include "mitkBeamformingSettings.h"

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

  void ChangedProbe();
  void UseResampling();
  void UseLogfilter();
  void SetResampling();
  void UseImageSpacing();
  void UpdateImageInfo();
  void UseSignalDelay();

  /** \brief Beamforming is being performed in a separate thread to keep the workbench from freezing.
  */
  void StartBeamformingThread();

  /** \brief B-mode filtering is being performed in a separate thread to keep the workbench from freezing.
  */
  void StartBmodeThread();

  /** \brief Cropping is being performed in a separate thread to keep the workbench from freezing.
  */
  void StartCropThread();

  /** \brief Method called when the bandpass thread finishes;
  *  it adds the image to a new data node and registers it to the worbench's data storage
  */
  void HandleResults(mitk::Image::Pointer image, std::string nameExtension);

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
  void CreateQtPartControl(QWidget *parent) override;

  void SetFocus() override;

  /** \brief called by QmitkFunctionality when DataManager's selection has changed.
  *  On a change some parameters are internally updated to calculate bounds for GUI elements as the slice selector for beamforming or
  *  the bandpass filter settings.
  */
  void OnSelectionChanged(berry::IWorkbenchPart::Pointer source,
    const QList<mitk::DataNode::Pointer>& nodes) override;

  /** \brief Instance of the GUI controls
  */
  Ui::PAImageProcessingControls m_Controls;

  float m_ResampleSpacing;
  bool m_UseLogfilter;
  std::string m_OldNodeName;

  /** \brief Method for updating the BFconfig by using a selected image and the GUI configuration.
  */
  mitk::BeamformingSettings::Pointer CreateBeamformingSettings(mitk::Image::Pointer image);

  void EnableControls();
  void DisableControls();

  /** \brief Class through which the filters are called.
  */
  mitk::PhotoacousticFilterService::Pointer m_FilterBank;
};

class BeamformingThread : public QThread
{
  Q_OBJECT
    void run() Q_DECL_OVERRIDE;

signals:
  void result(mitk::Image::Pointer, std::string nameExtension);
  void updateProgress(int, std::string);
  void message(std::string);

public:
  BeamformingThread() : m_SignalDelay(0) {}

  void setConfig(mitk::BeamformingSettings::Pointer BFconfig);
  void setSignalDelay(float delay);
  void setInputImage(mitk::Image::Pointer image);
  void setFilterBank(mitk::PhotoacousticFilterService::Pointer filterBank)
  {
    m_FilterBank = filterBank;
  }

protected:
  mitk::BeamformingSettings::Pointer m_BFconfig;
  mitk::Image::Pointer m_InputImage;
  int m_Cutoff;
  float m_SignalDelay; // [us]

  mitk::PhotoacousticFilterService::Pointer m_FilterBank;
};

class BmodeThread : public QThread
{
  Q_OBJECT
    void run() Q_DECL_OVERRIDE;

signals:
  void result(mitk::Image::Pointer, std::string nameExtension);

public:
  enum BModeMethod { ShapeDetection, Abs };

  void setConfig(bool useLogfilter, double resampleSpacing, mitk::PhotoacousticFilterService::BModeMethod method, bool useGPU);
  void setInputImage(mitk::Image::Pointer image);
  void setFilterBank(mitk::PhotoacousticFilterService::Pointer filterBank)
  {
    m_FilterBank = filterBank;
  }

protected:
  mitk::Image::Pointer m_InputImage;

  mitk::PhotoacousticFilterService::BModeMethod m_Method;
  bool m_UseLogfilter;
  double m_ResampleSpacing;
  bool m_UseGPU;

  mitk::PhotoacousticFilterService::Pointer m_FilterBank;
};

class CropThread : public QThread
{
  Q_OBJECT
    void run() Q_DECL_OVERRIDE;

signals:
  void result(mitk::Image::Pointer, std::string nameExtension);

public:
  void setConfig(unsigned int CutAbove, unsigned int CutBelow, unsigned int CutRight, unsigned int CutLeft, unsigned int CutSliceFirst, unsigned int CutSliceLast);
  void setInputImage(mitk::Image::Pointer image);
  void setFilterBank(mitk::PhotoacousticFilterService::Pointer filterBank)
  {
    m_FilterBank = filterBank;
  }

protected:
  mitk::Image::Pointer m_InputImage;

  unsigned int m_CutAbove;
  unsigned int m_CutBelow;
  unsigned int m_CutRight;
  unsigned int m_CutLeft;
  unsigned int m_CutSliceLast;
  unsigned int m_CutSliceFirst;

  mitk::PhotoacousticFilterService::Pointer m_FilterBank;
};

class BandpassThread : public QThread
{
  Q_OBJECT
    void run() Q_DECL_OVERRIDE;

signals:
  void result(mitk::Image::Pointer, std::string nameExtension);

public:
  void setConfig(float BPHighPass, float BPLowPass, float TukeyAlphaHighPass, float TukeyAlphaLowPass, float TimeSpacing, float SpeedOfSound, bool IsBFImage);
  void setInputImage(mitk::Image::Pointer image);
  void setFilterBank(mitk::PhotoacousticFilterService::Pointer filterBank)
  {
    m_FilterBank = filterBank;
  }

protected:
  mitk::Image::Pointer m_InputImage;

  float m_BPHighPass;
  float m_BPLowPass;
  float m_TukeyAlphaHighPass;
  float m_TukeyAlphaLowPass;
  float m_TimeSpacing;
  float m_SpeedOfSound;
  bool m_IsBFImage;

  mitk::PhotoacousticFilterService::Pointer m_FilterBank;
};

#endif // PAImageProcessing_h
