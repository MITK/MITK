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

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>
#include <qthread.h>

#include "ui_PAImageProcessingControls.h"

#include "mitkPhotoacousticBeamformingDASFilter.h"
#include "mitkPhotoacousticBeamformingDMASFilter.h"

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

    enum BeamformingAlgorithms { DAS, DMAS };

  protected slots:

    /// \brief Called when the user clicks the GUI button
    
    void UseResampling();
    void UseLogfilter();
    void SetResampling();
    void UseImageSpacing();
    void UpdateFrequency();
    void UpdateRecordTime(mitk::Image::Pointer image);
    void UseBandpass();

    void HandleBeamformingResults(mitk::Image::Pointer image);
    void StartBeamformingThread();

    void HandleBmodeResults(mitk::Image::Pointer image);
    void StartBmodeThread();

    void UpdateProgress(int progress, std::string progressInfo);

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

    mitk::BeamformingDMASFilter::beamformingSettings DMASconfig;
    mitk::BeamformingDASFilter::beamformingSettings DASconfig;

    BeamformingAlgorithms m_CurrentBeamformingAlgorithm;

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
    void setConfigs(mitk::BeamformingDMASFilter::beamformingSettings DMASconfig, mitk::BeamformingDASFilter::beamformingSettings DASconfig);
    void setBeamformingAlgorithm(PAImageProcessing::BeamformingAlgorithms beamformingAlgorithm);
    void setInputImage(mitk::Image::Pointer image);
    void setCutoff(int cutoff);

  protected:
    mitk::BeamformingDMASFilter::beamformingSettings m_DMASconfig;
    mitk::BeamformingDASFilter::beamformingSettings m_DASconfig;

    PAImageProcessing::BeamformingAlgorithms m_CurrentBeamformingAlgorithm;
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
  void setConfig(bool useLogfilter, double resampleSpacing);
  void setInputImage(mitk::Image::Pointer image);

protected:
  mitk::Image::Pointer m_InputImage;

  bool m_UseLogfilter;
  double m_ResampleSpacing;
};

#endif // PAImageProcessing_h
