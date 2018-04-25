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

#include "mitkPAPropertyCalculator.h"

/**
  \brief SpectralUnmixing

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class SpectralUnmixing : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  // Add Wavelengths with button:
  int size = 0;
  std::vector<int> m_Wavelengths;

  bool DeOxbool;
  bool Oxbool;

protected:
  virtual void CreateQtPartControl(QWidget *parent) override;

  virtual void SetFocus() override;

  /// \brief called by QmitkFunctionality when DataManager's selection has changed
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer source,
                                  const QList<mitk::DataNode::Pointer> &nodes) override;

  /// \brief Called when the user clicks the GUI button
  void DoImageProcessing();

  virtual void Wavelength();
  unsigned int wavelength;
  unsigned int numberofChromophores;
  Ui::SpectralUnmixingControls m_Controls;

  mitk::pa::PropertyCalculator::Pointer m_PropertyCalculator;

};

#endif // SpectralUnmixing_h
