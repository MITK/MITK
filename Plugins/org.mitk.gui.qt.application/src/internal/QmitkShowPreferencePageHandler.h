/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkShowPreferencePageHandler_h
#define QmitkShowPreferencePageHandler_h

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

#endif
