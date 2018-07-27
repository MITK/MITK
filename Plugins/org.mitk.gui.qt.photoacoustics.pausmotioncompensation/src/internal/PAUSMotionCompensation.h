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
  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void SetFocus() override;

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer source,
                                  const QList<mitk::DataNode::Pointer> &nodes) override;

  /// \brief Called when the user clicks the GUI button
  void DoImageProcessing();
  void DoUpdateParameters();

  Ui::PAUSMotionCompensationControls m_Controls;
  mitk::PhotoacousticMotionCorrectionFilter::Pointer m_Filter = mitk::PhotoacousticMotionCorrectionFilter::New();
};

#endif // PAUSMotionCompensation_h
