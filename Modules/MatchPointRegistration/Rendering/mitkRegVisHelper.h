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


#ifndef _MITK_REG_VIS_HELPER__H_
#define _MITK_REG_VIS_HELPER__H_

//VTK
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

//MITK
#include <mitkDataNode.h>
#include <mitkGeometry3D.h>

//MatchPoint
#include <mapRegistrationKernelBase.h>

// MITK
#include "MitkMatchPointRegistrationExports.h"


namespace mitk
{

/** Generates the geometry info used to visualized a registration based on the properties
 * of the data node containing the registration.\n
 * @pre regNode is a correctly initialized data node of a registration
 * @param regNode Pointer to the data node of the registration.
 * @param [out] gridDesc Smartpointer to the extracted grid geometry.
 * @param [out] gridFrequ Grid frequency stored in the regNode.
 */
void MITKMATCHPOINTREGISTRATION_EXPORT GetGridGeometryFromNode(const mitk::DataNode* regNode, mitk::Geometry3D::Pointer& gridDesc, unsigned int& gridFrequ);

/**
 * Generates a 3D defomration gird according to a passed Geometry3D info. It is the basis
 * for most of the visualizations of a MatchPoint registration.
 */
vtkSmartPointer<vtkPolyData> MITKMATCHPOINTREGISTRATION_EXPORT Generate3DDeformationGrid(const mitk::BaseGeometry* gridDesc, unsigned int gridFrequence, const map::core::RegistrationKernelBase<3,3>* regKernel = nullptr);

/**
 * Generates a 3D glyph representation of the given regKernel in the FOV defined by gridDesc.
 */
vtkSmartPointer<vtkPolyData> MITKMATCHPOINTREGISTRATION_EXPORT Generate3DDeformationGlyph(const mitk::BaseGeometry* gridDesc, const map::core::RegistrationKernelBase<3,3>* regKernel);

/**
 * Checks if the grid relevant node properties are outdated regarding the passed time stamp
 * reference*/
bool MITKMATCHPOINTREGISTRATION_EXPORT GridIsOutdated(const mitk::DataNode* regNode, const itk::TimeStamp& reference);

/**
 * Checks if the property of the passed node is outdated regarding the passed time stamp
 * reference
 * If the property does not exist the return value indicates if the node is outdated.*/
bool MITKMATCHPOINTREGISTRATION_EXPORT PropertyIsOutdated(const mitk::DataNode* regNode, const std::string& propName, const itk::TimeStamp& reference);

/**
 * Gets the relevant kernel for visualization of a registration node. The kernel is determined
 * by the direction property of the node.
 * @return Pointer to the relevant kernel. Method may return nullptr if data node is not valid, node
 * contains no registration or has no direction property.*/
MITKMATCHPOINTREGISTRATION_EXPORT const map::core::RegistrationKernelBase<3,3>* GetRelevantRegKernelOfNode(const mitk::DataNode* regNode);


}


#endif


