/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkRegVisHelper_h
#define mitkRegVisHelper_h

//VTK
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>

//MITK
#include <mitkDataNode.h>
#include <mitkGeometry3D.h>

//MatchPoint
#include <mapRegistrationKernelBase.h>

// MITK
#include <MitkMatchPointRegistrationExports.h>


namespace mitk
{

/**
 * \brief Generate the geometry info used to visualize a registration.
 *
 * Extracts the grid geometry and grid frequency from the properties
 * of the data node containing the registration.
 *
 * \pre regNode is a correctly initialized data node of a registration.
 * \param[in] regNode Pointer to the data node of the registration.
 * \param[out] gridDesc Smart pointer to the extracted grid geometry.
 * \param[out] gridFrequ Grid frequency stored in the regNode.
 */
void MITKMATCHPOINTREGISTRATION_EXPORT GetGridGeometryFromNode(const mitk::DataNode* regNode, mitk::Geometry3D::Pointer& gridDesc, unsigned int& gridFrequ);

/**
 * \brief Generate a 3D deformation grid for visualizing a MatchPoint registration.
 *
 * \param[in] gridDesc The geometry defining the field of view.
 * \param[in] gridFrequence The grid sampling frequency.
 * \param[in] regKernel Optional registration kernel to deform the grid; nullptr for an undeformed grid.
 * \return VTK poly data representing the deformation grid.
 */
vtkSmartPointer<vtkPolyData> MITKMATCHPOINTREGISTRATION_EXPORT Generate3DDeformationGrid(const mitk::BaseGeometry* gridDesc, unsigned int gridFrequence, const map::core::RegistrationKernelBase<3,3>* regKernel = nullptr);

/**
 * \brief Generate a 3D glyph representation of a registration kernel.
 *
 * \param[in] gridDesc The geometry defining the field of view.
 * \param[in] regKernel The registration kernel to visualize.
 * \return VTK poly data containing the glyph representation.
 */
vtkSmartPointer<vtkPolyData> MITKMATCHPOINTREGISTRATION_EXPORT Generate3DDeformationGlyph(const mitk::BaseGeometry* gridDesc, const map::core::RegistrationKernelBase<3,3>* regKernel);

/**
 * \brief Check if grid-relevant node properties are outdated.
 *
 * \param[in] regNode The data node to check.
 * \param[in] reference The time stamp to compare against.
 * \return True if any grid-relevant property was modified after the reference time stamp.
 */
bool MITKMATCHPOINTREGISTRATION_EXPORT GridIsOutdated(const mitk::DataNode* regNode, const itk::TimeStamp& reference);

/**
 * \brief Check if a specific property of a node is outdated.
 *
 * If the property does not exist, the return value indicates whether the node itself is outdated.
 *
 * \param[in] regNode The data node to check.
 * \param[in] propName The name of the property to check.
 * \param[in] reference The time stamp to compare against.
 * \return True if the property (or node) was modified after the reference time stamp.
 */
bool MITKMATCHPOINTREGISTRATION_EXPORT PropertyIsOutdated(const mitk::DataNode* regNode, const std::string& propName, const itk::TimeStamp& reference);

/**
 * \brief Get the relevant registration kernel for visualization.
 *
 * The kernel is determined by the direction property of the node.
 *
 * \param[in] regNode The data node containing the registration.
 * \return Pointer to the relevant kernel, or nullptr if the data node is not valid,
 *         contains no registration, or has no direction property.
 */
MITKMATCHPOINTREGISTRATION_EXPORT const map::core::RegistrationKernelBase<3,3>* GetRelevantRegKernelOfNode(const mitk::DataNode* regNode);


}


#endif
