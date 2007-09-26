/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#ifndef PICDESCRIPTORTONODESECOND_H_HEADER_INCLUDED
#define PICDESCRIPTORTONODESECOND_H_HEADER_INCLUDED

#include <mitkPropertyList.h>
#include <mitkDataTreeNode.h>
#include <list>
#include <vector>
#include <set>
#include <ipPic/ipPic.h>
#include "mitkCommon.h"

namespace mitk {

class DataTreeNode;

  /**
  This class creates multiple mitk::DataTreeNodes (mitk::Images) from a list of ipPicDescriptors.

  WARNING:
  This class arranged as helper-class. Dont use this class, use mitk::ChiliPlugin.
  If you use them, be carefull with the parameter.
  This class use QcPlugin::pFetchSliceGeometryFromPic(...) which is only available at chiliversion 3.8.
  */

class PicDescriptorToNodeSecond : public itk::Object
{
  public:

    /** constuctor and destructor */
   mitkClassMacro( PicDescriptorToNodeSecond, itk::Object );
   itkNewMacro( PicDescriptorToNodeSecond );

    /*!
    \brief Set a list of ipPicDescriptors and the SeriesOID as Input.
    @param inputPicDescriptorList   These are the different slices, which get combined to volumes.
    @param inputSeriesOID   The SeriesOID added to the Result-DataTreeNode and get used to Save (override, parent-child-relationship).
    Both parameter have to be set.
    How to get the SeriesOID?
    If you load from lightbox use "lightbox->currentSeries()->oid;".
    If you load from chili-database use "mitk::ChiliPlugin::GetSeriesInformation().OID;".
    */
    void SetInput( std::list< ipPicDescriptor* > inputPicDescriptorList, std::string inputSeriesOID );

    /*!
    \brief Create multiple nodes (images).
    Therefore the PicDescriptors seperated by different factors like spacing, time, pixelspacing, ... .
    This function is subdivided into several steps. For further information look at the protected-functions.
    */
    void Update();

    /*!
    \brief Use this to get the generated mitk::DataTreeNodes.
    @returns A vector of mitk::DataTreeNodes.
    */
    std::vector< DataTreeNode::Pointer > GetOutput();

  protected:

    /** constructor, destructor */
    PicDescriptorToNodeSecond();
    ~PicDescriptorToNodeSecond();

    /** struct for every single slice/point/picDescriptor */
    struct Slice
    {
      ipPicDescriptor* currentPic;
      Vector3D origin;
      Vector3D normal;
      int imageNumber;
      std::set< double > sliceUsedWithSpacing;
    };

    /** struct for a single group */
    struct Group
    {
      std::vector< Slice > includedSlices;
      std::string referenceUID;
      std::string seriesDescription;
      int dimension;
      Vector3D pixelSize;
      Vector3D normalWithImageSize;
      std::vector< std::set< Slice* > > possibleCombinations;
      std::vector< std::vector < std::set< Slice* > > > resultCombinations;
   };

    /** the group-list */
    std::vector< Group > groupList;

    /** internal use */
    std::vector< std::vector < std::set< Slice* > > > groupResultCombinations;
    std::vector< std::set< Slice* > >::iterator iterGroupEnd;
    unsigned int totalCombinationCount;

    /** input */
    std::list< ipPicDescriptor* > m_PicDescriptorList;
    std::string m_SeriesOID;

    /** output */
    std::vector< DataTreeNode::Pointer > m_Output;

    /** function to put the input-PicDescriptors to struct slice and struct group */
    void SortSlicesToGroup();
    /** sort the included slices for every group by there slice-location, the smaler one get first */
    void SortGroupsByLocation();
    /** create all possible combinations of the included picdescriptors by the spacing, the number of slices have to be three or more */
    void CreatePossibleCombinations();
    /** sort the combinations by the number of included elements, the highest one get first */
    void SortPossibleCombinations();
    /** search the minimum number of combinations to represent all slices, therefore multitudes get used */
    void SearchForMinimumCombination();
    void RekCombinationSearch( std::vector< std::set< Slice* > >::iterator iterBegin, std::set< Slice* > currentCombination, std::vector< std::set< Slice* > > resultCombination );
    /** generate the result - nodes */
    void GenerateNodes();

    /** helpfunctions */
    static bool LocationSort ( const Slice elem1, const Slice elem2 );
    static bool CombinationSort( const std::set< Slice* > elem1, const std::set< Slice* > elem2 );
    double Round( double number, unsigned int decimalPlaces );
    const mitk::PropertyList::Pointer CreatePropertyListFromPicTags( ipPicDescriptor* );

    /** debug-output */
    void ShowAllGroupsWithSlices();
    void ShowAllPossibleCombinations();
    void ShowAllResultCombinations();
    void ShowAllSlicesWithUsedSpacings();
};

} // namespace mitk

#endif
