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
  SmartPointer<UIElement> GetElement() const;

  /*
   * @see IElementReference#GetCommandId()
   */
  QString GetCommandId() const;

  /*
   * @see IElementReference#GetParameters()
   */
  QHash<QString, Object::Pointer> GetParameters() const;

  void AddParameter(const QString& name, const Object::Pointer& value);
};

}

#endif // BERRYELEMENTREFERENCE_H
