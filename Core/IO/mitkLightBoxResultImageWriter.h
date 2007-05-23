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

#ifndef LIGHTBOXRESULTIMAGEWRITER_H_HEADER_INCLUDED_C1F48A22
#define LIGHTBOXRESULTIMAGEWRITER_H_HEADER_INCLUDED_C1F48A22

#include "mitkBaseProcess.h"
#include "mitkCommon.h"
#include <mitkImage.h>

class QcLightbox;

namespace mitk {

class Image;

/**
 @brief Writes mitk::Images into a Chili lightbox.

 @ingroup IO
 @ingroup Chili
*/
class LightBoxResultImageWriter : public BaseProcess
{
public:
  /** Standard class typedefs. */
  mitkClassMacro(LightBoxResultImageWriter, BaseProcess);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  virtual void SetInput(const mitk::Image *image);

  virtual void SetSeriesDescription( const std::string& );

  //##Documentation
  //## @brief Set the level-window, which will be written in the
  //## lightbox
  virtual void SetLevelWindow(LevelWindow levelwindow);

  //##Description 
  //## @brief Set the lightbox to write into
  virtual void SetLightBox(QcLightbox* lightbox);

  //##Description 
  //## @brief Set the lightbox to write into to the current lightbox
  //##
  //## The current lightbox at the time of the method
  //## call is set as lightbox to write into.
  //## \sa SetLightBox
  virtual void SetLightBoxToCurrentLightBox();

  //##Description 
  //## @brief Set the lightbox to write into to a new lightbox
  //##
  //## A new lightbox is set as lightbox to write into.
  //## \sa SetLightBox
  virtual bool SetLightBoxToNewLightBox();

  virtual void Write();

protected:

  LightBoxResultImageWriter();
  virtual ~LightBoxResultImageWriter();

};

} // namespace mitk

#endif /* LIGHTBOXRESULTIMAGEWRITER_H_HEADER_INCLUDED_C1F48A22 */

