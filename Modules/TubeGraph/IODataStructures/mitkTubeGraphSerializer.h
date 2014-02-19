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

#ifndef mitkTubeGraphSerializer_h_included
#define mitkTubeGraphSerializer_h_included

#include "MitkTubeGraphExports.h"

#include "mitkBaseDataSerializer.h"



namespace mitk
{

class MitkTubeGraph_EXPORT TubeGraphSerializer : public BaseDataSerializer
{
  public:

    mitkClassMacro( TubeGraphSerializer, BaseDataSerializer );
    itkNewMacro(Self);
    virtual std::string Serialize();

  protected:

    TubeGraphSerializer();
    virtual ~TubeGraphSerializer();
};
} // namespace
#endif
