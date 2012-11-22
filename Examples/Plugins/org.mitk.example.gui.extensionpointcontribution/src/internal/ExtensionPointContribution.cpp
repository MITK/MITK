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

#include "ExtensionPointContribution.h"

// Qt includes
#include <QMessageBox>

ExtensionPointContribution::ExtensionPointContribution()
{
}

ExtensionPointContribution::~ExtensionPointContribution()
{
}

void ExtensionPointContribution::ChangeExtensionLabelText(const QString& /*s*/)
{
  QMessageBox::critical(0, "Error", "Provided by extension!");
}

