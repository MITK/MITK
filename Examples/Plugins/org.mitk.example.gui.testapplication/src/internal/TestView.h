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


#ifndef TESTVIEW_H_
#define TESTVIEW_H_

#include <berryIViewPart.h>

#include <QObject>


class TestView : public QObject, public berry::IViewPart
{
  Q_OBJECT
  Q_INTERFACES(berry::IViewPart)

public:

  TestView();

  //virtual void CreateQtPartControl(QWidget *parent);

};

#endif /* TESTPERSPECTIVE_H_ */
