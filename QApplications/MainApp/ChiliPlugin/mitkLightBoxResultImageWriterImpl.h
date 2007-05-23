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

#ifndef LIGHTBOXRESULTIMAGEWRITERIMPL_H_HEADER_INCLUDED
#define LIGHTBOXRESULTIMAGEWRITERIMPL_H_HEADER_INCLUDED

#include <mitkLightBoxResultImageWriter.h>
#include "mitkBaseProcess.h"
#include "mitkCommon.h"

class QcLightbox;

namespace mitk {

class Image;

class LightBoxResultImageWriterImpl : public LightBoxResultImageWriter
{
public:
  /** Standard class typedefs. */
  mitkClassMacro(LightBoxResultImageWriterImpl, LightBoxResultImageWriter);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  virtual void SetInput(const mitk::Image *image);

  virtual void SetSeriesDescription( const std::string& description );

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

  LightBoxResultImageWriterImpl();

  virtual ~LightBoxResultImageWriterImpl();

  QcLightbox* m_LightBox;

  LevelWindow m_LevelWindow;

  const Image* m_Image;

  std::string m_SeriesDescription;
};

} // namespace mitk

#endif /* LIGHTBOXRESULTIMAGEWRITER_H_HEADER_INCLUDED_C1F48A22 */

