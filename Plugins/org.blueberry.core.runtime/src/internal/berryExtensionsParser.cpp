/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryExtensionsParser.h"

#include "berryConfigurationElement.h"
#include "berryExtension.h"
#include "berryExtensionPoint.h"
#include "berryExtensionRegistry.h"
#include "berryIContributor.h"
#include "berryIExtension.h"
#include "berryLog.h"
#include "berryMultiStatus.h"
#include "berryRegistryContribution.h"
#include "berryRegistryMessages.h"
#include "berryRegistryObject.h"
#include "berryRegistryObjectFactory.h"
#include "berryRegistryObjectManager.h"

#include <QTime>

namespace berry {

long ExtensionsParser::cumulativeTime = 0;

// Valid States
const int ExtensionsParser::IGNORED_ELEMENT_STATE = 0;
const int ExtensionsParser::INITIAL_STATE = 1;
const int ExtensionsParser::PLUGIN_STATE = 2;
const int ExtensionsParser::PLUGIN_EXTENSION_POINT_STATE = 5;
const int ExtensionsParser::PLUGIN_EXTENSION_STATE = 6;
const int ExtensionsParser::CONFIGURATION_ELEMENT_STATE = 10;

const int ExtensionsParser::EXTENSION_POINT_INDEX = 0;
const int ExtensionsParser::EXTENSION_INDEX = 1;
const int ExtensionsParser::LAST_INDEX = 1;

const int ExtensionsParser::PARSE_PROBLEM = 1;

const QString ExtensionsParser::PLUGIN = "plugin";
const QString ExtensionsParser::PLUGIN_ID = "id";
const QString ExtensionsParser::PLUGIN_NAME = "name";
const QString ExtensionsParser::BUNDLE_UID = "id";

const QString ExtensionsParser::EXTENSION_POINT = "extension-point";
const QString ExtensionsParser::EXTENSION_POINT_NAME = "name";
const QString ExtensionsParser::EXTENSION_POINT_ID = "id";
const QString ExtensionsParser::EXTENSION_POINT_SCHEMA = "schema";

const QString ExtensionsParser::EXTENSION = "extension";
const QString ExtensionsParser::EXTENSION_NAME = "name";
const QString ExtensionsParser::EXTENSION_ID = "id";
const QString ExtensionsParser::EXTENSION_TARGET = "point";

const QString ExtensionsParser::ELEMENT = "element";
const QString ExtensionsParser::ELEMENT_NAME = "name";
const QString ExtensionsParser::ELEMENT_VALUE = "value";

const QString ExtensionsParser::PROPERTY = "property";
const QString ExtensionsParser::PROPERTY_NAME = "name";
const QString ExtensionsParser::PROPERTY_VALUE = "value";


ExtensionsParser::ExtensionsParser(const SmartPointer<MultiStatus>& status, ExtensionRegistry* registry)
  : locator(nullptr), extractNamespaces(true), status(status),
    registry(registry), resources(nullptr), objectManager(nullptr)
{
}

void ExtensionsParser::setDocumentLocator(QXmlLocator* locator)
{
  this->locator = locator;
}

bool ExtensionsParser::characters(const QString& ch)
{
  int state = stateStack.back();
  if (state != CONFIGURATION_ELEMENT_STATE)
    return true;
  if (state == CONFIGURATION_ELEMENT_STATE)
  {
    // Accept character data within an element, is when it is
    // part of a configuration element (i.e. an element within an EXTENSION element
    ConfigurationElement::Pointer currentConfigElement = objectStack.back().Cast<ConfigurationElement>();
    if (configurationElementValue.isNull())
    {
      if (!ch.trimmed().isEmpty())
      {
        configurationElementValue = ch;
      }
    }
    else
    {
      configurationElementValue.append(ch);
    }
    if (!configurationElementValue.isEmpty())
      currentConfigElement->SetValue(configurationElementValue);
  }
  return true;
}

bool ExtensionsParser::endElement(const QString& /*uri*/, const QString& elementName, const QString& /*qName*/)
{
  switch (stateStack.back())
  {
  case IGNORED_ELEMENT_STATE :
  {
    stateStack.pop();
    return true;
  }
  case INITIAL_STATE :
  {
    // shouldn't get here
    internalError(QString("Element/end element mismatch for element \"%1\".").arg(elementName));
    return false;
  }
  case PLUGIN_STATE :
  {
    stateStack.pop();

    QList<RegistryObject::Pointer>& extensionPoints = scratchVectors[EXTENSION_POINT_INDEX];
    QList<RegistryObject::Pointer>& extensions = scratchVectors[EXTENSION_INDEX];
    QList<int> namespaceChildren;
    namespaceChildren.push_back(0);
    namespaceChildren.push_back(0);
    // Put the extension points into this namespace
    if (extensionPoints.size() > 0)
    {
      namespaceChildren[RegistryContribution::EXTENSION_POINT] = extensionPoints.size();
      for (int i = 0; i < extensionPoints.size(); ++i)
      {
        namespaceChildren.push_back(extensionPoints[i]->GetObjectId());
      }
      extensionPoints.clear();
    }

    // Put the extensions into this namespace too
    if (extensions.size() > 0)
    {
      namespaceChildren[RegistryContribution::EXTENSION] = extensions.size();
      for (int i = 0; i < extensions.size(); ++i)
      {
        namespaceChildren.push_back(extensions[i]->GetObjectId());
      }
      extensions.clear();
    }
    contribution->SetRawChildren(namespaceChildren);
    return true;
  }
  case PLUGIN_EXTENSION_POINT_STATE :
  {
    if (elementName == EXTENSION_POINT)
    {
      stateStack.pop();
    }
    return true;
  }
  case PLUGIN_EXTENSION_STATE :
  {
    if (elementName == EXTENSION)
    {
      stateStack.pop();
      // Finish up extension object
      Extension::Pointer currentExtension = objectStack.pop().Cast<Extension>();
      if (currentExtension->GetNamespaceIdentifier().isEmpty())
        currentExtension->SetNamespaceIdentifier(contribution->GetDefaultNamespace());
      currentExtension->SetContributorId(contribution->GetContributorId());
      scratchVectors[EXTENSION_INDEX].push_back(currentExtension);
    }
    return true;
  }
  case CONFIGURATION_ELEMENT_STATE :
  {
    // We don't care what the element name was
    stateStack.pop();
    // Now finish up the configuration element object
    configurationElementValue.clear();
    ConfigurationElement::Pointer currentConfigElement = objectStack.pop().Cast<ConfigurationElement>();

    QString value = currentConfigElement->GetValueAsIs();
    if (!value.isEmpty())
    {
      currentConfigElement->SetValue(translate(value).trimmed());
    }

    RegistryObject::Pointer parent = objectStack.back().Cast<RegistryObject>();
    // Want to add this configuration element to the subelements of an extension
    QList<int> newValues = parent->GetRawChildren();
    newValues.push_back(currentConfigElement->GetObjectId());
    parent->SetRawChildren(newValues);
    currentConfigElement->SetParentId(parent->GetObjectId());
    currentConfigElement->SetParentType(parent.Cast<ConfigurationElement>() ?
                                          RegistryObjectManager::CONFIGURATION_ELEMENT : RegistryObjectManager::EXTENSION);
    return true;
  }
  default:
    // should never get here
    return false;
  }
}

bool ExtensionsParser::error(const QXmlParseException& ex)
{
  logStatus(ex);
  return true;
}

bool ExtensionsParser::fatalError(const QXmlParseException& ex)
{
  cleanup();
  logStatus(ex);
  return false;
}

bool
ExtensionsParser::parseManifest(QXmlReader* reader, QXmlInputSource* in,
                                const QString& manifestName, RegistryObjectManager* registryObjects,
                                const SmartPointer<RegistryContribution>& currentNamespace,
                                QTranslator* translator)
{
  QTime start;
  this->resources = translator;
  this->objectManager = registryObjects;
  //initialize the parser with this object
  this->contribution = currentNamespace;
  if (registry->Debug())
    start.start();

  if (reader == nullptr)
  {
    cumulativeTime += start.elapsed();
    throw ctkInvalidArgumentException("XML Reader not available");
  }

  locationName = manifestName;
  reader->setContentHandler(this);
  reader->setDeclHandler(this);
  reader->setDTDHandler(this);
  reader->setEntityResolver(this);
  reader->setErrorHandler(this);
  reader->setLexicalHandler(this);
  bool success  = reader->parse(in);

  if (registry->Debug())
  {
    cumulativeTime += start.elapsed();
    BERRY_INFO << "Cumulative parse time so far : " << cumulativeTime;
  }

  return success;
}

bool ExtensionsParser::startDocument()
{
  stateStack.push(INITIAL_STATE);
  for (int i = 0; i <= LAST_INDEX; i++)
  {
    scratchVectors[i].clear();
  }
  return true;
}

bool ExtensionsParser::startElement(const QString& /*uri*/, const QString& elementName,
                                    const QString& /*qName*/, const QXmlAttributes& attributes)
{
  switch (stateStack.back())
  {
  case INITIAL_STATE :
    handleInitialState(elementName, attributes);
    break;
  case PLUGIN_STATE :
    handlePluginState(elementName, attributes);
    break;
  case PLUGIN_EXTENSION_POINT_STATE :
    handleExtensionPointState(elementName);
    break;
  case PLUGIN_EXTENSION_STATE :
  case CONFIGURATION_ELEMENT_STATE :
    handleExtensionState(elementName, attributes);
    break;
  default :
    stateStack.push(IGNORED_ELEMENT_STATE);
    internalError(QString("Unknown element \"%1\", found at the top level, ignored.").arg(elementName));
  }
  return true;
}

bool ExtensionsParser::warning(const QXmlParseException& ex)
{
  logStatus(ex);
  return true;
}

void ExtensionsParser::error(const SmartPointer<IStatus>& error)
{
  status->Add(error);
}

void ExtensionsParser::cleanup()
{
  foreach (RegistryObject::Pointer object, addedRegistryObjects)
  {
    if (ExtensionPoint::Pointer extPoint = object.Cast<ExtensionPoint>())
    {
      QString id = extPoint->GetUniqueIdentifier();
      objectManager->RemoveExtensionPoint(id);
    }
    else
    {
      objectManager->Remove(object->GetObjectId(), true);
    }
  }
}

void ExtensionsParser::handleExtensionPointState(const QString& elementName)
{
  // We ignore all elements under extension points (if there are any)
  stateStack.push(IGNORED_ELEMENT_STATE);
  unknownElement(EXTENSION_POINT, elementName);
}

void ExtensionsParser::handleExtensionState(const QString& elementName, const QXmlAttributes& attributes)
{
  // You need to change the state here even though we will be executing the same
  // code for ExtensionState and ConfigurationElementState.  We ignore the name
  // of the element for ConfigurationElements.  When we are wrapping up, we will
  // want to add each configuration element object to the subElements vector of
  // its parent configuration element object.  However, the first configuration
  // element object we created (the last one we pop off the stack) will need to
  // be added to a vector in the extension object called _configuration.
  stateStack.push(CONFIGURATION_ELEMENT_STATE);

  configurationElementValue.clear();

  // create a new Configuration Element and push it onto the object stack
  ConfigurationElement::Pointer currentConfigurationElement =
      registry->GetElementFactory()->CreateConfigurationElement(contribution->ShouldPersist());
  currentConfigurationElement->SetContributorId(contribution->GetContributorId());
  objectStack.push(currentConfigurationElement);
  currentConfigurationElement->SetName(elementName);

  // Processing the attributes of a configuration element involves creating
  // a new configuration property for each attribute and populating the configuration
  // property with the name/value pair of the attribute.  Note there will be one
  // configuration property for each attribute
  parseConfigurationElementAttributes(attributes);
  objectManager->Add(currentConfigurationElement, true);
  addedRegistryObjects.push_back(currentConfigurationElement);
}

void ExtensionsParser::handleInitialState(const QString& /*elementName*/, const QXmlAttributes& /*attributes*/)
{
  // new manifests should have the plugin (or fragment) element empty
  stateStack.push(PLUGIN_STATE);
  objectStack.push(contribution);
}

void ExtensionsParser::handlePluginState(const QString& elementName, const QXmlAttributes& attributes)
{
  if (elementName == EXTENSION_POINT)
  {
    stateStack.push(PLUGIN_EXTENSION_POINT_STATE);
    parseExtensionPointAttributes(attributes);
    return;
  }
  if (elementName == EXTENSION)
  {
    stateStack.push(PLUGIN_EXTENSION_STATE);
    parseExtensionAttributes(attributes);
    return;
  }

  // If we get to this point, the element name is one we don't currently accept.
  // Set the state to indicate that this element will be ignored
  stateStack.push(IGNORED_ELEMENT_STATE);
  unknownElement(PLUGIN, elementName);
}

void ExtensionsParser::logStatus(const QXmlParseException& ex)
{
  QString name = ex.systemId();
  if (name.isEmpty())
    name = locationName;

  if (!name.isEmpty())
    name = name.mid(1 + name.lastIndexOf("/"));

  QString msg;
  if (name.isEmpty())
    msg = QString("Parsing error: \"%1\"").arg(ex.message());
  else
    msg = QString("Parsing error in \"%1\" [line %2, column %3]: \"%4\".").arg(name)
        .arg(ex.lineNumber()).arg(ex.columnNumber()).arg(ex.message());
  IStatus::Pointer status(new Status(IStatus::WARNING_TYPE, RegistryMessages::OWNER_NAME,
                                     PARSE_PROBLEM, msg, BERRY_STATUS_LOC));
  error(status);
}

void ExtensionsParser::parseConfigurationElementAttributes(const QXmlAttributes& attributes)
{
  ConfigurationElement::Pointer parentConfigurationElement =
      objectStack.back().Cast<ConfigurationElement>();

  // process attributes
  int len = attributes.length();
  if (len == 0)
  {
    parentConfigurationElement->SetProperties(QList<QString>());
    return;
  }
  QList<QString> properties;
  for (int i = 0; i < len; i++)
  {
    properties.push_back(attributes.localName(i));
    properties.push_back(translate(attributes.value(i)));
  }
  parentConfigurationElement->SetProperties(properties);
}

void ExtensionsParser::parseExtensionAttributes(const QXmlAttributes& attributes)
{
  Extension::Pointer currentExtension = registry->GetElementFactory()->CreateExtension(contribution->ShouldPersist());
  objectStack.push(currentExtension);

  QString simpleId;
  QString namespaceName;
  // Process Attributes
  int len = attributes.length();
  for (int i = 0; i < len; i++)
  {
    QString attrName = attributes.localName(i);
    QString attrValue = attributes.value(i).trimmed();

    if (attrName == EXTENSION_NAME)
      currentExtension->SetLabel(translate(attrValue));
    else if (attrName == EXTENSION_ID)
    {
      int simpleIdStart = attrValue.lastIndexOf('.');
      if ((simpleIdStart != -1) && extractNamespaces)
      {
        simpleId = attrValue.mid(simpleIdStart + 1);
        namespaceName = attrValue.left(simpleIdStart);
      }
      else
      {
        simpleId = attrValue;
        namespaceName = contribution->GetDefaultNamespace();
      }
      currentExtension->SetSimpleIdentifier(simpleId);
      currentExtension->SetNamespaceIdentifier(namespaceName);
    }
    else if (attrName == EXTENSION_TARGET)
    {
      // check if point is specified as a simple or qualified name
      QString targetName;
      if (attrValue.lastIndexOf('.') == -1)
      {
        QString baseId = contribution->GetDefaultNamespace();
        targetName = baseId + '.' + attrValue;
      }
      else
      {
        targetName = attrValue;
      }
      currentExtension->SetExtensionPointIdentifier(targetName);
    }
    else
    {
      unknownAttribute(attrName, EXTENSION);
    }
  }
  if (currentExtension->GetExtensionPointIdentifier().isEmpty())
  {
    missingAttribute(EXTENSION_TARGET, EXTENSION);
    stateStack.pop();
    stateStack.push(IGNORED_ELEMENT_STATE);
    objectStack.pop();
    return;
  }
  // if we have an Id specified, check for duplicates. Only issue warning (not error) if duplicate found
  // as it might still work fine - depending on the access pattern.
  if (!simpleId.isEmpty() && registry->Debug())
  {
    QString uniqueId = namespaceName + '.' + simpleId;
    IExtension::Pointer existingExtension = registry->GetExtension(uniqueId);
    if (existingExtension.IsNotNull())
    {
      QString currentSupplier = contribution->GetDefaultNamespace();
      QString existingSupplier = existingExtension->GetContributor()->GetName();
      QString msg = QString("Extensions supplied by \"%1\" and \"%2\" have the same Id: \"%3\".")
          .arg(currentSupplier).arg(existingSupplier).arg(uniqueId);
      IStatus::Pointer status(new Status(IStatus::WARNING_TYPE, RegistryMessages::OWNER_NAME, 0,
                                         msg, BERRY_STATUS_LOC));
      registry->Log(status);
    }
    else if (!processedExtensionIds.isEmpty())
    { // check elements in this contribution
      foreach (QString processedId, processedExtensionIds)
      {
        if (uniqueId == processedId)
        {
          QString currentSupplier = contribution->GetDefaultNamespace();
          QString existingSupplier = currentSupplier;
          QString msg = QString("Extensions supplied by \"%1\" and \"%2\" have the same Id: \"%3\".")
              .arg(currentSupplier).arg(existingSupplier).arg(uniqueId);
          IStatus::Pointer status(new Status(IStatus::WARNING_TYPE, RegistryMessages::OWNER_NAME, 0,
                                             msg, BERRY_STATUS_LOC));
          registry->Log(status);
          break;
        }
      }
    }
    processedExtensionIds.push_back(uniqueId);
  }

  objectManager->Add(currentExtension, true);
  addedRegistryObjects.push_back(currentExtension);
}

void ExtensionsParser::missingAttribute(const QString& attribute, const QString& element)
{
  if (locator == nullptr)
    internalError(QString("Missing \"%1\" attribute in \"%2\" element. Element ignored.").arg(attribute).arg(element));
  else
    internalError(QString("Missing \"%1\" attribute in \"%2\" element (line: %3). Element ignored.")
                  .arg(attribute).arg(element).arg(locator->lineNumber()));
}

void ExtensionsParser::unknownAttribute(const QString& attribute, const QString& element)
{
  if (locator == nullptr)
    internalError(QString("Unknown attribute \"%1\" for element \"%2\" ignored.").arg(attribute).arg(element));
  else
    internalError(QString("Unknown attribute \"%1\" for element \"%2\" ignored (line: %3).")
                  .arg(attribute).arg(element).arg(locator->lineNumber()));
}

void ExtensionsParser::unknownElement(const QString& parent, const QString& element)
{
  if (locator == nullptr)
    internalError(QString("Unknown element \"%1\", found within a \"%2\", ignored.").arg(element).arg(parent));
  else
    internalError(QString("Unknown element \"%1\", found within a \"%2\", ignored (line: %3).")
                  .arg(element).arg(parent).arg(locator->lineNumber()));
}

void ExtensionsParser::parseExtensionPointAttributes(const QXmlAttributes& attributes)
{
  ExtensionPoint::Pointer currentExtPoint =
      registry->GetElementFactory()->CreateExtensionPoint(contribution->ShouldPersist());

  // Process Attributes
  int len = attributes.length();
  for (int i = 0; i < len; i++)
  {
    QString attrName = attributes.localName(i);
    QString attrValue = attributes.value(i).trimmed();

    if (attrName == EXTENSION_POINT_NAME)
      currentExtPoint->SetLabel(translate(attrValue));
    else if (attrName == EXTENSION_POINT_ID)
    {
      QString uniqueId;
      QString namespaceName;
      int simpleIdStart = attrValue.lastIndexOf('.');
      if (simpleIdStart != -1 && extractNamespaces)
      {
        namespaceName = attrValue.left(simpleIdStart);
        uniqueId = attrValue;
      }
      else
      {
        namespaceName = contribution->GetDefaultNamespace();
        uniqueId = namespaceName + '.' + attrValue;
      }
      currentExtPoint->SetUniqueIdentifier(uniqueId);
      currentExtPoint->SetNamespace(namespaceName);

    }
    else if (attrName == EXTENSION_POINT_SCHEMA)
      currentExtPoint->SetSchema(attrValue);
    else
      unknownAttribute(attrName, EXTENSION_POINT);
  }
  if (currentExtPoint->GetSimpleIdentifier().isEmpty() || currentExtPoint->GetLabel().isEmpty())
  {
    QString attribute = currentExtPoint->GetSimpleIdentifier().isEmpty() ? EXTENSION_POINT_ID : EXTENSION_POINT_NAME;
    missingAttribute(attribute, EXTENSION_POINT);
    stateStack.pop();
    stateStack.push(IGNORED_ELEMENT_STATE);
    return;
  }
  if (!objectManager->AddExtensionPoint(currentExtPoint, true))
  {
    // avoid adding extension point second time as it might cause
    // extensions associated with the existing extension point to
    // become inaccessible.
    if (registry->Debug())
    {
      QString msg = QString("Ignored duplicate extension point \"%1\" supplied by \"%2\".")
          .arg(currentExtPoint->GetUniqueIdentifier()).arg(contribution->GetDefaultNamespace());
      IStatus::Pointer status(new Status(IStatus::ERROR_TYPE, RegistryMessages::OWNER_NAME, 0, msg, BERRY_STATUS_LOC));
      registry->Log(status);
    }
    stateStack.pop();
    stateStack.push(IGNORED_ELEMENT_STATE);
    return;
  }
  if (currentExtPoint->GetNamespace().isEmpty())
    currentExtPoint->SetNamespace(contribution->GetDefaultNamespace());
  currentExtPoint->SetContributorId(contribution->GetContributorId());
  addedRegistryObjects.push_back(currentExtPoint);

  // Now populate the the vector just below us on the objectStack with this extension point
  scratchVectors[EXTENSION_POINT_INDEX].push_back(currentExtPoint);
}

void ExtensionsParser::internalError(const QString& message)
{
  IStatus::Pointer status(new Status(IStatus::WARNING_TYPE, RegistryMessages::OWNER_NAME,
                                     PARSE_PROBLEM, message, BERRY_STATUS_LOC));
  error(status);
}

QString ExtensionsParser::translate(const QString& key) const
{
  return registry->Translate(key, resources);
}

}
