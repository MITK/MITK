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


#ifndef DiffusionImageMapper_txx_HEADER_INCLUDED
#define DiffusionImageMapper_txx_HEADER_INCLUDED

#include "mitkProperties.h"
#include "mitkDiffusionImage.h"

template<class TPixelType>
mitk::DiffusionImageMapper<TPixelType>::DiffusionImageMapper()
{

}

template<class TPixelType>
mitk::DiffusionImageMapper<TPixelType>::~DiffusionImageMapper()
{

}

template<class TPixelType>
void
mitk::DiffusionImageMapper<TPixelType>::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
  int displayIndex(0);


  this->GetDataNode()->GetIntProperty( "DisplayChannel", displayIndex, renderer );
  mitk::Image *input = const_cast< mitk::Image* >(
    this->GetInput()
    );
  mitk::DiffusionImage<TPixelType> *input2 = dynamic_cast< mitk::DiffusionImage<TPixelType>* >(
    input
    );

  input2->SetDisplayIndexForRendering(displayIndex);
  Superclass::GenerateDataForRenderer(renderer);
}

template<class TPixelType>
void mitk::DiffusionImageMapper<TPixelType>::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  node->AddProperty( "DisplayChannel", mitk::IntProperty::New( 0 ), renderer, overwrite );
  Superclass::SetDefaultProperties(node, renderer, overwrite);
}

#endif
