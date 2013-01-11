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


#ifndef DiffusionImageMapper_H_HEADER_INCLUDED
#define DiffusionImageMapper_H_HEADER_INCLUDED

#include "mitkImageVtkMapper2D.h"

namespace mitk {

  //##Documentation
  //## @brief Mapper for raw diffusion weighted images
  //## @ingroup Mapper
  template<class TPixelType>
  class DiffusionImageMapper : public ImageVtkMapper2D
  {
  public:

    mitkClassMacro(DiffusionImageMapper,ImageVtkMapper2D);
    itkNewMacro(Self);

    void GenerateDataForRenderer( mitk::BaseRenderer *renderer );

    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = NULL, bool overwrite = false );

  protected:

    DiffusionImageMapper();
    virtual ~DiffusionImageMapper();

  };

} // namespace mitk

#include "mitkDiffusionImageMapper.cpp"


#endif /* COMPOSITEMAPPER_H_HEADER_INCLUDED */

