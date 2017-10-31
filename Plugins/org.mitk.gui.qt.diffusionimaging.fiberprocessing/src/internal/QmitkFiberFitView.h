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

#ifndef QmitkFiberFitView_h
#define QmitkFiberFitView_h

#include <QmitkAbstractView.h>
#include "ui_QmitkFiberFitViewControls.h"
#include <itkImage.h>

/*!
\brief Weight fibers by linearly fitting them to the image data.

*/
class QmitkFiberFitView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkFiberFitView();
  virtual ~QmitkFiberFitView();

  virtual void CreateQtPartControl(QWidget *parent) override;

  ///
  /// Sets the focus to an internal widget.
  ///
  virtual void SetFocus() override;

protected slots:

  void StartFit();
  void DataSelectionChanged();

protected:

  /// \brief called by QmitkAbstractView when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;

  Ui::QmitkFiberFitViewControls* m_Controls;

};



#endif // _QMITKFIBERTRACKINGVIEW_H_INCLUDED

