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
#ifndef BERRYXMLPREFERENCES_H_
#define BERRYXMLPREFERENCES_H_

#include <org_blueberry_core_runtime_Export.h>
#include "berryAbstractPreferencesStorage.h"

#include "Poco/Path.h"
#include "Poco/DOM/Node.h"
#include "Poco/DOM/Document.h"
#include "Poco/AutoPtr.h"

namespace berry
{
  class Preferences;
  ///
  /// Implementation of the OSGI Preferences Interface.
  /// Wraps a DOMNode.
  ///
  class BERRY_RUNTIME XMLPreferencesStorage: public AbstractPreferencesStorage
  {
  public:
    ///
    /// For use with berry::SmartPtr
    ///
    berryObjectMacro(berry::XMLPreferencesStorage);
    ///
    /// Construct a new XML-based PreferencesStorage
    ///
    XMLPreferencesStorage(const Poco::File& _File);

    ///
    /// To be implemented in the subclasses.
    ///
    virtual void Flush(IPreferences* _Preferences) throw(Poco::Exception, BackingStoreException);

    ///
    /// Nothing to do here
    ///
    virtual ~XMLPreferencesStorage();

  protected:
    ///
    /// Construct Preferences tree from DOM tree (recursive)
    /// Construct a PreferencesNode from the DOMNode by using prefParentNode
    ///
    void ToPreferencesTree(Poco::XML::Node* DOMNode, Preferences* prefParentNode);
    ///
    /// Construct DOM tree from Preferences tree (recursive)
    /// Construct a DOMNode from the prefNode by using parentDOMNode
    ///
    void ToDOMTree(Preferences* prefNode, Poco::XML::Node* parentDOMNode);
  protected:
    Poco::AutoPtr<Poco::XML::Document> m_Document;

  };
}
#endif /* BERRYXMLPREFERENCES_H_ */
