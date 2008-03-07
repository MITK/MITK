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
  This class creates multiple mitk::DataTreeNodes (mitk::Images) from a list of ipPicDescriptors.

  WARNING:
  This class arranged as helper-class. Dont use this class, use mitk::ChiliPlugin.
  If you use them, be carefull with the parameter.
  This class use QcPlugin::pFetchSliceGeometryFromPic(...) which is only available at chiliversion 3.8.
  */

class StreamReader : public PicDescriptorToNode
{
  public:

   mitkClassMacro( StreamReader, PicDescriptorToNode );
   itkNewMacro( StreamReader );
   /** destructor */
   ~StreamReader();

    /*!
    \brief Set a list of ipPicDescriptors and the SeriesOID as Input.
    @param inputPicDescriptorList   These are the different slices, which get combined to volumes.
    @param inputSeriesOID   The SeriesOID added to the Result-DataTreeNode and get used to Save (override, parent-child-relationship).
    Both parameter have to be set.
    How to get the SeriesOID?
    If you load from lightbox use "lightbox->currentSeries()->oid;".
    If you load from chili-database use "mitk::ChiliPlugin::GetSeriesInformation().OID;".
    */
    virtual void SetInput( std::list< ipPicDescriptor* > inputPicDescriptorList, std::string inputSeriesOID );

    virtual void SetSecondInput( interSliceGeometry_t* geometry, std::string seriesDescription );

    /*!
    \brief Create multiple nodes (images).
    This function is subdivided into several steps. For further information look at the protected-functions.
    */
    virtual void Update();

    /*!
    \brief Use this to get the generated mitk::DataTreeNodes.
    @returns A vector of mitk::DataTreeNodes.
    */
    virtual std::vector< DataTreeNode::Pointer > GetOutput();

    virtual std::vector< std::list< std::string > > GetImageInstanceUIDs();

  protected:

    /** constructor */
    StreamReader();

    /** input */
    std::string m_SeriesOID;
    std::string m_SeriesDescription;
    std::list< ipPicDescriptor* > m_PicDescriptorList;
    interSliceGeometry_t* m_Geometry;

    /** output */
    std::vector< DataTreeNode::Pointer > m_Output;
};

} // namespace mitk

#endif
