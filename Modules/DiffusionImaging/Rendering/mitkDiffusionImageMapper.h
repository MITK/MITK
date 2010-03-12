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


#ifndef DiffusionImageMapper_H_HEADER_INCLUDED
#define DiffusionImageMapper_H_HEADER_INCLUDED

#include "mitkImageMapper2D.h"

namespace mitk {

  //##Documentation
  //## @brief Mapper for raw diffusion weighted images
  //## @ingroup Mapper
  template<class TPixelType>
  class DiffusionImageMapper : public ImageMapper2D
  {
  public:

    mitkClassMacro(DiffusionImageMapper,ImageMapper2D);
    itkNewMacro(Self);

    void GenerateData( mitk::BaseRenderer *renderer );

    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = NULL, bool overwrite = false );
    
  protected:

    DiffusionImageMapper();
    virtual ~DiffusionImageMapper();

  };

} // namespace mitk

#include "mitkDiffusionImageMapper.cpp"


#endif /* COMPOSITEMAPPER_H_HEADER_INCLUDED */

