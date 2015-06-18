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

#ifndef BERRYCONFIGURATIONELEMENTATTRIBUTE_H
#define BERRYCONFIGURATIONELEMENTATTRIBUTE_H

#include <QString>

namespace berry {

/**
 * Describes properties of configuration elements to be added to the registry.
 * Properties are represented as pairs of strings: {attribute name; attribute value}.
 * <p>
 * It is expected that both attribute name and attribute value are not null.
 * </p>
 * <p>
 * This class can be instantiated.
 * </p>
 * <p>
 * This class is not intended to be subclassed.
 * </p>
 * @see ConfigurationElementDescription
 * @see IConfigurationElement
 */
class ConfigurationElementAttribute
{

private:

  /**
   * Attribute name.
   * @see IConfigurationElement#getAttributeNames()
   */
  QString name;

  /**
   * Attribute value.
   * @see IConfigurationElement#getAttributeAsIs(String)
   */
  QString value;

public:

  /**
   * Constructor.
   *
   * @param name attribute name
   * @param value attribute value
   */
  ConfigurationElementAttribute(const QString& name, const QString& value);

  /**
   * Returns attribute name.
   * @return attribute name
   * @see IConfigurationElement#getAttributeNames()
   */
  QString GetName() const;

  /**
   * Returns value of the attribute.
   * @return attribute value
   * @see IConfigurationElement#getAttributeAsIs(String)
   */
  QString GetValue() const;
};

}

#endif // BERRYCONFIGURATIONELEMENTATTRIBUTE_H
