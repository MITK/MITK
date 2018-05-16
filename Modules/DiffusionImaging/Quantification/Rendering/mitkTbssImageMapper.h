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


#ifndef TbssImageMapper_H_HEADER_INCLUDED
#define TbssImageMapper_H_HEADER_INCLUDED

#include "mitkImageVtkMapper2D.h"
#include "MitkQuantificationExports.h"

namespace mitk {

  //##Documentation
  //## @brief Mapper for raw diffusion weighted images
  //## @ingroup Mapper

  class MITKQUANTIFICATION_EXPORT TbssImageMapper : public ImageVtkMapper2D
  {
  public:

    mitkClassMacro(TbssImageMapper,ImageVtkMapper2D);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    void GenerateDataForRenderer( mitk::BaseRenderer *renderer ) override;

    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = nullptr, bool overwrite = false );

  protected:

    TbssImageMapper();
    ~TbssImageMapper() override;

  };

} // namespace mitk

//#include "mitkTbssImageMapper.cpp"


#endif /* TbssImageMapper_H_HEADER_INCLUDED */

