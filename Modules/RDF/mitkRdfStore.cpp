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

#include "mitkRdfStore.h"
#include "mitkRdfUri.h"
#include "mitkRdfTriple.h"
#include "mitkRdfNode.h"

#include <redland.h>

namespace mitk {

class RdfStorePrivate {

public:
  RdfStorePrivate();
  ~RdfStorePrivate();
};

RdfStore::RdfStore()
  : m_BaseUri(0), m_World(0), m_Storage(0), m_Model(0)
{
  // SetUp base prefixes
  m_Prefixes["rdf"] = RdfUri("http://www.w3.org/1999/02/22-rdf-syntax-ns#");
  m_Prefixes["rdfs"] = RdfUri("http://www.w3.org/2000/01/rdf-schema#");
  m_Prefixes["xsd"] = RdfUri("http://www.w3.org/2001/XMLSchema#");
  m_Prefixes["owl"] = RdfUri("http://www.w3.org/2002/07/owl#");

  // SetUp new store
  CleanUp();
}

RdfStore::~RdfStore()
{
  if(m_Model)
    librdf_free_model(m_Model);
  if(m_Storage)
    librdf_free_storage(m_Storage);
  if(m_World)
    librdf_free_world(m_World);
}

void RdfStore::SetBaseUri(RdfUri uri)
{
  m_BaseUri = uri;
  m_Prefixes[""] = m_BaseUri;
}

RdfUri RdfStore::GetBaseUri()
{
  return m_BaseUri;
}

void RdfStore::AddPrefix(std::string prefix, RdfUri uri)
{
  m_Prefixes[prefix] = uri;
}

void RdfStore::CleanUp()
{
  // CleanUp old Store if there is one
  if(m_Model)
    librdf_free_model(m_Model);
  if(m_Storage)
    librdf_free_storage(m_Storage);
  if(m_World)
    librdf_free_world(m_World);

  // SetUp new Store
  m_World = librdf_new_world();
  librdf_world_open(m_World);
  m_Storage = librdf_new_storage(m_World, "memory", 0, 0);
  m_Model = librdf_new_model(m_World, m_Storage, 0);
}

bool RdfStore::Add(RdfTriple triple)
{
  librdf_statement* statement = RdfTripleToStatement(triple);

  if (CheckComplete(statement))
  {
    librdf_free_statement(statement);
    return false;
  }

  // Store already contains statement
  if (Contains(triple)) return false;

  if (librdf_model_add_statement(m_Model, statement) != 0) {
    librdf_free_statement(statement);
    return false;
  }
  else
  {
    librdf_free_statement(statement);
    return true;
  }
  return true;
}

bool RdfStore::Remove(RdfTriple triple)
{
  librdf_statement* statement = RdfTripleToStatement(triple);

  if (CheckComplete(statement))
  {
    librdf_free_statement(statement);
    return false;
  }

  // Store does not contain statement
  if (!Contains(triple)) return false;

  if (librdf_model_remove_statement(m_Model, statement) != 0) {
    librdf_free_statement(statement);
    return false;
  }
  else
  {
    librdf_free_statement(statement);
    return true;
  }
  return false;
}

bool RdfStore::Contains(RdfTriple triple)
{
  librdf_statement* statement = RdfTripleToStatement(triple);

  // if 0 there is no triple
  if (librdf_model_contains_statement(m_Model, statement) == 0) {
    librdf_free_statement(statement);
    return false;
  }
  else
  {
    librdf_free_statement(statement);
    return true;
  }
  return false;
}

void RdfStore::Save(std::string filename)
{
  SaveAs(filename, "turtle");
}

void RdfStore::SaveAs(std::string filename, std::string format)
{
  librdf_uri* baseUri = RdfUriToLibRdfUri(m_BaseUri);
  librdf_serializer* s = librdf_new_serializer(m_World, format.c_str(), 0, 0);

  for (PrefixMap::const_iterator i = m_Prefixes.begin(); i != m_Prefixes.end(); ++i)
  {
    librdf_serializer_set_namespace(s, RdfUriToLibRdfUri(i->second), i->first.c_str());
  }
  // May this is not relevant
  librdf_serializer_set_namespace(s, RdfUriToLibRdfUri(m_BaseUri), "");

  FILE* f = fopen(filename.c_str(), "w+");

  librdf_serializer_serialize_model_to_file_handle(s, f, baseUri, m_Model);

  librdf_free_serializer(s);
  librdf_free_uri(baseUri);
}

void RdfStore::Import(std::string url, std::string format)
{
  std::string baseUri = m_BaseUri.ToString();

  if (baseUri.empty())
  {
    baseUri = url;
  }

  // Redland uses relative paths like file:readme.txt
  if (false)
  {
    /* TODO */
  }
}

RdfStore* RdfStore::Load(std::string baseUri, std::string format)
{
  RdfStore* store = new RdfStore();
  store->SetBaseUri(RdfUri(baseUri));
  store->Import(baseUri, format);
  return store;
}

/*****************************************************************************
 ********************************** Private **********************************
 *****************************************************************************/

bool RdfStore::CheckComplete(librdf_statement* statement)
{
  if (librdf_statement_is_complete(statement) != 0) return true;
  else return false;
}

librdf_statement* RdfStore::RdfTripleToStatement(RdfTriple triple)
{
  librdf_node* subject = RdfNodeToLibRdfNode(triple.GetSubject());
  librdf_node* predicate = RdfNodeToLibRdfNode(triple.GetPredicate());
  librdf_node* object = RdfNodeToLibRdfNode(triple.GetObject());

  librdf_statement* statement = librdf_new_statement_from_nodes(m_World, subject, predicate, object);
  if(!statement) return NULL;
  return statement;
}

librdf_node* RdfStore::RdfNodeToLibRdfNode(RdfNode node)
{
  librdf_node* newNode = NULL;

  switch (node.type)
  {
  case RdfNode::Nothing:
    break;
  case RdfNode::Blank:
    newNode = librdf_new_node_from_blank_identifier(m_World, (const unsigned char*) node.value.c_str());
    break;
  case RdfNode::Literal:
    {
      if (node.datatype != RdfUri())
      {
        librdf_uri* typeUri = RdfUriToLibRdfUri(node.datatype);
        newNode = librdf_new_node_from_typed_literal(m_World, (const unsigned char*) node.value.c_str(), 0, typeUri);
      }
      else
      {
        newNode = librdf_new_node_from_literal(m_World, (const unsigned char*) node.value.c_str(), 0, 0);
      }
    }
    break;
  case RdfNode::URI:
    newNode = librdf_new_node_from_uri( m_World, librdf_new_uri(m_World, (const unsigned char*) node.value.c_str()) );
    break;
  default:
    break;
  }
  return newNode;
}

librdf_uri* RdfStore::RdfUriToLibRdfUri(RdfUri uri)
{
  librdf_uri* libUri = librdf_new_uri(m_World, (const unsigned char*) uri.ToString().c_str());
  if (!libUri) return NULL;
  return libUri;
}

RdfTriple RdfStore::StatementToRdfTriple(librdf_statement* statement)
{
  librdf_node *subject = librdf_statement_get_subject(statement);
  librdf_node *predicate = librdf_statement_get_predicate(statement);
  librdf_node *object = librdf_statement_get_object(statement);

  RdfTriple triple(LibRdfNodeToRdfNode(subject),
    LibRdfNodeToRdfNode(predicate),
    LibRdfNodeToRdfNode(object));

  return triple;
}

RdfNode RdfStore::LibRdfNodeToRdfNode(librdf_node* node)
{
  RdfNode mitkNode = NULL;

  if (!node) return mitkNode;

  if (librdf_node_is_resource(node))
  {
    mitkNode.type = RdfNode::URI;
    librdf_uri *uri = librdf_node_get_uri(node);
    mitkNode.value = LibRdfUriToRdfUri(uri).ToString();
  }
  else if (librdf_node_is_literal(node))
  {
    mitkNode.type = RdfNode::Literal;
    std::string value = (const char*) librdf_node_get_literal_value(node);
    if (!value.empty()) mitkNode.value = value;
    librdf_uri* typeUri = librdf_node_get_literal_value_datatype_uri(node);
    if (typeUri) mitkNode.datatype = LibRdfUriToRdfUri(typeUri);
  }
  else if (librdf_node_is_blank(node))
  {
    mitkNode.type = RdfNode::Blank;
    std::string str = (const char*) librdf_node_get_blank_identifier(node);
    if (!str.empty()) mitkNode.value = str;
  }
  return mitkNode;
}

RdfUri RdfStore::LibRdfUriToRdfUri(librdf_uri* uri)
{
  std::string str = (const char*) librdf_uri_as_string(uri);
  if (!str.empty()) return RdfUri(str);

  return RdfUri();
}

} // end of namespace mitk
