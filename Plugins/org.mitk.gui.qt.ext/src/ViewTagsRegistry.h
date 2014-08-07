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

#ifndef VIEWTAGSREGISTRY_H
#define VIEWTAGSREGISTRY_H

#include <berryObject.h>

#include "ViewTagsDescriptor.h"

class ViewTagsRegistry : public berry::Object
{
public:

  ViewTagsRegistry();
  ~ViewTagsRegistry();

  /**
   * Return an "change text" descriptor with the given extension id. If no "change text" exists,
   * with the id return <code>null</code>.
   *
   * @param id
   *           the id to search for
   * @return the descriptor or <code>null</code>
   */
  ViewTagsDescriptor::Pointer Find(const QString& id) const;
  ViewTagsDescriptor::Pointer Find(const std::string& id) const;

  /**
   * Return a list of "change texts" defined in the registry.
   *
   * @return the change texts.
   */
  QList<ViewTagsDescriptor::Pointer> GetViewTags() const;

private:

  QHash<QString, ViewTagsDescriptor::Pointer> m_ListRegisteredViewTags;
};

#endif /*VIEWTAGSREGISTRY_H*/
