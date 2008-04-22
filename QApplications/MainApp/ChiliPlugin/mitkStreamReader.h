/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-05-30 13:17:52 +0200 (Mi, 30 Mai 2007) $
Version:   $Revision: 10537 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef STREAMREADER_H_HEADER_INCLUDED
#define STREAMREADER_H_HEADER_INCLUDED

#include <mitkPicDescriptorToNode.h>
#include <chili/isg.h>

namespace mitk {

  /**
  This class creates multiple mitk::DataTreeNodes (mitk::Images) from a list of mitkIpPicDescriptors.

  WARNING:
  This class arranged as helper-class. Dont use this class, use mitk::ChiliPlugin.
  If you use them, be carefull with the parameter.
  This filter need the CHILI-Version 3.10.
  */

class StreamReader : public PicDescriptorToNode
{
  public:

   mitkClassMacro( StreamReader, PicDescriptorToNode );
   itkNewMacro( StreamReader );
   /** destructor */
   virtual ~StreamReader();

    virtual void SetSecondInput( interSliceGeometry_t* geometry, std::string seriesDescription );

    /*!
    \brief Create multiple nodes (images).
    This function is subdivided into several steps. For further information look at the protected-functions.
    */
    virtual void Update();

  protected:

    /** constructor */
    StreamReader();

    /** input */
    std::string m_SeriesDescription;
    interSliceGeometry_t* m_Geometry;
};

} // namespace mitk

#endif
