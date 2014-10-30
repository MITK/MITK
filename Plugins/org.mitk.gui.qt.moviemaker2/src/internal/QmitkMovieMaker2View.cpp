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

#include "QmitkMovieMaker2View.h"

const std::string QmitkMovieMaker2View::VIEW_ID = "org.mitk.views.moviemaker2";

QmitkMovieMaker2View::QmitkMovieMaker2View()
{
}

QmitkMovieMaker2View::~QmitkMovieMaker2View()
{
}

void QmitkMovieMaker2View::CreateQtPartControl(QWidget* parent)
{
  m_Controls.setupUi(parent);
}

void QmitkMovieMaker2View::SetFocus()
{
}
