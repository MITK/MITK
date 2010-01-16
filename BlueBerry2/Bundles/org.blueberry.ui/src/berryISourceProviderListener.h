/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef BERRYISOURCEPROVIDERLISTENER_H_
#define BERRYISOURCEPROVIDERLISTENER_H_

#include "berryUiDll.h"

#include <osgi/framework/Macros.h>
#include <osgi/framework/Object.h>
#include <osgi/framework/Message.h>

#include <map>

namespace berry {

using namespace osgi::framework;

/**
 * <p>
 * A listener to changes in a particular source of information. This listener is
 * notified as the source changes. Typically, workbench services will implement
 * this interface, and register themselves as listeners to the
 * <code>ISourceProvider</code> instances that are registered with them.
 * </p>
 *
 * @since 3.1
 * @see org.blueberry.ui.ISources
 * @see org.blueberry.ui.ISourceProvider
 */
struct BERRY_UI ISourceProviderListener : public virtual Object {

  osgiInterfaceMacro(berry::ISourceProviderListener);

  struct Events {
    Message2<int, const std::map<std::string, Object::Pointer>& > multipleSourcesChanged;
    Message3<int, const std::string&, Object::Pointer> singleSourceChanged;

    void AddListener(ISourceProviderListener::Pointer l);
    void RemoveListener(ISourceProviderListener::Pointer l);

  private:

    typedef MessageDelegate2<ISourceProviderListener, int, const std::map<std::string, Object::Pointer>& > Delegate2;
    typedef MessageDelegate3<ISourceProviderListener, int, const std::string&, Object::Pointer> Delegate3;
  };

  /**
   * Handles a change to multiple sources. The source priority should be a bit
   * mask indicating the sources. The map will be used to construct the
   * variables on an <code>IEvaluationContext</code>
   *
   * @param sourcePriority
   *            A bit mask of all the source priorities that have changed.
   * @param sourceValuesByName
   *            A mapping of the source names (<code>String</code>) to the
   *            source values (<code>Object</code>). The names should
   *            never be <code>null</code>, but the values may be. The map
   *            must not be <code>null</code>, and should contain at least
   *            two elements (one for each source).
   * @see org.blueberry.core.expressions.IEvaluationContext
   * @see ISources
   */
  virtual void SourceChanged(int sourcePriority,
      const std::map<std::string, Object::Pointer>& sourceValuesByName) = 0;

  /**
   * Handles a change to one source. The source priority should indicate the
   * source, and the name-value pair will be used to create an
   * <code>IEvaluationContext</code> with a single variable.
   *
   * @param sourcePriority
   *            A bit mask of all the source priorities that have changed.
   * @param sourceName
   *            The name of the source that changed; must not be
   *            <code>null</code>.
   * @param sourceValue
   *            The new value for that source; may be <code>null</code>.
   * @see org.blueberry.core.expressions.IEvaluationContext
   * @see ISources
   */
  virtual void SourceChanged(int sourcePriority,
      const std::string& sourceName, Object::Pointer sourceValue) = 0;
};

}

#endif /* BERRYISOURCEPROVIDERLISTENER_H_ */
