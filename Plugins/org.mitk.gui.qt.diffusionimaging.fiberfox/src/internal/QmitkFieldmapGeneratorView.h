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

#ifndef _QMITKQmitkFieldmapGeneratorView_H_INCLUDED
#define _QMITKQmitkFieldmapGeneratorView_H_INCLUDED

#include <berryISelectionListener.h>
#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>
#include "ui_QmitkFieldmapGeneratorViewControls.h"
#include <mitkPointSet.h>
#include <itkImage.h>
#include "mitkILifecycleAwarePart.h"
#include <QmitkSliceNavigationListener.h>

/*!
  \brief Generate float image with artificial frequency maps used by Fiberfox. Simulates additional frequencies at (possibly multiple) positions based on 3D gaussians with the specified variance and amplitude and/or as a linear gradient in the image.
* See "Fiberfox: Facilitating the creation of realistic white matter software phantoms" (DOI: 10.1002/mrm.25045) for details.
*/

class QmitkFieldmapGeneratorView : public QmitkAbstractView, public mitk::ILifecycleAwarePart
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkFieldmapGeneratorView();
  virtual ~QmitkFieldmapGeneratorView();

  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void SetFocus() override;

  virtual void Activated() override;
  virtual void Deactivated() override;
  virtual void Visible() override;
  virtual void Hidden() override;


protected slots:

  void GenerateFieldmap();
  void PlaceFieldSource();
  void OnVarianceChanged(double value);
  void OnHeightChanged(double value);
  void OnSliceChanged();

protected:

  /// \brief called by QmitkAbstractView when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

  Ui::QmitkFieldmapGeneratorViewControls* m_Controls;

  /** observer flags */
  int m_SliceObserverTag1;
  int m_SliceObserverTag2;
  int m_SliceObserverTag3;
  int m_PropertyObserverTag;

  mitk::Point3D                 m_WorldPoint;
  mitk::DataNode::Pointer       m_SelectedSource;
  QmitkSliceNavigationListener  m_SliceChangeListener;
};



#endif // _QmitkFieldmapGeneratorView_H_INCLUDED

