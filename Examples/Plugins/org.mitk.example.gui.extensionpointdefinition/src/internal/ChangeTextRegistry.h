/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef CHANGETEXTREGISTRY_H_
#define CHANGETEXTREGISTRY_H_

#include <berryObject.h>

#include "ChangeTextDescriptor.h"

class ChangeTextRegistry : public berry::Object
{
public:
  ChangeTextRegistry();
  ~ChangeTextRegistry() override;

  /**
   * Return an "change text" descriptor with the given extension id. If no "change text" exists,
   * with the id return <code>null</code>.
   *
   * @param id
   *           the id to search for
   * @return the descriptor or <code>null</code>
   */
  ChangeTextDescriptor::Pointer Find(const QString &id) const;

  /**
   * Return a list of "change texts" defined in the registry.
   *
   * @return the change texts.
   */
  QList<ChangeTextDescriptor::Pointer> GetChangeTexts() const;

private:
  QHash<QString, ChangeTextDescriptor::Pointer> m_ListRegisteredTexts;
};

#endif /*CHANGETEXTREGISTRY_H_*/
