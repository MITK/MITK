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

#include "mitkProperties.h"
#include "mitkTbssImage.h"


template<class TPixelType>
mitk::TbssImageMapper<TPixelType>::TbssImageMapper()
{
}

template<class TPixelType>
mitk::TbssImageMapper<TPixelType>::~TbssImageMapper()
{
}

template<class TPixelType>
void
mitk::TbssImageMapper<TPixelType>::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
  int displayIndex(0);


  this->GetDataNode()->GetIntProperty( "DisplayChannel", displayIndex, renderer );
  mitk::Image *input = const_cast< mitk::Image* >(
    this->GetInput()
    );
  mitk::TbssImage<TPixelType> *input2 = dynamic_cast< mitk::TbssImage<TPixelType>* >(
    input
    );

  MITK_INFO << "displayindex: " << displayIndex;


  input2->SetDisplayIndexForRendering(displayIndex);
  Superclass::GenerateDataForRenderer(renderer);
}

template<class TPixelType>
void mitk::TbssImageMapper<TPixelType>::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  node->AddProperty( "DisplayChannel", mitk::IntProperty::New( 0 ), renderer, overwrite );
  Superclass::SetDefaultProperties(node, renderer, overwrite);
}

#endif
