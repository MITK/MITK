/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef PAUSMotionCompensation_h
#define PAUSMotionCompensation_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_PAUSMotionCompensationControls.h"

#include "mitkPhotoacousticMotionCorrectionFilter.h"

/**
  \brief PAUSMotionCompensation

  Photoacoustic and ultrasound image motion correction can be performed with this plugin. Internally OpenCV2 calc OpticalFlowFarneback is used.
  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class PAUSMotionCompensation : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  static const std::string VIEW_ID;

protected:
  void CreateQtPartControl(QWidget *parent) override;

  void SetFocus() override;

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  void OnSelectionChanged(berry::IWorkbenchPart::Pointer source,
                                  const QList<mitk::DataNode::Pointer> &nodes) override;

  /// \brief Called when the user clicks the GUI button
  void DoImageProcessing();
  void DoUpdateParameters();

  Ui::PAUSMotionCompensationControls m_Controls;
  mitk::PhotoacousticMotionCorrectionFilter::Pointer m_Filter = mitk::PhotoacousticMotionCorrectionFilter::New();
};

#endif // PAUSMotionCompensation_h
