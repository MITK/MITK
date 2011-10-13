/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:56:03 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 17179 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


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
