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

#include "ui_PAImageProcessingControls.h"

#include "mitkPhotoacousticBeamformingDASFilter.h"
#include "mitkPhotoacousticBeamformingDMASFilter.h"

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
    
    void UseResampling();
    void UseLogfilter();
    void SetResampling();
    void UseImageSpacing();
    void UpdateFrequency();
    void UpdateRecordTime(mitk::Image::Pointer image);
    void UseBandpass();

    void ApplyBModeFilter();
    void ApplyBeamforming();

  protected:
    virtual void CreateQtPartControl(QWidget *parent) override;

    virtual void SetFocus() override;

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
                                     const QList<mitk::DataNode::Pointer>& nodes ) override;

    Ui::PAImageProcessingControls m_Controls;

    float m_ResampleSpacing;
    bool m_UseLogfilter;

    mitk::BeamformingDMASFilter::beamformingSettings DMASconfig;
    mitk::BeamformingDASFilter::beamformingSettings DASconfig;

    enum BeamformingAlgorithms {DAS, DMAS};

    BeamformingAlgorithms m_CurrentBeamformingAlgorithm;

    void UpdateBFSettings(mitk::Image::Pointer image);
};

#endif // PAImageProcessing_h
