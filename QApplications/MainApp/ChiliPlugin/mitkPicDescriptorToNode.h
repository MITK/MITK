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

#ifndef PICDESCRIPTORTONODE_H_HEADER_INCLUDED
#define PICDESCRIPTORTONODE_H_HEADER_INCLUDED

//TODO nicht mitkChiliPlugin.h includen, doch was benötige ich für Vector3D bzw. std::list; die Properties dann nicht vergessen???
#include "mitkChiliPlugin.h"
#include "ipPic/ipPic.h"

namespace mitk {

class DataTreeNode;

  /**
  This class creates multiple mitk::DataTreeNodes (mitk::Images) from a list of ipPicDescriptors.

  WARNING:
  This class arranged as helper-class and dont check the input.
  Probably you dont should use this class. Use mitk::ChiliPlugin::LoadFromChili(...) or mitk::ChiliPlugin::LoadImageFromLightBox(...). If you do yet, use it carefully.
  This class use mitk::ChiliPlugin::GetSliceGeometryFromPic(...) which is only available at chiliversion 3.8.
  */

class PicDescriptorToNode
{
  public:

    /** constuctor and destructor */
    PicDescriptorToNode();
    ~PicDescriptorToNode();

   /*!
   \brief Set a list of ipPicDescriptors and the SeriesOID as Input.

   @param inputPicDescriptorList   these are the different slices, which get combined to volumes
   @param inputSeriesOID   the SeriesOID added to the Result-DataTreeNode and get used to Save (override, parent-child-relationship)

   if you load from lightbox, you get the SeriesOID with lightbox->currentSeries()->oid;
   if you load from chili-database, use mitk::ChiliPlugin::GetCurrentSeriesOID();
   you have to set both parameter;
   */
    void SetInput( std::list< ipPicDescriptor* > inputPicDescriptorList, std::string inputSeriesOID );

   /*!
   \brief Create multiple nodes (images).

   Therefore the PicDescriptors seperated by different factors like spacing, time, pixelspacing, ... .
   */
    void GenerateNodes();

   /*!
   \brief Use this to get the results.

   @returns a vector of the generated mitk::DataTreeNodes
   */
    std::vector< DataTreeNode::Pointer > GetOutput();

  protected:

    /** this struct illustrate one possible output */
    struct DifferentOutputs
    {
      /** criteria to seperate the different outputs, get set by CreatePossibleOutputs()*/
      std::string refferenceUID;
      std::string seriesDescription;
      Vector3D normale;
      Vector3D pixelSpacing;
      Vector3D sliceSpacing;
      /** this parameter can be changing while creating the outputs */
      int numberOfSlices;
      int numberOfTimeSlices;
      /** is true if different number of timepoints detected, then the volume have to seperate by time */
      bool differentTimeSlices;
      /** the single slices from the volume */
      std::list< ipPicDescriptor* > descriptors;
    };

    /** we want to create multiple outputs, so we need a vector of "DifferentOutputs" */
    typedef std::vector<DifferentOutputs> PossibleOutputs;

    /**
    these variable get used while generating the final output
    they got more ore less while generating
    the final outputs are mitk::DataTreeNodes
    */
    PossibleOutputs m_PossibleOutputs;

    /** the list of ipPicDescriptor which get combined to volumes and set by SetInput(...) */
    std::list< ipPicDescriptor* > m_PicDescriptorList;

    /** the seriesOID which get added to the result-Nodes and set by SetInput(...) */
    std::string m_SeriesOID;

    /** the studyOID which get added to the result-Nodes and set by SetInput(...) */
    std::string m_StudyOID;

    /** thats the real outputs */
    std::vector< DataTreeNode::Pointer > m_Output;

    /**
    this function seperate the PossibleOutputs
    - all slices without isg get ignored
    - sort by frameOfRefferenceUID --> all slices of one geometry have the same one
    - sort by "NormalenVektor" --> if they are parallel, the slices located one above the other
    - sort by pixelSpacing --> separation of extent
    - sort by SeriesDescription
    */
    void CreatePossibleOutputs();

    /** sort the descriptors of every PossibleOutput by ImageNumber */
    void SortByImageNumber();

    /** sort the descriptors of every PossibleOutput by SliceLocation */
    void SortBySliceLocation();

    /** seperate the PossibleOutputs by different spacings and set "numberOfSlices", "numberOfTimeSlices" and "differentTimeSlices" */
    void SeperateOutputsBySpacing();

    /** seperate the PossibleOutputs by different timeslices, a coherently volume get prefer before timeslice */
    void SeperateOutputsByTime();

    /** 2 slices can be a volume too, but we dont want to have such volumes, three slices are minimum */
    void SplitDummiVolumes();

    /** create mitk::DataTreeNodes and there content from the PossibleOutputs */
    void CreateNodesFromOutputs();

    /** Create a propertyList from the given PicDescriptor. Needed to override Series. Therefor the Pic-Tags have to be identically. We save them to the node, so we can be shure they dont get deleted while changing the image-content. */
    //const mitk::PropertyList::Pointer CreatePropertyListFromPicTags( ipPicDescriptor* );

    /** a helpfunction to round */
    double Round( double number, unsigned int decimalPlaces );

    /** a helpfunction for debugging, show all PossibleOutputs and the ImageNumber from the descriptors */
    void DebugOutput();
};

} // namespace mitk

#endif
