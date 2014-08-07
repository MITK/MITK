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

#ifndef VIEWTAGSDESCRIPTOR_H_
#define VIEWTAGSDESCRIPTOR_H_

#include <berryIConfigurationElement.h>
#include <berryObject.h>

class ViewTagsDescriptor : public berry::Object
{
public:

  berryObjectMacro(ViewTagsDescriptor);

  /**
   * Initialize the "ChangeText" Descriptor with the given extension point.
   *
   * @param changeTextExtensionPoint
   *           element, that refers to a extension point (type, id, name, class)
   */
  ViewTagsDescriptor(berry::IConfigurationElement::Pointer changeTextExtensionPoint);
  ViewTagsDescriptor(QString id);

  /**
   * Default destructor
   */
  ~ViewTagsDescriptor();

  /**
   * Returns the id of this "ChangeText".
   *
   * @return the id
   */
  QString GetID() const;


  std::vector<QString> GetTags();
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

  QString m_Id;
  std::vector<QString> m_Tags;
};

#endif /*VIEWTAGSDESCRIPTOR_H_*/
