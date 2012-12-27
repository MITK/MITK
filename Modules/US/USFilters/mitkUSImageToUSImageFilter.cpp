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

#include "mitkUSImageToUSImageFilter.h"
#include <string>


mitk::USImageToUSImageFilter::USImageToUSImageFilter() : mitk::ImageToImageFilter()
{

}

mitk::USImageToUSImageFilter::~USImageToUSImageFilter()
{

}

      // ---- OVERRIDDEN INHERITED METHODS ---- //
void   mitk::USImageToUSImageFilter::SetInput (const mitk::USImage * image){
  mitk::ImageToImageFilter::SetInput(image);
}

void   mitk::USImageToUSImageFilter::SetInput (unsigned int index, const mitk::USImage * image){
  mitk::ImageToImageFilter::SetInput(index, image);
}


mitk::USImage::Pointer mitk::USImageToUSImageFilter::GetOutput(unsigned int idx){
  if (this->GetNumberOfOutputs() < 1)
    return NULL;
  return static_cast<USImage*>(this->ProcessObject::GetOutput(idx));
}

mitk::USImage::Pointer mitk::USImageToUSImageFilter::GetInput(unsigned int idx){
  if (this->GetNumberOfInputs() < 1)
    return NULL;
  mitk::USImage::Pointer result = static_cast<USImage*>(this->ProcessObject::GetInput(idx));
  return result;
}


