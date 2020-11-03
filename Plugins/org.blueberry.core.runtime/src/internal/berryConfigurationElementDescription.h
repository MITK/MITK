/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYCONFIGURATIONELEMENTDESCRIPTION_H
#define BERRYCONFIGURATIONELEMENTDESCRIPTION_H

#include <QString>
#include <QList>

namespace berry {

class ConfigurationElementAttribute;

/**
 * Describes configuration elements (content of an extension) to be added to
 * the extension registry. Configuration element can contain attributes or
 * a String value. Configuration element can contain other configuration
 * elements (children).
 * <p>
 * It is expected that configuration element's name is not null. Attributes and
 * children of the extension description might be null; value might be null as well.
 * </p>
 * <p>
 * This class can be instantiated.
 * </p>
 * <p>
 * This class is not intended to be subclassed.
 * </p>
 * @see ConfigurationElementAttribute
 */
class ConfigurationElementDescription
{

private:

  /**
   * Name of the configuration element.
   * @see IConfigurationElement#getName()
   */
  QString name;

  /**
   * Attributes of the configuration element.
   * @see IConfigurationElement#getAttribute(String)
   */
  QList<ConfigurationElementAttribute> attributes;

  /**
   * String value to be stored in this configuration element.
   * @see IConfigurationElement#getValue()
   */
  QString value;

  /**
   * Children of the configuration element.
   * @see IConfigurationElement#getChildren()
   */
  QList<ConfigurationElementDescription> children;

public:

  /**
   * Constructor.
   *
   * @param name - name of the configuration element
   * @param attributes - attributes of the configuration element. Might be null.
   * @param value - string value to be stored. Might be null.
   * @param children - children of the configuration element. Might be null.
   * @see IConfigurationElement#getName()
   * @see IConfigurationElement#getChildren()
   * @see IConfigurationElement#getAttribute(String)
   * @see IConfigurationElement#getValue()
   */
  ConfigurationElementDescription(const QString& name, const QList<ConfigurationElementAttribute>& attributes,
                                  const QString& value, const QList<ConfigurationElementDescription>& children);

  /**
   * Constructor.
   *
   * @param name - name of the configuration element
   * @param attribute - attribute of the configuration element. Might be null.
   * @param value - string value to be stored. Might be null.
   * @param children - children of the configuration element. Might be null.
   * @see IConfigurationElement#getName()
   * @see IConfigurationElement#getChildren()
   * @see IConfigurationElement#getAttribute(String)
   * @see IConfigurationElement#getValue()
   */
  ConfigurationElementDescription(const QString& name, const ConfigurationElementAttribute& attribute,
                                  const QString& value, const QList<ConfigurationElementDescription>& children);

  /**
   * Returns children of the configuration element.
   * @return - children of the extension
   * @see IConfigurationElement#getChildren()
   */
  QList<ConfigurationElementDescription> GetChildren() const;

  /**
   * Returns name of the configuration element.
   * @return - extension name
   * @see IConfigurationElement#getName()
   */
  QString GetName() const;

  /**
   * Returns attributes of the configuration element.
   * @return - attributes of the extension description
   * @see IConfigurationElement#getAttribute(String)
   */
  QList<ConfigurationElementAttribute> GetAttributes() const;

  /**
   * Returns string value stored in the configuration element.
   * @return - String value to be stored in the element
   * @see IConfigurationElement#getValue()
   */
  QString GetValue() const;
};

}

#endif // BERRYCONFIGURATIONELEMENTDESCRIPTION_H
