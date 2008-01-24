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
#include <mitkPropertyList.h>

namespace mitk {

class DataTreeNode;

  /**
  This class creates multiple mitk::DataTreeNodes (mitk::Images) from a list of ipPicDescriptors.

  WARNING:
  This class arranged as helper-class. Dont use this class, use mitk::ChiliPlugin.
  If you use them, be carefull with the parameter.
  This class use QcPlugin::pFetchSliceGeometryFromPic(...) which is only available at chiliversion 3.8.
  */

class ImageNumberFilter : public PicDescriptorToNode
{
  public:

    /** constuctor and destructor */
   mitkClassMacro( ImageNumberFilter, PicDescriptorToNode );
   itkNewMacro( ImageNumberFilter );
   virtual ~ImageNumberFilter();

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
    Therefore the PicDescriptors seperated by different factors like spacing, time, pixelspacing, ... .
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

    ImageNumberFilter();

    /** This struct illustrate one possible output. */
    struct DifferentOutputs
    {
      /** Criteria to seperate the different outputs, set by CreatePossibleOutputs(). */
      std::string refferenceUID;
      std::string seriesDescription;
      int dimension;
      Vector3D origin;
      Vector3D normale;
      Vector3D pixelSize;
      Vector3D sliceSpacing;
      /** This parameter can be changing while creating the outputs. */
      int numberOfSlices;
      int numberOfTimeSlices;
      /** Is true if different number of timepoints detected, then the volume have to seperate by time. */
      bool differentTimeSlices;
      /** The single slices from the volume. */
      std::list< ipPicDescriptor* > descriptors;
    };

    /** We want to create multiple outputs, so we need a vector of "DifferentOutputs". */
    typedef std::vector<DifferentOutputs> PossibleOutputs;

    /** The list of ipPicDescriptor which get combined to volumes and set by SetInput(...). */
    std::list< ipPicDescriptor* > m_PicDescriptorList;

    /** The seriesOID which get added to the result-Nodes and set by SetInput(...). */
    std::string m_SeriesOID;

    /**
    These variable get used while generating the final output ( GenerateNodes() ). The size changed while generating. This variable get used in CreateNodesFromOutputs() to generate the mitk::DataTreeNodes.
    */
    PossibleOutputs m_PossibleOutputs;

    /** Thats the real Outputs. */
    std::vector< DataTreeNode::Pointer > m_Output;

    std::vector< std::list< std::string > > m_ImageInstanceUIDs;

    struct SpacingStruct
    {
      Vector3D spacing;
      int count;
    };

    /**
    This function seperate the PossibleOutputs:
    - all slices without isg get ignored
    - sort by frameOfRefferenceUID --> all slices of one geometry have the same one
    - sort by "NormalenVektor" --> if they are parallel, the slices located one above the other
    - sort by pixelSpacing --> separation of extent
    - sort by SeriesDescription.
    */
    void CreatePossibleOutputs();

    /** Sort the descriptors of every PossibleOutput by ImageNumber. If the ImageNumbers are equal, the SliceLocation used. */
    void SortByImageNumber();

    /** Sort the descriptors of every PossibleOutput by SliceLocation. */
    void SortBySliceLocation();

    /** Seperate the PossibleOutputs by different spacings and set "numberOfSlices", "numberOfTimeSlices" and "differentTimeSlices". */
    void SeperateOutputsBySpacing();

    /** Seperate the PossibleOutputs by different timeslices, a coherently volume get prefer before timesliced-volume. */
    void SeperateOutputsByTime();

    /** This function seperate two-slice-volume. Thats a possible result, but we dont want to have such volumes, three slices are minimum. */
    void SplitDummiVolumes();

    /** This function create the mitk::Image and put them into the mitk::DataTreeNodes. All pic-tags, the SeriesOID, NumberOfSlices and NumberOfTimeSlices get added as mitk::DataTreeNode::Porperty. */
    void CreateNodesFromOutputs();

    /** Create a propertyList from the given PicDescriptor. Therefore all pic-tags get readed and added. */
    const mitk::PropertyList::Pointer CreatePropertyListFromPicTags( ipPicDescriptor* );

    /** A helpfunction to round. */
    double Round( double number, unsigned int decimalPlaces );

    /** A helpfunction for debugging, show all Attributes from the PossibleOutputs and the ImageNumber from the descriptors. */
    void DebugOutput();
};

} // namespace mitk

#endif
