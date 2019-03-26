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

#ifndef QmitkFiberQuantificationView_h
#define QmitkFiberQuantificationView_h

#include <QmitkAbstractView.h>
#include "ui_QmitkFiberQuantificationViewControls.h"

#include <mitkFiberBundle.h>
#include <mitkPointSet.h>
#include <itkCastImageFilter.h>
#include <mitkILifecycleAwarePart.h>

/*!
\brief Generation of images from fiber bundles (TDI, envelopes, endpoint distribution) and extraction of principal fiber directions from tractograms.

*/
class QmitkFiberQuantificationView : public QmitkAbstractView, public mitk::ILifecycleAwarePart
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  typedef itk::Image< unsigned char, 3 >    itkUCharImageType;

  static const std::string VIEW_ID;

  QmitkFiberQuantificationView();
  virtual ~QmitkFiberQuantificationView();

  virtual void CreateQtPartControl(QWidget *parent) override;

  ///
  /// Sets the focus to an internal widget.
  ///
  virtual void SetFocus() override;

  virtual void Activated() override;
  virtual void Deactivated() override;
  virtual void Visible() override;
  virtual void Hidden() override;

protected slots:

  void ProcessSelectedBundles();    ///< start selected operation on fiber bundle (e.g. tract density image generation)
  void CalculateFiberDirections();  ///< Calculate main fiber directions from tractogram
  void UpdateGui();     ///< update button activity etc. dpending on current datamanager selection

protected:

  /// \brief called by QmitkAbstractView when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

  Ui::QmitkFiberQuantificationViewControls* m_Controls;

  std::vector<mitk::DataNode::Pointer>  m_SelectedFB;       ///< selected fiber bundle nodes
  mitk::Image::Pointer                  m_SelectedImage;
  float                                 m_UpsamplingFactor; ///< upsampling factor for all image generations

  mitk::DataNode::Pointer GenerateTractDensityImage(mitk::FiberBundle::Pointer fib, bool binary, bool absolute, std::string name);
  mitk::DataNode::Pointer GenerateColorHeatmap(mitk::FiberBundle::Pointer fib);
  mitk::DataNode::Pointer GenerateFiberEndingsImage(mitk::FiberBundle::Pointer fib);
  mitk::DataNode::Pointer GenerateFiberEndingsPointSet(mitk::FiberBundle::Pointer fib);
  bool  m_Visible;
};



#endif // _QMITKFIBERTRACKINGVIEW_H_INCLUDED

