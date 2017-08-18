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

    void UpdateBounds();
    
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

    mitk::BeamformingFilter::beamformingSettings BFconfig;

    void UpdateBFSettings(mitk::Image::Pointer image);

    void EnableControls();
    void DisableControls();
};

class BeamformingThread : public QThread
{
  Q_OBJECT
    void run() Q_DECL_OVERRIDE;

  signals:
    void result(mitk::Image::Pointer);
    void updateProgress(int, std::string);

  public:
    void setConfig(mitk::BeamformingFilter::beamformingSettings BFconfig);
    void setInputImage(mitk::Image::Pointer image);
    void setCutoff(int cutoff);

  protected:
    mitk::BeamformingFilter::beamformingSettings m_BFconfig;
    mitk::Image::Pointer m_InputImage;
    int m_Cutoff;
};

class BmodeThread : public QThread
{
  Q_OBJECT
    void run() Q_DECL_OVERRIDE;

  signals:
    void result(mitk::Image::Pointer);

  public:
    enum BModeMethod { ShapeDetection, Abs };

    void setConfig(bool useLogfilter, double resampleSpacing, mitk::PhotoacousticImage::BModeMethod method);
    void setInputImage(mitk::Image::Pointer image);

  protected:
    mitk::Image::Pointer m_InputImage;

    mitk::PhotoacousticImage::BModeMethod m_Method;
    bool m_UseLogfilter;
    double m_ResampleSpacing;
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

protected:
  mitk::Image::Pointer m_InputImage;

  unsigned int m_CutAbove;
  unsigned int m_CutBelow;
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

protected:
  mitk::Image::Pointer m_InputImage;

  float m_BPHighPass;
  float m_BPLowPass;
  float m_TukeyAlpha;
  float m_RecordTime;
};

#endif // PAImageProcessing_h
