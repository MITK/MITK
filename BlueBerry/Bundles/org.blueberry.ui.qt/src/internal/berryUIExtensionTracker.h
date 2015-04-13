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
                   const QList<SmartPointer<Object> >& objects);

  void ApplyAdd(IExtensionChangeHandler* handler,
                const SmartPointer<IExtension>& addedExtension);
};

}

#endif // BERRYUIEXTENSIONTRACKER_H
