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

#ifndef CHANGETEXTREGISTRY_H_
#define CHANGETEXTREGISTRY_H_

#include <Poco/HashMap.h>
#include <string>
#include <vector>

#include <ChangeTextDescriptor.h>

  class ChangeTextRegistry
  {
  public:

    // easier maintenance
    typedef ChangeTextDescriptor::Pointer ChangeTextDescriptorPtr;

    ChangeTextRegistry();
    ~ChangeTextRegistry();

        /**
        * Return an "change text" descriptor with the given extension id. If no "change text" exists,
        * with the id return <code>null</code>.
        *
        * @param id 
        *           the id to search for
        * @return the descriptor or <code>null</code>
        */
    ChangeTextDescriptorPtr Find(const std::string& id) const;

        /**
        * Return a list of "change texts" defined in the registry.
        *
        * @return the change texts.
        */
    std::vector<ChangeTextDescriptorPtr> GetChangeTexts() const;

  protected:

  private:
    Poco::HashMap<std::string, ChangeTextDescriptorPtr> m_ListRegisteredTexts;

  };

#endif /*CHANGETEXTREGISTRY_H_*/
