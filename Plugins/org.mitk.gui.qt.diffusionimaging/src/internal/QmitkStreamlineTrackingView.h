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

#ifndef QmitkStreamlineTrackingView_h
#define QmitkStreamlineTrackingView_h

#include <QmitkFunctionality.h>

#include "ui_QmitkStreamlineTrackingViewControls.h"

#include <mitkTensorImage.h>
#include <mitkDataStorage.h>
#include <mitkImage.h>
#include <itkImage.h>
#include <itkStreamlineTrackingFilter.h>


/*!
\brief QmitkStreamlineTrackingView

\warning  Implements standard streamline tracking as proposed by Mori et al. 1999 "Three-Dimensional Tracking of Axonal Projections in the Brain by Magnetic Resonance Imaging"

\sa QmitkFunctionality
\ingroup Functionalities
*/
class QmitkStreamlineTrackingView : public QmitkFunctionality
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  typedef itk::Image< unsigned char, 3 > ItkUCharImageType;

  QmitkStreamlineTrackingView();
  virtual ~QmitkStreamlineTrackingView();

  virtual void CreateQtPartControl(QWidget *parent);

  virtual void StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget);
  virtual void StdMultiWidgetNotAvailable();

  protected slots:

  void DoFiberTracking();

protected:

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged( std::vector<mitk::DataNode*> nodes );
  Ui::QmitkStreamlineTrackingViewControls* m_Controls;
  QmitkStdMultiWidget* m_MultiWidget;

protected slots:

  void OnSeedsPerVoxelChanged(int value);
  void OnMinTractLengthChanged(int value);
  void OnFaThresholdChanged(int value);
  void OnStepsizeChanged(int value);

private:

  mitk::Image::Pointer          m_SeedRoi;
  mitk::TensorImage::Pointer    m_TensorImage;
  mitk::DataNode::Pointer       m_TensorImageNode;

};



#endif // _QMITKFIBERTRACKINGVIEW_H_INCLUDED

