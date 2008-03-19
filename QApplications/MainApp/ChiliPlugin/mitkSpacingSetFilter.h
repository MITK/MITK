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

#ifndef SPACINGSETFILTER_H_HEADER_INCLUDED
#define SPACINGSETFILTER_H_HEADER_INCLUDED

#include <mitkPicDescriptorToNode.h>
#include <vector>
#include <set>

namespace mitk {

  /**
  This class creates multiple mitk::DataTreeNodes (mitk::Images) from a list of ipPicDescriptors.

  WARNING:
  This class arranged as helper-class. Dont use this class, use mitk::ChiliPlugin.
  If you use them, be carefull with the parameter.
  This filter need the CHILI-Version 3.10.
  */

class SpacingSetFilter : public PicDescriptorToNode
{
  public:

   mitkClassMacro( SpacingSetFilter, PicDescriptorToNode );
   itkNewMacro( SpacingSetFilter );
    /** destructor */
   virtual ~SpacingSetFilter();

    /*!
    \brief Create multiple nodes (images).
    This function is subdivided into several steps. For further information look at the protected-functions.
    */
    virtual void Update();

  protected:

    /** constructor */
    SpacingSetFilter();

    /** Struct for single slices. */
    struct Slice
    {
      ipPicDescriptor* currentPic;
      Vector3D origin;
      Vector3D normal;
      int imageNumber;
      std::set< double > sliceUsedWithSpacing;
    };

    /** Struct for a single group. */
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

   /** all groups */
    std::vector< Group > groupList;

    /** This struct is used to calculate the most commonly used spacing. */
    struct Spacing
    {
      Vector3D spacing;
      int count;
    };

    /** internal use */
    std::vector< std::vector < std::set< Slice* > > > m_GroupResultCombinations;
    std::vector< std::set< Slice* > >::iterator m_IterGroupEnd;
    unsigned int m_TotalCombinationCount;
    std::set< Slice* > m_Set;

    /** This function sort the ipPicDescriptor to groups and slices.  */
    void SortPicsToGroup();
    /** This function sort the slices by location and imagenumber. */
    void SortSlicesByLocation();
    /** This function create all possible combinations between the ipPicDescriptors (with the same spacing). */
    void CreatePossibleCombinations();
    /** This function sort the combinations by the number of included elements, the highest one get first. */
    void SortPossibleCombinations();
    /** This function check the combinations if they are timesliced. */
    void CheckForTimeSlicedCombinations();
    /** This function search for the minimum number of combinations to represent all slices. */
    void SearchForMinimumCombination();
    /** This function generate all needed parameter to create the mitk::Images. */
    void GenerateImages();

    /** helpfunctions */
    static bool PositionSort ( const Slice elem1, const Slice elem2 );
    static bool CombinationCountSort( const std::set< Slice* > elem1, const std::set< Slice* > elem2 );

    void CalculateSpacings( std::vector< Slice >::iterator basis, Group* currentGroup );
    void searchFollowingSlices( std::vector< Slice >::iterator basis, double spacing, int imageNumberSpacing, Group* currentGroup );
    bool EqualImageNumbers( std::vector< Slice >::iterator testIter );
    void RekCombinationSearch( std::vector< std::set< Slice* > >::iterator iterBegin, unsigned int remainingCombinations, std::set< Slice* > currentCombination, std::vector< std::set< Slice* > > resultCombinations );

    /** debug-output */
    void ShowAllGroupsWithSlices();
    void ShowAllPossibleCombinations();
    void ShowAllResultCombinations();
    void ShowAllSlicesWithUsedSpacings();
};

} // namespace mitk

#endif
