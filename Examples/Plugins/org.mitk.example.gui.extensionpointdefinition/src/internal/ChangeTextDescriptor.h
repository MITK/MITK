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

#ifndef CHANGETEXTDESCRIPTOR_H_
#define CHANGETEXTDESCRIPTOR_H_

#include <berryIConfigurationElement.h>
#include <berryObject.h>

#include "IChangeText.h"

class ChangeTextDescriptor : public berry::Object
{

public:

  berryObjectMacro(ChangeTextDescriptor)

  /**
   * Initialize the "ChangeText" Descriptor with the given extension point.
   *
   * @param changeTextExtensionPoint
   *           element, that refers to a extension point (type, id, name, class)
   */
  ChangeTextDescriptor(berry::IConfigurationElement::Pointer changeTextExtensionPoint);

  /**
   * Default destructor
   */
  ~ChangeTextDescriptor();

  /**
   * Creates an instance of "ChangeText" defined in the descriptor.
   *
   * @return change text
   */
  IChangeText::Pointer CreateChangeText();

  /**
   * Returns the description of this "ChangeText".
   *
   * @return the description
   */
  QString GetDescription() const;

  /**
   * Returns the id of this "ChangeText".
   *
   * @return the id
   */
  QString GetID() const;

  /**
   * Returns the name of this "ChangeText".
   *
   * @return the name
   */
  QString GetName() const;

  /**
   * Equals this class with the given parameter.
   *
   * @param object
   *           the object for the equation
   * @return true, if the objects are equal :: false, if they differ in any way
   */
  bool operator==(const Object* object) const;

private:

  // IConfigurationElements are used to access xml files (here: plugin.xml)
  berry::IConfigurationElement::Pointer m_ChangeTextExtensionPoint;
  IChangeText::Pointer m_ChangeText;

  QString m_Id;
  QString m_Name;
  QString m_Description;

};

#endif /*CHANGETEXTDESCRIPTOR_H_*/
