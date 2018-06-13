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

#ifndef SpectralUnmixing_h
#define SpectralUnmixing_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include "ui_SpectralUnmixingControls.h"

class SpectralUnmixing : public QmitkAbstractView
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

  virtual void ClearWavelength();

  Ui::SpectralUnmixingControls m_Controls;

  std::vector<int> m_Wavelengths;

  // Selection of Chromophores
  bool DeOxbool;
  bool Oxbool;
};

#endif // SpectralUnmixing_h
