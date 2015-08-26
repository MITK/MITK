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
#include <list>

#include "mitkRdfTriple.h"
#include <mitkCommon.h>

namespace mitk {
  class RdfStorePrivate;

  /**
  * \ingroup MitkRDFModule
  */
  class MITKRDF_EXPORT RdfStore
  {
  public:

    typedef std::map<std::string, std::list<RdfNode> > ResultMap;
    typedef std::map<std::string, RdfUri> PrefixMap;

    /**
    * Construct a new triplestore.
    */
    RdfStore();

    /**
    * Destruct a triplestore.
    */
    ~RdfStore();

    /**
    * Set the base URI of the triplestore.
    * @param uri An URI which is the base for the triplestore and for new nodes.
    */
    void SetBaseUri(RdfUri uri);

    /**
    * Get the base URI of the triplestore.
    */
    RdfUri GetBaseUri() const;

    /**
    * Add a new prefix which represents an URI as an abbreviation to the triplestore.
    * @param prefix The short form of an URI.
    * @param uri The full form of an URI.
    */
    void AddPrefix(std::string prefix, RdfUri uri);

    /**
    * Get a Map with all prefixes of the triplestore.
    * @return A Map with all Prefixes of the RdfStore.
    */
    PrefixMap GetPrefixes() const;

    /**
    * Clean up the triplestore to the state of a new store.
    */
    void CleanUp();

    /**
    * Add a new triple to the triplestore.
    * Checks if the triplestore contains the triple.
    * @param triple A triple.
    * @return If the triple is successfully added or if the triplestore already contains the triple, true will be returned. If none of the previous options happen, false will be returned.
    */
    bool Add(RdfTriple triple);

    /**
    * Remove a triple from the triplestore.
    * Checks if the triplestore contains the triple.
    * @param triple A triple.
    * @return If the triple is successfully removed or if the triplestore doesn't contain the triple, true will be returned. If none of the previous options happen, false will be returned.
    */
    bool Remove(RdfTriple triple);

    /**
    * Checks if the triplestore contains the triple.
    * @param triple A triple.
    * @return If the triplestore contains the triple, true will be returned. Otherwise, false will be returned.
    */
    bool Contains(RdfTriple triple);

    /**
    * Queries over the triplestore with the given SPARQL query.
    * @param query A std:string which stands for a SPARQL query text.
    * @return The result of the query will be returned as a map of keys with their values as lists of nodes.
    * @deprecatedSince{2015_11} Use mitk::RdfStore::ExecuteTupleQuery() instead.
    */
    DEPRECATED(ResultMap Query(const std::string& query) const);

    /**
    * Queries over the triplestore with the given SPARQL query.
    * @param query A std:string which stands for a SPARQL query text.
    * @return The result of the query will be returned as a map of keys with their values as lists of nodes.
    */
    ResultMap ExecuteTupleQuery(const std::string& query) const;

    /**
     * Tests whether or not the specified query pattern has a solution.
     * @param query The SPARQL query string in ASK form.
     * @return True if query pattern has a solution, otherwise false.
     * @throws mitk::Exception This exception is thrown if one of the following errors occur:
     *                         (1) failure on query object creation, (2) SPARQL syntax error,
     *                         (3) trying to execute a non-boolean query, (4) error while retrieving execution result
     */
    bool ExecuteBooleanQuery(const std::string& query) const;

    /**
    * Saves the current state of the triplestore in a file. The currently supported formats are: "ntriples", "turtle"(default), "nquads".
    * @param filename A full filepath to the lokal storage.
    * @param format One of the supported formats. Default: "turtle".
    */
    void Save(std::string filename, std::string format = "");

    /**
    * Imports the state of the triplestore of an URL (URI).
    * @param filename A full filepath to the lokal storage or http address as URL. A lokal file path has to look like "file:YOURPATH" ( Example: file:D:/home/readme.txt ).
    * @param format The current supported formats are: "turtle" (default), "ntriples", "nquads".
    */
    void Import(std::string url, std::string format = "");

  private:
    RdfStorePrivate* d;
  };
}

#endif // MITKRDFSTORE_H
