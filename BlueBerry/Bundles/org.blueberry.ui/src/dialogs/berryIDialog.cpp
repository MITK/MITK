/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryIDialog.h"

namespace berry {

const int IDialog::NONE = 0;
const int IDialog::ERR = 1;
const int IDialog::INFORMATION = 2;
const int IDialog::QUESTION = 3;
const int IDialog::WARNING = 4;

IDialog::~IDialog()
{

}

}
