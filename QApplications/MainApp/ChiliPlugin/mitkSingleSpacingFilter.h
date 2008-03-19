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

  /**
  This class creates multiple mitk::DataTreeNodes (mitk::Images) from a list of ipPicDescriptors.

  WARNING:
  This class arranged as helper-class. Dont use this class, use mitk::ChiliPlugin.
  If you use them, be carefull with the parameter.
  This filter need the CHILI-Version 3.10.
  */

class SingleSpacingFilter : public PicDescriptorToNode
{
  public:

   mitkClassMacro( SingleSpacingFilter, PicDescriptorToNode );
   itkNewMacro( SingleSpacingFilter );
   /** destructor */
   virtual ~SingleSpacingFilter();

    /*!
    \brief Create multiple nodes (images).
    This function is subdivided into several steps. For further information look at the protected-functions.
    */
    virtual void Update();

  protected:

    /** constructor */
    SingleSpacingFilter();

    /** This struct combine all slices with the same origin. */
    struct Position
    {
      Vector3D origin;
      Vector3D normal;
      std::vector< ipPicDescriptor* > includedPics;
    };

    /** Struct for a single group. */
    struct Group
    {
      std::vector< Position > includedPositions;
      std::string referenceUID;
      std::string seriesDescription;
      int dimension;
      Vector3D pixelSize;
      Vector3D normalWithImageSize;
   };

   /** all groups */
   std::vector< Group > m_GroupVector;

    /** This struct is used to calculate the most commonly used spacing. */
    struct Spacing
    {
      Vector3D spacing;
      int count;
    };

    /** This function sort the ipPicDescriptor to groups and positions.  */
    void SortPicsToGroup();
    /** This function sort the positions by location and the ipPicDescriptor by imagenumber. */
    void SortPositionsAndPics();
    /** This function create mitk::Images until all groups and all ipPicDescriptor used. */
    void CreateResults();
    /** This function calculate the most coherent slices, the most commonly used spacing, the minimum timeStep and delete the used positions. */
    void SearchParameter( unsigned int currentGroup );

    /** help-functions */
    static bool PositionSort( const Position& elem1, const Position& elem2 );
    static bool PicSort( ipPicDescriptor* elem1, ipPicDescriptor* elem2 );
};

} // namespace mitk

#endif
