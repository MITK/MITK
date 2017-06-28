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

#include <QmitkAbstractView.h>

#include "ui_QmitkStreamlineTrackingViewControls.h"

#include <mitkTensorImage.h>
#include <mitkDataStorage.h>
#include <mitkDataNode.h>
#include <mitkImage.h>
#include <itkImage.h>
#include <itkStreamlineTrackingFilter.h>
#include <Algorithms/TrackingHandlers/mitkTrackingHandlerTensor.h>
#include <Algorithms/TrackingHandlers/mitkTrackingHandlerPeaks.h>
#include <Algorithms/TrackingHandlers/mitkTrackingHandlerOdf.h>
#include <random>
#include <mitkPointSet.h>
#include <mitkPointSetShapeProperty.h>


/*!
\brief View for tensor based deterministic streamline fiber tracking.
*/
class QmitkStreamlineTrackingView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  typedef itk::Image< unsigned char, 3 > ItkUCharImageType;
  typedef itk::Image< float, 3 > ItkFloatImageType;

  QmitkStreamlineTrackingView();
  virtual ~QmitkStreamlineTrackingView();

  virtual void CreateQtPartControl(QWidget *parent) override;

  ///
  /// Sets the focus to an internal widget.
  ///
  virtual void SetFocus() override;

protected slots:

  void DoFiberTracking();   ///< start fiber tracking
  void UpdateGui();
  void ToggleInteractive();


protected:

  /// \brief called by QmitkAbstractView when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

  Ui::QmitkStreamlineTrackingViewControls* m_Controls;

protected slots:

private:

  int m_SliceObserverTag1;
  int m_SliceObserverTag2;
  int m_SliceObserverTag3;
  std::vector< itk::Point<float> > m_SeedPoints;
  mitk::DataNode::Pointer m_InteractiveNode;
  mitk::DataNode::Pointer m_InteractivePointSetNode;

  std::vector< mitk::DataNode::Pointer > m_InputImageNodes; ///< input images
  std::vector< mitk::Image::Pointer > m_InputImages; ///< input image datanode
  bool		m_FirstTensorProbRun;

  void OnSliceChanged(const itk::EventObject& e);
};



#endif // _QMITKFIBERTRACKINGVIEW_H_INCLUDED

