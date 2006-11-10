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

#include "mitkBaseProcess.h"
#include "mitkCommon.h"
#include "mitkDataTree.h"

#include <mitkLightBoxResultImageWriter.h>

class QcLightbox;

namespace mitk {

class Image;

/**
* @brief Writer for result images into a Chili lightbox. The result image
* must be of the same size and geometry as an @em source image (set of 
* slices), which is already in the Chili database and has to be provided 
* to the writer
*
* @ingroup IO
* @ingroup Chili
* 
* itk::BaseProcess inputs: 1. image to store in database
*                          2. sample image, that is already in the database and has the same dimensions as 1.
*/
class LightBoxResultImageWriterImpl : public LightBoxResultImageWriter
{
public:
  /** Standard class typedefs. */
  mitkClassMacro(LightBoxResultImageWriterImpl, LightBoxResultImageWriter);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  virtual void SetInput(const mitk::Image *image);
  const mitk::Image *GetInput(void);

  //##Documentation
  //## @brief Sets the input to node->GetData() and additionally 
  //## m_LevelWindow and m_ImageTypeName from the node properties
  virtual void SetInputByNode(const mitk::DataTreeNode *node);

  virtual const mitk::Image *GetSourceImage(void);
  virtual void SetSourceImage(const mitk::Image *source); 
  virtual bool SetSourceByTreeSearch(mitk::DataTreeIteratorBase* iterator);

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

  //##Description 
  //## @brief Set the lightbox to write into to the lightbox containing the image to be written
  //##
  //## The lightbox that contains the image to be written is set as lightbox to write into.
  //## \sa SetLightBox
  virtual bool SetLightBoxToCorrespondingLightBox();

  //##Description 
  //## @brief Get the lightbox to write into
  QcLightbox* GetLightBox() const;

  //##Documentation
  //## @brief Get the level-window, which will be written in the
  //## lightbox
  const LevelWindow& GetLevelWindow() const
  {
    return m_LevelWindow;
  }

  //##Documentation
  //## @brief Set the level-window, which will be written in the
  //## lightbox
  virtual void SetLevelWindow(const LevelWindow& levelwindow)
  {
    if(m_LevelWindow!=levelwindow)
    {
      m_LevelWindow = levelwindow;
      Modified();
    }
  }

  virtual void Write() const;

protected:
  virtual void GenerateData();

  LightBoxResultImageWriterImpl();

  virtual ~LightBoxResultImageWriterImpl();

  QcLightbox* m_LightBox;


};

} // namespace mitk

#endif /* LIGHTBOXRESULTIMAGEWRITER_H_HEADER_INCLUDED_C1F48A22 */

