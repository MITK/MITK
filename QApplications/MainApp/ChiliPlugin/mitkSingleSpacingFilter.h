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

  protected:

    /** constructor */
    SingleSpacingFilter();

    /** struct for every single slice/point/picDescriptor */
    struct Slice
    {
      ipPicDescriptor* currentPic;
      int imageNumber;
    };

    /** this struct combine all slices with the same origin */
    struct PositionAtSpace
    {
      Vector3D origin;
      Vector3D normal;
      std::vector< Slice > includedSlices;
    };

    /** the spacing created between the PositionAtSpace */
    struct Spacing
    {
      Vector3D spacing;
      int count;
    };

    /** struct for a single group */
    struct Group
    {
      std::vector< PositionAtSpace > includedPositions;
      std::vector< Spacing > foundSpacings;
      std::string referenceUID;
      std::string seriesDescription;
      int dimension;
      Vector3D pixelSize;
      Vector3D normalWithImageSize;
   };

    /** the group-list */
    std::vector< Group > groupList;

    struct SpacingStruct
    {
      Vector3D spacing;
      int count;
    };

    /** input */
    std::list< ipPicDescriptor* > m_PicDescriptorList;
    std::string m_SeriesOID;

    /** output */
    std::vector< DataTreeNode::Pointer > m_Output;

    /** function to put the input-PicDescriptors to struct Slice, PositionAtSpace and Group */
    void SortSlicesToGroup();
    /** sort the PositionAtSpace for every group by there location */
    void SortGroupsByLocation();
    /** create the spacings, therefore every spacing between two PositionAtSpace get created and counted, the highest one get used */
    void CreateSpacings();
    /** generate the result - nodes */
    void GenerateNodes();

    /** helpfunctions */
    static bool LocationSort( const PositionAtSpace elem1, const PositionAtSpace elem2 );
    static bool SpacingCountSort( const Spacing elem1, const Spacing elem2 );
    static bool ImageNumberSort( const Slice elem1, const Slice elem2 );
    double Round( double number, unsigned int decimalPlaces );
    const mitk::PropertyList::Pointer CreatePropertyListFromPicTags( ipPicDescriptor* );

    /** debug-output */
    void ShowAllGroupsWithSlices();
    void ShowAllFoundSpacings();
};

} // namespace mitk

#endif
