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


#ifndef QMITKMOCKFUNCTIONALITY_H_
#define QMITKMOCKFUNCTIONALITY_H_

#include <QmitkFunctionality.h>

#include <util/berryCallHistory.h>

class QmitkMockFunctionality : public QmitkFunctionality
{
  Q_OBJECT

public:

  berryObjectMacro(QmitkMockFunctionality)

  static const std::string ID; // = "org.mitk.gui.qt.common.tests.api.MockFunctionality";

  QmitkMockFunctionality();
  QmitkMockFunctionality(const QmitkMockFunctionality& other);

  ~QmitkMockFunctionality();

  berry::CallHistory::Pointer GetCallHistory() const;

  void SetFocus();

protected:

  void CreateQtPartControl(QWidget* parent);

private:

  berry::CallHistory::Pointer callTrace;
};

#endif /* QMITKMOCKFUNCTIONALITY_H_ */
