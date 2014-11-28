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

#ifndef MITKRDFSTORE_H
#define MITKRDFSTORE_H

#include <MitkRDFExports.h>
#include <map>

#include <redland.h>

#include "mitkRdfUri.h"

namespace mitk {

class RdfTriple;
class RdfNode;

class MitkRDF_EXPORT RdfStore
{
public:
  RdfStore();
  ~RdfStore();

  void SetBaseUri(RdfUri uri);
  RdfUri GetBaseUri();

  void AddPrefix(std::string prefix, RdfUri uri);
  void CleanUp();

  bool Add(RdfTriple triple);
  bool Remove(RdfTriple triple);

  bool Contains(RdfTriple triple);

  void Save(std::string filename);
  void SaveAs(std::string filename, std::string format);

  void Import(std::string url, std::string format);

  static RdfStore* Load(std::string baseUri, std::string format="");

private:
  RdfUri m_BaseUri;

  typedef std::map<std::string, RdfUri> PrefixMap;
  PrefixMap m_Prefixes;

  librdf_model* m_Model;
  librdf_storage* m_Storage;
  librdf_world* m_World;

  librdf_statement* RdfTripleToStatement(RdfTriple triple);
  librdf_node* RdfNodeToLibRdfNode(RdfNode node);
  librdf_uri* RdfUriToLibRdfUri(RdfUri uri);

  RdfTriple StatementToRdfTriple(librdf_statement* statement);
  RdfNode LibRdfNodeToRdfNode(librdf_node* node);
  RdfUri LibRdfUriToRdfUri(librdf_uri* uri);

  bool CheckComplete(librdf_statement* statement);
};

}

#endif // MITKRDFSTORE_H
