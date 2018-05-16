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

#ifndef QmitkFiberClusteringView_h
#define QmitkFiberClusteringView_h

#include <QmitkAbstractView.h>
#include "ui_QmitkFiberClusteringViewControls.h"
#include <itkImage.h>

/*!
\brief Cluster fibers by shape and image properties

*/
class QmitkFiberClusteringView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkFiberClusteringView();
  virtual ~QmitkFiberClusteringView();

  virtual void CreateQtPartControl(QWidget *parent) override;

  typedef itk::Image< float, 3 > FloatImageType;
  typedef itk::Image< short, 3 > ShortImageType;

  ///
  /// Sets the focus to an internal widget.
  ///
  virtual void SetFocus() override;

protected slots:

  void StartClustering();
  void FiberSelectionChanged();

protected:

  /// \brief called by QmitkAbstractView when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

  Ui::QmitkFiberClusteringViewControls* m_Controls;

};



#endif // _QMITKFIBERTRACKINGVIEW_H_INCLUDED

