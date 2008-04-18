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

#ifndef PICDESCRIPTORTONODE_H_HEADER_INCLUDED
#define PICDESCRIPTORTONODE_H_HEADER_INCLUDED

#include <itkObject.h>

#include <mitkDataTreeNode.h>
#include <mitkPropertyList.h>
#include <ipPic/ipPic.h>
#include <list>

namespace mitk {

  /**
  This class creates multiple mitk::DataTreeNodes (mitk::Images) from a list of ipPicDescriptors.
  */

class PicDescriptorToNode : public itk::Object
{
  public:

    /** constuctor and destructor */
   mitkClassMacro( PicDescriptorToNode, itk::Object );
   itkNewMacro( PicDescriptorToNode );
   virtual ~PicDescriptorToNode();

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

    /*!
    \brief Create multiple nodes (images).
    */
    virtual void Update();

    /*!
    \brief Use this to get the generated mitk::DataTreeNodes.
    @returns A vector of mitk::DataTreeNodes.
    */
    virtual std::vector< DataTreeNode::Pointer > GetOutput();

    /*!
    \brief This function return the ImageInstanceUIDs of the used Slices.
    @returns A vector of strings.
    */
    virtual std::vector< std::list< std::string > > GetImageInstanceUIDs();

  protected:

    PicDescriptorToNode();

    bool m_Abort;
    unsigned long m_ObserverTag;

    void Abort(const itk::EventObject&);

    /** the input */
    std::string m_SeriesOID;
    std::list< ipPicDescriptor* > m_PicDescriptorList;

    /** the results */
    std::vector< DataTreeNode::Pointer > m_Output;
    std::vector< std::list< std::string > > m_ImageInstanceUIDs;

    double Round( double number, unsigned int decimalPlaces );

    /** Create a propertyList from the given PicDescriptor. Therefore all pic-tags get readed and added. */
    const mitk::PropertyList::Pointer CreatePropertyListFromPicTags( ipPicDescriptor* );

    void GenerateData( std::list<ipPicDescriptor*> slices, int sliceSteps, int timeSteps, Vector3D spacing, std::string seriesDescription );

    std::string GetImageInstanceUID( ipPicDescriptor* input );
};

} // namespace mitk

#endif
