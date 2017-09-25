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

class PAImageProcessing : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    PAImageProcessing();

  protected slots:

    /// \brief Called when the user clicks the GUI button

    void UpperSliceBoundChanged();
    void LowerSliceBoundChanged();
    void SliceBoundsEnabled();
    
    void UseResampling();
    void UseLogfilter();
    void SetResampling();
    void UseImageSpacing();
    void UpdateImageInfo();
    void UpdateRecordTime(mitk::Image::Pointer image);

    void HandleBeamformingResults(mitk::Image::Pointer image);
    void StartBeamformingThread();

    void HandleBmodeResults(mitk::Image::Pointer image);
    void StartBmodeThread();

    void HandleCropResults(mitk::Image::Pointer image);
    void StartCropThread();

    void HandleBandpassResults(mitk::Image::Pointer image);
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

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
                                     const QList<mitk::DataNode::Pointer>& nodes ) override;

    Ui::PAImageProcessingControls m_Controls;

    float m_ResampleSpacing;
    bool m_UseLogfilter;
    std::string m_OldNodeName;

    mitk::BeamformingSettings BFconfig;

    void UpdateBFSettings(mitk::Image::Pointer image);

    void EnableControls();
    void DisableControls();

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
  void setConfig(unsigned int CutAbove, unsigned int CutBelow);
  void setInputImage(mitk::Image::Pointer image);
  void setFilterBank(mitk::PhotoacousticImage::Pointer filterBank)
  {
    m_FilterBank = filterBank;
  }

protected:
  mitk::Image::Pointer m_InputImage;

  unsigned int m_CutAbove;
  unsigned int m_CutBelow;

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
