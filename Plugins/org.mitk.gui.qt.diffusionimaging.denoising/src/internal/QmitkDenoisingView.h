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

#ifndef _QMITKQmitkDenoisingView_H_INCLUDED
#define _QMITKQmitkDenoisingView_H_INCLUDED

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_QmitkDenoisingViewControls.h"

#include <itkVectorImage.h>
#include <itkImage.h>
#include <mitkImage.h>
#include <QThread>
#include <QTimer>


/**
 * \class QmitkDenoisingView
 * \brief View for denoising diffusion-weighted images.
 */
class QmitkDenoisingView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkDenoisingView();
  virtual ~QmitkDenoisingView();

  /** Typedefs */
  typedef short                                                                         DiffusionPixelType;
  typedef itk::VectorImage<DiffusionPixelType, 3>                                       DwiImageType;
  typedef itk::Image<DiffusionPixelType, 3>                                             DwiVolumeType;

  virtual void CreateQtPartControl(QWidget *parent) override;

  ///
  /// Sets the focus to an internal widget.
  ///
  virtual void SetFocus() override;

private slots:

  void StartDenoising();
  void UpdateGui();

private:

  /// \brief called by QmitkAbstractView when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

  Ui::QmitkDenoisingViewControls*             m_Controls;
  mitk::DataNode::Pointer                     m_ImageNode;
};



#endif // _QmitkDenoisingView_H_INCLUDED
