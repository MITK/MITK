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
#include "mitkDataTree.h"

class QcLightbox;

namespace mitk {

class Image;

/**
* @brief Writer for result images into a Chili lightbox. The result image
* must be of the same size and geometry as an @em source image (set of 
* slices), which is already in the Chili database and has to be provided 
* to the writer.
* @ingroup Process
*/
class LightBoxResultImageWriter : public BaseProcess
{
public:
  /** Standard class typedefs. */
  mitkClassMacro(LightBoxResultImageWriter, BaseProcess);

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  virtual void SetInput(const mitk::Image *image);
  const mitk::Image *GetInput(void);

  //##Documentation
  //## @brief Sets the input to node->GetData() and additionally 
  //## m_LevelWindow and m_ImageTypeName from the node properties
  virtual void SetInputByNode(const mitk::DataTreeNode *node);

  const mitk::Image *GetSourceImage(void);
  virtual void SetSourceImage(const mitk::Image *source); 
  bool SetSourceByTreeSearch(mitk::DataTreeIteratorBase* iterator);

  void SetLightBox(QcLightbox* lightbox);
  QcLightbox* GetLightBox() const;

  itkSetStringMacro( ImageTypeName );
  itkGetStringMacro( ImageTypeName );

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
  void SetLevelWindow(const LevelWindow& levelwindow)
  {
    if(m_LevelWindow!=levelwindow)
    {
      m_LevelWindow = levelwindow;
      Modified();
    }
  }

protected:
  virtual void GenerateData();

  LightBoxResultImageWriter();

  ~LightBoxResultImageWriter();

  QcLightbox* m_LightBox;

  LevelWindow m_LevelWindow;

  std::string m_ImageTypeName;
};

} // namespace mitk

#endif /* LIGHTBOXRESULTIMAGEWRITER_H_HEADER_INCLUDED_C1F48A22 */
