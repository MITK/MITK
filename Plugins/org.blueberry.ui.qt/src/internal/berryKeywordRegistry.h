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

#ifndef BERRYKEYWORDREGISTRY_H
#define BERRYKEYWORDREGISTRY_H

#include <berryIExtensionChangeHandler.h>

namespace berry {

struct IExtensionPoint;

/**
 * Contains extensions defined on the <code>keywords</code> extension point.
 */
class KeywordRegistry : public IExtensionChangeHandler
{

private:

  static const QString ATT_ID; // = "id"

  static const QString ATT_LABEL; // = "label"

  static const QString TAG_KEYWORD; // = "keyword"

  /**
   * Map of id->labels.
   */
  QHash<QString, QString> internalKeywordMap;

  /**
   * Private constructor.
   */
  KeywordRegistry();


  SmartPointer<IExtensionPoint> GetExtensionPointFilter() const;


public:

  /**
   * Return the singleton instance of the <code>KeywordRegistry</code>.
   *
   * @return the singleton registry
   */
  static KeywordRegistry* GetInstance();

  void AddExtension(IExtensionTracker* tracker, const SmartPointer<IExtension>& extension) override;

  /**
   * Return the label associated with the given keyword.
   *
   * @param id the keyword id
   * @return the label or <code>null</code>
   */
  QString GetKeywordLabel(const QString& id);

  void RemoveExtension(const SmartPointer<IExtension>& extension, const QList<SmartPointer<Object> >& objects) override;

};

}

#endif // BERRYKEYWORDREGISTRY_H
