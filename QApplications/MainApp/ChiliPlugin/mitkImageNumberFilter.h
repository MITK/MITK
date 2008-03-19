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

#ifndef IMAGENUMBERFILTER_H_HEADER_INCLUDED
#define IMAGENUMBERFILTER_H_HEADER_INCLUDED

#include <mitkPicDescriptorToNode.h>
#include <vector>

namespace mitk {

  /**
  This class creates multiple mitk::DataTreeNodes (mitk::Images) from a list of ipPicDescriptors.

  WARNING:
  This class arranged as helper-class. Dont use this class, use mitk::ChiliPlugin.
  If you use them, be carefull with the parameter.
  This filter need the CHILI-Version 3.10.
  */

class ImageNumberFilter : public PicDescriptorToNode
{
  public:

   mitkClassMacro( ImageNumberFilter, PicDescriptorToNode );
   itkNewMacro( ImageNumberFilter );
   /** destructor */
   virtual ~ImageNumberFilter();

    /*!
    \brief Create multiple nodes (images).
    This function is subdivided into several steps. For further information look at the protected-functions.
    */
    virtual void Update();

  protected:

    /** constructor */
    ImageNumberFilter();

    /** Struct for single slices. */
    struct Slice
    {
      ipPicDescriptor* currentPic;
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
      int dimension;
      Vector3D pixelSize;
      Vector3D normalWithImageSize;
   };

    /** all groups */
    std::vector< Group > m_GroupList;

    /** This struct is used to calculate the most commonly used spacing. */
    struct Spacing
    {
      Vector3D spacing;
      int count;
    };

    /** This function sort the ipPicDescriptor to groups and slices.  */
    void SortPicsToGroup();
    /** This function sort the slices by imagenumber and location. */
    void SortSlicesByImageNumber();
    /** This function seperate ipPicDescriptors with different spacings. */
    void SeperateBySpacing();
    /** This function seperate ipPicDescriptors with different time-count. */
    void SeperateByTime();
    /** This function seperate two ipPicDescriptors. Three or more slices represent a volume. */
    void SplitDummiVolumes();
    /** This function generate all needed parameter to create the mitk::Images. */
    void GenerateImages();

    /** help-functions */
    static bool PositionSort( const Slice elem1, const Slice elem2 );
    static bool NumberSort( const Slice elem1, const Slice elem2 );
};

} // namespace mitk

#endif
