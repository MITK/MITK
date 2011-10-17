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


#ifndef TbssGradientImageMapper__cpp
#define TbssGradientImageMapper__cpp

#include "mitkTbssGradientImageMapper.h"

#include "mitkProperties.h"
#include "mitkTbssGradientImage.h"



mitk::TbssGradientImageMapper::TbssGradientImageMapper()
{
}


mitk::TbssGradientImageMapper::~TbssGradientImageMapper()
{
}


void mitk::TbssGradientImageMapper::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
  int displayIndex(0);


  this->GetDataNode()->GetIntProperty( "DisplayChannel", displayIndex, renderer );
  mitk::Image *input = const_cast< mitk::Image* >(
    this->GetInput()
    );
  mitk::TbssGradientImage *input2 = dynamic_cast< mitk::TbssGradientImage* >(
    input
    );

  MITK_INFO << "displayindex: " << displayIndex;


  input2->SetDisplayIndexForRendering(displayIndex);
  Superclass::GenerateDataForRenderer(renderer);
}


void mitk::TbssGradientImageMapper::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  node->AddProperty( "DisplayChannel", mitk::IntProperty::New( 0 ), renderer, overwrite );
  Superclass::SetDefaultProperties(node, renderer, overwrite);
}

#endif
