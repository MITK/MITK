/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYEXTENSIONSPARSER_H
#define BERRYEXTENSIONSPARSER_H

#include "berrySmartPointer.h"

#include <QStack>
#include <QXmlDefaultHandler>

class QTranslator;

namespace berry {

struct IStatus;

class ExtensionRegistry;
class MultiStatus;
class Object;
class RegistryContribution;
class RegistryObject;
class RegistryObjectManager;

class ExtensionsParser : public QXmlDefaultHandler
{

private:

  static long cumulativeTime; // = 0;

  // Valid States
  static const int IGNORED_ELEMENT_STATE; // = 0;
  static const int INITIAL_STATE; // = 1;
  static const int PLUGIN_STATE; // = 2;
  static const int PLUGIN_EXTENSION_POINT_STATE; // = 5;
  static const int PLUGIN_EXTENSION_STATE; // = 6;
  static const int CONFIGURATION_ELEMENT_STATE; // = 10;

  // Keep a group of vectors as a temporary scratch space.  These
  // vectors will be used to populate arrays in the bundle model
  // once processing of the XML file is complete.
  static const int EXTENSION_POINT_INDEX; // = 0;
  static const int EXTENSION_INDEX; // = 1;
  static const int LAST_INDEX; // = 1;

  QList<SmartPointer<RegistryObject> > scratchVectors[2];

  QXmlLocator* locator;

  // Cache the behavior toggle (true: attempt to extract namespace from qualified IDs)
  bool extractNamespaces;

  QList<QString> processedExtensionIds;

  // Keep track of elements added into the registry manager in case we encounter a error
  // and need to rollback
  QList<SmartPointer<RegistryObject> > addedRegistryObjects;

  // File name for this extension manifest
  // This to help with error reporting
  QString locationName;

  // Current State Information
  QStack<int> stateStack;

  // Current object stack (used to hold the current object we are
  // populating in this plugin descriptor
  QStack<SmartPointer<Object> > objectStack;

  QString schemaVersion;

  // A status for holding results.
  SmartPointer<MultiStatus> status;

  // Owning extension registry
  ExtensionRegistry* registry;

  // Resource bundle used to translate the content of the plugin.xml
  QTranslator* resources;

  // Keep track of the object encountered.
  RegistryObjectManager* objectManager;

  SmartPointer<RegistryContribution> contribution;

  //This keeps tracks of the value of the configuration element in case the value comes in several pieces (see characters()). See as well bug 75592.
  QString configurationElementValue;

public:

  /**
   * Status code constant (value 1) indicating a problem in a bundle extensions
   * manifest (<code>extensions.xml</code>) file.
   */
  static const int PARSE_PROBLEM; // = 1;

  static const QString PLUGIN; // = "plugin";
  static const QString PLUGIN_ID; // = "id";
  static const QString PLUGIN_NAME; // = "name";
  static const QString BUNDLE_UID; // = "id";

  static const QString EXTENSION_POINT; // = "extension-point";
  static const QString EXTENSION_POINT_NAME; // = "name";
  static const QString EXTENSION_POINT_ID; // = "id";
  static const QString EXTENSION_POINT_SCHEMA; // = "schema";

  static const QString EXTENSION; // = "extension";
  static const QString EXTENSION_NAME; // = "name";
  static const QString EXTENSION_ID; // = "id";
  static const QString EXTENSION_TARGET; // = "point";

  static const QString ELEMENT; // = "element";
  static const QString ELEMENT_NAME; // = "name";
  static const QString ELEMENT_VALUE; // = "value";

  static const QString PROPERTY; // = "property";
  static const QString PROPERTY_NAME; // = "name";
  static const QString PROPERTY_VALUE; // = "value";


  ExtensionsParser(const SmartPointer<MultiStatus>& status, ExtensionRegistry* registry);

  /*
   * @see QXmlDefaultHandler#setDocumentLocator(QXmlLocator*)
   */
  void setDocumentLocator(QXmlLocator* locator) override;

  /*
   * @see QXmlDefaultHandler#characters(const QString&)
   */
  bool characters(const QString& ch) override;

  /*
   * @see QXmlDefaultHandler#endElement(const QString&, const QString&, const QString&)
   */
  bool endElement(const QString& uri, const QString& elementName, const QString& qName) override;

  /*
   * @see QXmlDefaultHandler#error(const QXmlParseException&)
   */
  bool error(const QXmlParseException& ex) override;

  /*
   * @see QXmlDefaultHandler#fatalError(const QXmlParseException&)
   */
  bool fatalError(const QXmlParseException& ex) override;

  bool parseManifest(QXmlReader* reader, QXmlInputSource* in,
                     const QString& manifestName,
                     RegistryObjectManager* registryObjects,
                     const SmartPointer<RegistryContribution>& currentNamespace,
                     QTranslator* translator);

  /*
   * @see QXmlDefaultHandler#startDocument()
   */
  bool startDocument() override;

  /*
   * @see QXmlDefaultHandler#startElement(const QString&, const QString&, const QString&, const QXmlAttributes&)
   */
  bool startElement(const QString& uri, const QString& elementName,
                    const QString& qName, const QXmlAttributes& attributes) override;

  /*
   * @see QXmlDefaultHandler#warning(const QXmlParseException&)
   */
  bool warning(const QXmlParseException& ex) override;

  /**
   * Handles an error state specified by the status.  The collection of all logged status
   * objects can be accessed using <code>getStatus()</code>.
   *
   * @param error a status detailing the error condition
   */
  void error(const SmartPointer<IStatus>& error);

private:

  /**
   * Remove all elements that we have added so far into registry manager
   */
  void cleanup();

  void handleExtensionPointState(const QString& elementName);

  void handleExtensionState(const QString& elementName, const QXmlAttributes& attributes);

  void handleInitialState(const QString& elementName, const QXmlAttributes& attributes);

  void handlePluginState(const QString& elementName, const QXmlAttributes& attributes);

  void logStatus(const QXmlParseException& ex);

  void parseConfigurationElementAttributes(const QXmlAttributes& attributes);

  void parseExtensionAttributes(const QXmlAttributes& attributes);

  //todo: Are all three methods needed??
  void missingAttribute(const QString& attribute, const QString& element);

  void unknownAttribute(const QString& attribute, const QString& element);

  void unknownElement(const QString& parent, const QString& element);

  void parseExtensionPointAttributes(const QXmlAttributes& attributes);

  void internalError(const QString& message);

  QString translate(const QString& key) const;

};

}

#endif // BERRYEXTENSIONSPARSER_H
