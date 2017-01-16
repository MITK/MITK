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

#ifndef QMITKPAUSVIEWERVIEW_H_INCLUDED
#define QMITKPAUSVIEWERVIEW_H_INCLUDED

#include <berryISelectionListener.h>
#include <QmitkAbstractView.h>

#include "ui_QmitkPAUSViewerViewControls.h"

#include "mitkCommon.h"

class QmitkPAUSViewerView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkPAUSViewerView();
  virtual ~QmitkPAUSViewerView();

  virtual void CreateQtPartControl(QWidget *parent);
  void InitWindows();

protected:

  virtual void SetFocus() override;

  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer source,
    const QList<mitk::DataNode::Pointer>& nodes) override;

  Ui::QmitkPAUSViewerViewControls m_Controls;
};

#endif // QMITKPAUSVIEWERVIEW_H_INCLUDED

