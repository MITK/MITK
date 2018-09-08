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

#include "mitkRESTServer.h"

#include <mitkCommon.h>

mitk::RESTServer::RESTServer() {}

mitk::RESTServer::RESTServer(utility::string_t url) : m_Listener(url) {}

mitk::RESTServer::~RESTServer() {}

void mitk::RESTServer::HandleError(pplx::task<void> &t)
{
  try
  {
    t.get();
  }
  catch (...)
  {
    mitkThrow() << "An error occured.";
  }
}