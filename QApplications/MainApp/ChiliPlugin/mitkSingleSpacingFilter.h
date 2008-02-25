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

#ifndef SINGLESPACINGFILTER_H_HEADER_INCLUDED
#define SINGLESPACINGFILTER_H_HEADER_INCLUDED

#include <mitkPicDescriptorToNode.h>
#include <vector>

namespace mitk {

class DataTreeNode;

  /**
  This class creates multiple mitk::DataTreeNodes (mitk::Images) from a list of ipPicDescriptors.

  WARNING:
  This class arranged as helper-class. Dont use this class, use mitk::ChiliPlugin.
  If you use them, be carefull with the parameter.
  This class use QcPlugin::pFetchSliceGeometryFromPic(...) which is only available at chiliversion 3.8.
  */

class SingleSpacingFilter : public PicDescriptorToNode
{
  public:

   mitkClassMacro( SingleSpacingFilter, PicDescriptorToNode );
   itkNewMacro( SingleSpacingFilter );
   /** destructor */
   ~SingleSpacingFilter();

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
    SingleSpacingFilter();

    /** input */
    std::string m_SeriesOID;
    std::list< ipPicDescriptor* > m_PicDescriptorList;

    /** this struct combine all slices with the same origin */
    struct Position
    {
      Vector3D origin;
      Vector3D normal;
      std::vector< ipPicDescriptor* > includedPics;
    };

    /** struct for a single group */
    struct Group
    {
      std::vector< Position > includedPositions;
      std::string referenceUID;
      std::string seriesDescription;
      int dimension;
      Vector3D pixelSize;
      Vector3D normalWithImageSize;
   };
   std::vector< Group > m_GroupVector;

    /** the spacing created between the PositionAtSpace */
    struct Spacing
    {
      Vector3D spacing;
      int count;
    };

    /** output */
    std::vector< DataTreeNode::Pointer > m_Output;
    std::vector< std::list< std::string > > m_ImageInstanceUIDs;

    /** function to put the input-PicDescriptors to struct Slice, PositionAtSpace and Group */
    void SortSlicesToGroup();
    /** sort the PositionAtSpace for every group by there location */
    void SortGroupsAndSlices();

    void CreateResults();
    void SearchParameter( unsigned int currentGroup );
    void GenerateNodes( std::vector<Position*> usedPos, Vector3D spacing, unsigned int timeCount, unsigned int currentGroup );

    /** helpfunctions */
    static bool PositionSort( const Position& elem1, const Position& elem2 );
    static bool SliceSort( ipPicDescriptor* elem1, ipPicDescriptor* elem2 );

    double Round( double number, unsigned int decimalPlaces );
    const mitk::PropertyList::Pointer CreatePropertyListFromPicTags( ipPicDescriptor* );
};

} // namespace mitk

#endif
