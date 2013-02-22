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


#ifndef TbssImageMapper__cpp
#define TbssImageMapper__cpp

#include "mitkTbssImageMapper.h"

#include "mitkProperties.h"
#include "mitkTbssImage.h"



mitk::TbssImageMapper::TbssImageMapper()
{
}


mitk::TbssImageMapper::~TbssImageMapper()
{
}


void mitk::TbssImageMapper::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
  int displayIndex(0);


  this->GetDataNode()->GetIntProperty( "DisplayChannel", displayIndex, renderer );
  mitk::Image *input = const_cast< mitk::Image* >(
    this->GetInput()
    );
  mitk::TbssImage *input2 = dynamic_cast< mitk::TbssImage* >(
    input
    );

  MITK_INFO << "displayindex: " << displayIndex;


  input2->SetDisplayIndexForRendering(displayIndex);
  Superclass::GenerateDataForRenderer(renderer);
}


void mitk::TbssImageMapper::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  node->AddProperty( "DisplayChannel", mitk::IntProperty::New( 0 ), renderer, overwrite );
  Superclass::SetDefaultProperties(node, renderer, overwrite);
}

#endif
