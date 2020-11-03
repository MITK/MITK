/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYISOURCEPROVIDERLISTENER_H_
#define BERRYISOURCEPROVIDERLISTENER_H_

#include <org_blueberry_ui_qt_Export.h>

#include <berryMacros.h>
#include <berryObject.h>
#include <berryMessage.h>

#include <map>

namespace berry {

/**
 * <p>
 * A listener to changes in a particular source of information. This listener is
 * notified as the source changes. Typically, workbench services will implement
 * this interface, and register themselves as listeners to the
 * <code>ISourceProvider</code> instances that are registered with them.
 * </p>
 *
 * @see ISources
 * @see ISourceProvider
 */
struct BERRY_UI_QT ISourceProviderListener
{

  struct Events {
    Message2<int, const QHash<QString, Object::ConstPointer>& > multipleSourcesChanged;
    Message3<int, const QString&, Object::ConstPointer> singleSourceChanged;

    void AddListener(ISourceProviderListener* l);
    void RemoveListener(ISourceProviderListener* l);

  private:

    typedef MessageDelegate2<ISourceProviderListener, int, const QHash<QString, Object::ConstPointer>& > Delegate2;
    typedef MessageDelegate3<ISourceProviderListener, int, const QString&, Object::ConstPointer> Delegate3;
  };

  virtual ~ISourceProviderListener();

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
                             const QHash<QString, Object::ConstPointer>& sourceValuesByName) = 0;

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
                             const QString& sourceName, Object::ConstPointer sourceValue) = 0;
};

}

#endif /* BERRYISOURCEPROVIDERLISTENER_H_ */
