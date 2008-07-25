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

#ifndef mitkImageNumberFilterHIncluded
#define mitkImageNumberFilterHIncluded

#include <mitkPicDescriptorToNode.h>
#include <vector>

namespace mitk {

/**
 * \brief Basic CHILI images import class.
 *
 *
 * WARNING:
 * This class arranged as helper-class. Dont use this class, use mitk::ChiliPlugin.
 * If you use them, be carefull with the parameter.
 * This filter need the CHILI-Version 3.10.
 */
class ImageNumberFilter : public PicDescriptorToNode
{
  public:

   mitkClassMacro( ImageNumberFilter, PicDescriptorToNode );
   itkNewMacro( ImageNumberFilter );

    /*!
    \brief Create multiple nodes (images).
    Calls other methods of this class in turn:

    <li>  SortPicsToGroup
    <li>  SortSlicesByImageNumber
    <li>  SeperateBySpacing
    <li>  SeperateByTime
    <li>  GenerateImages
      
    */
    virtual void Update();

  protected:

    ImageNumberFilter();  // purposely hidden, created by New()
    virtual ~ImageNumberFilter();

    /** Struct for single slices. */
    struct Slice
    {
      mitkIpPicDescriptor* pic;
      Vector3D origin;
      Vector3D normal;
      int imageNumber;
    };

    /** Struct for a single group. */
    struct Group
    {
      std::vector< Slice > includedSlices;
      std::string referenceUID;
      std::string seriesDescription;
      int dimension;  // dimension of the images grouped in this group
      Vector3D pixelSize;
      Vector3D normalWithImageSize;
   };

    /** all groups */
    std::vector< Group > m_GroupList;

    /**
     * \brief Group slices (mitkIpPicDescriptors) of equal size and orientation.
     *
     * For 2D slices: compare orientation (normal) and extent in x and y, group similar slices
     * For 3D volumes: compare origin, orientation and z extent, group similar volumes
     * For 4D images: sort into separate groups (no 2 4D images are grouped)
     */
    void SortPicsToGroup();
    /** This function sort the slices by imagenumber and location. */
  
    /** 
     * \brief Sort all slices in each group by image number.
     * 
     * Sort by image number if possible, 
     * else sort by distance of image origin from world origin.
     * 
     * Uses ImageNumberFilter::NumberSort for comparison of two slices.
     */
    void SortSlicesByImageNumber();

    /**
     * \brief Split groups into sub-groups with uniform slice distances.
     */
    void SeperateBySpacing();

    /**
     * Ensures that for each position in space we have an 
     * equal number of slices that share this position. The
     * number of "allowed" slices per position is set to the
     * minimum number of slices seen for any timestep.
     *
     * The loop inside this method will go through all slices and sort 
     * everything beyond minTimeSteps into a new group (waste?).
     */
    void SeperateByTime();

    /** This function generate all needed parameter to create the mitk::Images. */
    void GenerateImages();

    /** help-functions */
    static bool PositionSort( const Slice elem1, const Slice elem2 );
    
    /* \brief Comparison function for std::sort.
     *
     * Sort by image number if possible, 
     * else sort by distance of image origin from world origin.
     */
    static bool NumberSort( const Slice elem1, const Slice elem2 );
};

} // namespace mitk

#endif

