/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYUIEXTENSIONTRACKER_H
#define BERRYUIEXTENSIONTRACKER_H

#include <berryExtensionTracker.h>

namespace berry {

class Display;

class UIExtensionTracker : public ExtensionTracker
{
private:

  Display* display;

public:

  /**
   * @param display
   */
  UIExtensionTracker(Display* display);

protected:

  void ApplyRemove(IExtensionChangeHandler* handler,
                   const SmartPointer<IExtension>& removedExtension,
                   const QList<SmartPointer<Object> >& objects) override;

  void ApplyAdd(IExtensionChangeHandler* handler,
                const SmartPointer<IExtension>& addedExtension) override;
};

}

#endif // BERRYUIEXTENSIONTRACKER_H
