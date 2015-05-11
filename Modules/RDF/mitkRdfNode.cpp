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

#include "mitkRdfNode.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>

#include <redland.h>

namespace mitk {

bool RdfNode::dummy()
{
  librdf_world* world;
  librdf_storage *storage;
  librdf_model* model;
  librdf_statement* statement;
  raptor_world *raptor_world_ptr;
  raptor_iostream* iostr;

  world=librdf_new_world();
  librdf_world_open(world);
  raptor_world_ptr = librdf_world_get_raptor(world);

  model=librdf_new_model(world, storage=librdf_new_storage(world, "hashes", nullptr, "hash-type='memory'"), nullptr);

  librdf_model_add_statement(model,
                             statement=librdf_new_statement_from_nodes(world, librdf_new_node_from_uri_string(world, (const unsigned char*)"http://www.dajobe.org/"),
                                                             librdf_new_node_from_uri_string(world, (const unsigned char*)"http://purl.org/dc/elements/1.1/creator"),
                                                             librdf_new_node_from_literal(world, (const unsigned char*)"Dave Beckett", nullptr, 0)
                                                             )
                             );

  librdf_free_statement(statement);

  iostr = raptor_new_iostream_to_file_handle(raptor_world_ptr, stdout);
  librdf_model_write(model, iostr);
  raptor_free_iostream(iostr);

  librdf_free_model(model);
  librdf_free_storage(storage);

  librdf_free_world(world);

#ifdef LIBRDF_MEMORY_DEBUG
  librdf_memory_report(stderr);
#endif

  return true;
}

}
