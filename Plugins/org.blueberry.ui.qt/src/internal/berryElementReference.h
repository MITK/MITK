/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYELEMENTREFERENCE_H
#define BERRYELEMENTREFERENCE_H

#include <berryIElementReference.h>

namespace berry {

/**
 * Our element reference that is used during element
 * registration/unregistration.
 */
class ElementReference : public IElementReference
{

private:

  QString commandId;
  SmartPointer<UIElement> element;
  QHash<QString, Object::Pointer> parameters;

public:

  /**
   * Construct the reference.
   *
   * @param id
   *            command id. Must not be <code>null</code>.
   * @param adapt
   *            the element. Must not be <code>null</code>.
   * @param parms.
   *            parameters used for filtering. Must not be <code>null</code>.
   */
  ElementReference(const QString& id, const SmartPointer<UIElement>& adapt,
                   const QHash<QString, Object::Pointer>& parms);

  /*
   * @see IElementReference#GetElement()
   */
  SmartPointer<UIElement> GetElement() const override;

  /*
   * @see IElementReference#GetCommandId()
   */
  QString GetCommandId() const override;

  /*
   * @see IElementReference#GetParameters()
   */
  QHash<QString, Object::Pointer> GetParameters() const override;

  void AddParameter(const QString& name, const Object::Pointer& value);
};

}

#endif // BERRYELEMENTREFERENCE_H
