/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef ConnectomicsRenderingProperties_H_HEADER_INCLUDED
#define ConnectomicsRenderingProperties_H_HEADER_INCLUDED

#include <string.h>
#include "mitkProperties.h"
#include "mitkStringProperty.h"
#include "mitkEnumerationProperty.h"
#include "mitkColorProperty.h"

namespace mitk {

  /** \file mitkConnectomicsRenderingProperties.h
   * \brief This file defines the rendering properties available for connectomics networks in MITK.
   *
   * This file collects and explains the properties which can be used to modify the visualisation
   * of connectomics networks.
   */

  // Switching between rendering modes
  /**
   * \brief Define the rendering scheme to be used.
   *
   * Currently there are two possible rendering schemes in MITK.
   * <ol>
   *  <li> The VTK Graph Layout
   *  <li> The MITK Connectomics Visualisation - Default
   * </ol>
   *
   * The VTK Graph Layout is faster than the MITK Connectomics Visualisation, but provides less features
   * and settings.
   */
   const std::string connectomicsRenderingSchemePropertyName = "Connectomics.Rendering.Scheme";

   /**
    * \brief Much faster but less features.
    */
   const std::string connectomicsRenderingVTKScheme = "VTK Graph Layout";

   /**
    * \brief Slower but with several visualisation options.
    */
   const std::string connectomicsRenderingMITKScheme = "MITK Connectomics Visualisation";

   // All options below are only for the MITK Connectomics Visualisation scheme

   ////////////////////////////////////////
   // Filtering Options
   ////////////////////////////////////////

   /** \brief Edge filter option
    *
    * This option controls the filtering of edges for visualization purposes. Edges filtered out will not be shown, but still included in calculations.
    *
    * Currently there these options:
    * <ol>
    *  <li> No Edge Filtering - Default
    *  <li> Shortest Path to Node
    *  <li> Thresholding
    * </ol>
    *
    */
    const std::string connectomicsRenderingEdgeFilteringPropertyName = "Connectomics.Rendering.Edges.Filtering";

   /**
    * \brief Do not filter edges
    */
   const std::string connectomicsRenderingEdgeNoFilter = "No Edge Filtering";

   /**
    * \brief Only show edges which are part of a shortest path to the selected node from any other node
    */
   const std::string connectomicsRenderingEdgeShortestPathFilter = "Shortest Path to Node";

   /**
    * \brief Show only edges above a certain parameter threshold
    */
   const std::string connectomicsRenderingEdgeThresholdFilter = "Thresholding";

   /**
    * \brief Parameter to be thresholded
    */
   const std::string connectomicsRenderingEdgeThresholdFilterParameterName = "Connectomics.Rendering.Edges.Filtering.ThresholdParameter";

   /**
    * \brief Threshold
    */
   const std::string connectomicsRenderingEdgeThresholdFilterThresholdName = "Connectomics.Rendering.Edges.Filtering.Threshold";

    /** \brief Node filter option
    *
    * This option controls the filtering of nodes for visualization purposes. Nodes filtered out will not be shown, but still included in calculations.
    *
    * Currently there these options:
    * <ol>
    *  <li> No Node Filtering - Default
    *  <li> Thresholding
    * </ol>
    *
    */
    const std::string connectomicsRenderingNodeFilteringPropertyName = "Connectomics.Rendering.Nodes.Filtering";

   /**
    * \brief Do not filter nodes
    */
   const std::string connectomicsRenderingNodeNoFilter = "No Node Filtering";

   /**
    * \brief Only show nodes above a certain parameter threshold
    */
   const std::string connectomicsRenderingNodeThresholdingFilter = "Thresholding";

      /**
    * \brief Parameter to be thresholded
    */
   const std::string connectomicsRenderingNodeThresholdFilterParameterName = "Connectomics.Rendering.Nodes.Filtering.ThresholdParameter";

   /**
    * \brief Threshold
    */
   const std::string connectomicsRenderingNodeThresholdFilterThresholdName = "Connectomics.Rendering.Nodes.Filtering.Threshold";

   // Default values
   const mitk::StringProperty::Pointer connectomicsRenderingEdgeThresholdFilterParameterDefault =
     mitk::StringProperty::New( "" );
   const  mitk::FloatProperty::Pointer connectomicsRenderingEdgeThresholdFilterThresholdDefault =
     mitk::FloatProperty::New( 1.0 );

   const mitk::StringProperty::Pointer connectomicsRenderingNodeThresholdFilterParameterDefault =
     mitk::StringProperty::New( "" );
   const  mitk::FloatProperty::Pointer connectomicsRenderingNodeThresholdFilterThresholdDefault =
     mitk::FloatProperty::New( 1.0 );

   ////////////////////////////////////////
   // Node Options
   ////////////////////////////////////////

   // Color gradient
    /**
    * \brief Start Color
    *
    * The start color that will be used for gradient creation
    */
   const std::string connectomicsRenderingNodeGradientStartColorName = "Connectomics.Rendering.Nodes.Gradient.StartColor";

    /**
    * \brief End Color
    *
    * The end color that will be used for gradient creation
    */
   const std::string connectomicsRenderingNodeGradientEndColorName = "Connectomics.Rendering.Nodes.Gradient.EndColor";

    /**
    * \brief Color parameter
    *
    * This parameter will be used to select the color of the node.
    */
   const std::string connectomicsRenderingNodeGradientColorParameterName = "Connectomics.Rendering.Nodes.Gradient.Parameter";

   /**
    * \brief The chosen node label
    *
    * This node will be used for any visualisation requiring a specific node
    */
   const std::string connectomicsRenderingNodeChosenNodeName = "Connectomics.Rendering.Nodes.ChosenNode";

  // Radius
    /**
    * \brief Start Radius
    *
    * The start radius that will be used
    */
   const std::string connectomicsRenderingNodeRadiusStartName = "Connectomics.Rendering.Nodes.Radius.Start";

    /**
    * \brief End Radius
    *
    * The end radius that will be used
    */
   const std::string connectomicsRenderingNodeRadiusEndName = "Connectomics.Rendering.Nodes.Radius.End";

    /**
    * \brief Radius parameter
    *
    * This parameter will be used to select the radius of the node.
    */
   const std::string connectomicsRenderingNodeRadiusParameterName = "Connectomics.Rendering.Nodes.Radius.Parameter";

   // Possible parameters
   /**
    * \brief Using the node degree as parameter
    */
   const std::string connectomicsRenderingNodeParameterDegree = "Degree";

   /**
    * \brief Using the node betweenness as parameter
    */
   const std::string connectomicsRenderingNodeParameterBetweenness = "Betweenness centrality";

   /**
    * \brief Using the node clustering coefficient as parameter
    */
   const std::string connectomicsRenderingNodeParameterClustering = "Clustering coefficient";

   /**
    * \brief Color nodes by shortest path length to a chosen node
    */
   const std::string connectomicsRenderingNodeParameterColoringShortestPath = "Shortest Path Steps";

   /**
    * \brief Constant
    *
    * The node property will be constant.
    */
   const std::string connectomicsRenderingNodeParameterConstant = "Constant";

   // Default values
   const mitk::StringProperty::Pointer connectomicsRenderingNodeChosenNodeDefault =
     mitk::StringProperty::New("");

   const mitk::ColorProperty::Pointer connectomicsRenderingNodeGradientStartColorDefault =
     mitk::ColorProperty::New(0.0f, 0.0f, 1.0f);
   const mitk::ColorProperty::Pointer connectomicsRenderingNodeGradientEndColorDefault =
     mitk::ColorProperty::New(0.0f, 1.0f, 0.0f);

   const  mitk::FloatProperty::Pointer connectomicsRenderingNodeRadiusStartDefault =
     mitk::FloatProperty::New( 1.0 );
   const  mitk::FloatProperty::Pointer connectomicsRenderingNodeRadiusEndDefault =
     mitk::FloatProperty::New( 1.0 );

   ////////////////////////////////////////
   // Edge Options
   ////////////////////////////////////////


   // Color gradient
    /**
    * \brief Start Color
    *
    * The start color that will be used for gradient creation
    */
   const std::string connectomicsRenderingEdgeGradientStartColorName = "Connectomics.Rendering.Edges.Gradient.StartColor";

    /**
    * \brief End Color
    *
    * The end color that will be used for gradient creation
    */
   const std::string connectomicsRenderingEdgeGradientEndColorName = "Connectomics.Rendering.Edges.Gradient.EndColor";

    /**
    * \brief Color parameter
    *
    * This parameter will be used to select the color of the edge.
    */
   const std::string connectomicsRenderingEdgeGradientColorParameterName = "Connectomics.Rendering.Edges.Gradient.Parameter";

  // Radius
    /**
    * \brief Start Radius
    *
    * The start radius that will be used
    */
   const std::string connectomicsRenderingEdgeRadiusStartName = "Connectomics.Rendering.Edges.Radius.Start";

    /**
    * \brief End Radius
    *
    * The end radius that will be used
    */
   const std::string connectomicsRenderingEdgeRadiusEndName = "Connectomics.Rendering.Edges.Radius.End";

    /**
    * \brief Radius parameter
    *
    * This parameter will be used to select the radius of the edge.
    */
   const std::string connectomicsRenderingEdgeRadiusParameterName = "Connectomics.Rendering.Edges.Radius.Parameter";

   // Possible parameters
   /**
    * \brief Using the weight as parameter
    */
   const std::string connectomicsRenderingEdgeParameterWeight = "Weight";

   /**
    * \brief Using the edge centrality as parameter
    */
   const std::string connectomicsRenderingEdgeParameterCentrality = "Edge betweenness centrality";

    /**
    * \brief Constant
    *
    * The edge property will be constant.
    */
   const std::string connectomicsRenderingEdgeParameterConstant = "Constant";

   // Default values
   const mitk::ColorProperty::Pointer connectomicsRenderingEdgeGradientStartColorDefault =
     mitk::ColorProperty::New(0.8f, 0.0f, 0.6f);
   const mitk::ColorProperty::Pointer connectomicsRenderingEdgeGradientEndColorDefault =
     mitk::ColorProperty::New(0.0f, 0.8f, 0.2f);

   const  mitk::FloatProperty::Pointer connectomicsRenderingEdgeRadiusStartDefault =
     mitk::FloatProperty::New( 0.1 );
   const  mitk::FloatProperty::Pointer connectomicsRenderingEdgeRadiusEndDefault =
     mitk::FloatProperty::New( 0.4 );

} // namespace mitk


#endif /* ConnectomicsRenderingProperties_H_HEADER_INCLUDED */
