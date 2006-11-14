/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef LIGHTBOXIMAGEREADER_H_HEADER_INCLUDED_C1F48A22
#define LIGHTBOXIMAGEREADER_H_HEADER_INCLUDED_C1F48A22

#include "mitkImageSource.h"
#include "mitkCommon.h"

class QcLightbox;

namespace mitk {

  //##Documentation
  //## @brief Read images from Chili LightBox
  //## @ingroup Process
  //## @ingroup Chili
  class LightBoxImageReader : public ImageSource 
  {
    public:
      /** Standard class typedefs. */
      mitkClassMacro(LightBoxImageReader, ImageSource);

      /** Method for creation through the object factory. */
      itkNewMacro(Self);

      //##Description 
      //## @brief Set the lightbox to read from
      virtual void SetLightBox(QcLightbox* lightbox);

      //##Description 
      //## @brief Set the lightbox to read from to the current lightbox
      //##
      //## The current lightbox at the time of the method
      //## call is set as lightbox to read from.
      //## \sa SetLightBox
      virtual void SetLightBoxToCurrentLightBox();

      //##Description 
      //## @brief Get the lightbox to read from
      virtual QcLightbox* GetLightBox() const;
      virtual ~LightBoxImageReader() {}
    protected:
      LightBoxImageReader() {}
  };

} // namespace mitk


#endif /* LIGHTBOXIMAGEREADER_H_HEADER_INCLUDED_C1F48A22 */
