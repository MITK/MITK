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


#ifndef TbssGradientImageMapper_H_HEADER_INCLUDED
#define TbssGradientImageMapper_H_HEADER_INCLUDED

#include "mitkImageVtkMapper2D.h"
#include "MitkDiffusionImagingExports.h"

namespace mitk {

  //##Documentation
  //## @brief Mapper for Gradient images
  //## @ingroup Mapper

  class MitkDiffusionImaging_EXPORT TbssGradientImageMapper : public ImageVtkMapper2D
  {
  public:

    mitkClassMacro(TbssGradientImageMapper,ImageVtkMapper2D);
    itkNewMacro(Self);

    void GenerateDataForRenderer( mitk::BaseRenderer *renderer );

    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = NULL, bool overwrite = false );
    
  protected:

    TbssGradientImageMapper();
    virtual ~TbssGradientImageMapper();

  };

} // namespace mitk


#endif /* TbssGradientImageMapper_H_HEADER_INCLUDED */

