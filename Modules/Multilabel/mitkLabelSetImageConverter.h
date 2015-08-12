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


#ifndef __mitkLabelSetImageConverter_H_
#define __mitkLabelSetImageConverter_H_

// for export
#include "MitkMultilabelExports.h"

// mitk includes
#include "mitkImage.h"
#include "mitkLabelSetImage.h"


namespace mitk
{

  /**
  * \brief Converts mitkLabelSetImage into a mitk::Image containing an itk::VectorImage
  *
  * Convenience class to avoid the dimension templating hassle.
  */

class MITKMULTILABEL_EXPORT LabelSetImageConverter
{

public:
  /// \brief Converts a mitk::LabelSetImage to an a itk::VectorImage containing mitk::Image, templating is internal
  static mitk::Image::Pointer ConvertLabelSetImageToImage( const mitk::LabelSetImage::ConstPointer input );

  /// \brief Converst a mitk::Image to a mitk::LabelSetImage, templating and differentation between itk::Image and itk::VectorImage is internal
  static mitk::LabelSetImage::Pointer ConvertImageToLabelSetImage( const mitk::Image::Pointer input );

protected:

};

} // namespace mitk

#endif // __mitkLabelSetImageConverter_H_
