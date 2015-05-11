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

#ifndef QMITKSHOWPREFERENCEPAGEHANDLER_H
#define QMITKSHOWPREFERENCEPAGEHANDLER_H

#include <berryAbstractHandler.h>

/**
 * Shows the given preference page. If no preference page id is specified in the
 * parameters, then this opens the preferences dialog to whatever page was
 * active the last time the dialog was shown.
 */
class QmitkShowPreferencePageHandler : public berry::AbstractHandler
{
  Q_OBJECT

public:

  berry::Object::Pointer Execute(const berry::SmartPointer<const berry::ExecutionEvent> &event) override;

};

#endif // QMITKSHOWPREFERENCEPAGEHANDLER_H
