/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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
#include <mitkLevelWindow.h>
#include "mitkPACSPlugin.h"

class QcLightbox;

namespace mitk {

class Image;

  /**
  @brief Writes mitk::Images into a Chili lightbox.
  @ingroup IO
  @ingroup Chili

  this is the default-implementation, for the real implementation look at mitkLightBoxResultImageWriterImpl
  */

class LightBoxResultImageWriter : public BaseProcess
{
  public:
    /** Standard class typedefs. */
    mitkClassMacro(LightBoxResultImageWriter, BaseProcess);

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Set the input as MITK::Image */
    virtual void SetInput( const Image* image );

    /**
    Set the input as MITK::DataTreeNode 
    Set the image, levelwindow and description automatically. It doesnt set the propertylist.
    */
    virtual void SetInputByDataTreeNode( const DataTreeNode* node );

    /** Set the SeriesDescription for the new Series */
    virtual void SetSeriesDescription( const std::string& );

    /** Set the LevelWindow for the right view in chili  */
    virtual void SetLevelWindow( LevelWindow levelwindow );

    /**
    Set the propertylist, which get saved to chili in the pic-header.
    Only tags which start with "Chili: " get read and save from the propertylist.
    Warning:
    It is possible that the information always exist in the pic-header.
    If you want to change them, then set the input.
    If you dont want to change them, then dont set the input.
    The pic-header get override by the propertylist. But only this one, which are twice.
    Look at LightBoxImageReaderImpl::GetTagList() to know, which tags get override.
    These tags are not important to save the series, but maybe for the patient!
    "Save" works without setting the TagList. Use it for new Images without any information.
    */
    virtual void SetPropertyList( const mitk::PropertyList::Pointer );

    /** Set the lightbox to write into */
    virtual void SetLightBox(QcLightbox* lightbox);

    /** Set the lightbox to write into to the current lightbox */
    virtual void SetLightBoxToCurrentLightBox();

    /** Set the lightbox to write into to a new lightbox */
    virtual bool SetLightBoxToNewLightBox();

    /** Save the input to a lightbox */
    virtual void Write();

  protected:

    LightBoxResultImageWriter();
    virtual ~LightBoxResultImageWriter();

};

} // namespace mitk

#endif /* LIGHTBOXRESULTIMAGEWRITER_H_HEADER_INCLUDED_C1F48A22 */

