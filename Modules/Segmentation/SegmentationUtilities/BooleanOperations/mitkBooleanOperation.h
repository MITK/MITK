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

#ifndef mitkBooleanOperation_h
#define mitkBooleanOperation_h

#include "SegmentationExports.h"
#include <mitkImage.h>

namespace mitk
{
  class Segmentation_EXPORT BooleanOperation
  {
  public:
    enum Type
    {
      None,
      Difference,
      Intersection,
      Union
    };

    BooleanOperation(Type type, Image::ConstPointer segmentation1, Image::ConstPointer segmentation2, unsigned int time = 0);
    ~BooleanOperation();

    Image::Pointer GetResult() const;

  private:
    BooleanOperation(const BooleanOperation &);
    BooleanOperation & operator=(const BooleanOperation &);

    void ValidateSegmentation(Image::ConstPointer segmentation) const;
    void ValidateSegmentations() const;

    Type m_Type;
    Image::ConstPointer m_Segmentation1;
    Image::ConstPointer m_Segmentation2;
    unsigned int m_Time;
  };
}

#endif
