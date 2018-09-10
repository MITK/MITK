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

#ifndef SegmentationReworkREST_h
#define SegmentationReworkREST_h

#include <mitkRestServer.h>

class SegmentationReworkREST : public mitk::RESTServer {
public:
  SegmentationReworkREST(utility::string_t url);
  ~SegmentationReworkREST();

  void HandlePut(MitkRequest message);
};

#endif // SegmentationReworkREST_h