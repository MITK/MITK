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


#ifndef ConnectomicsNetworkMapper3D_H_HEADER_INCLUDED
#define ConnectomicsNetworkMapper3D_H_HEADER_INCLUDED

// VTK includes

#include <vtkSmartPointer.h>
#include "vtkPropAssembly.h"

// MITK includes
// base class
#include "mitkVtkMapper.h"

// data type

#include "mitkConnectomicsNetwork.h"

#include <vtkActor.h>

#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>
#include <vtkLineSource.h>
#include <vtkProperty.h>

#include <vtkTubeFilter.h>
#include <vtkPolyDataMapper.h>

#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>

#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkInteractorStyleTrackball.h>

#include <vtkRegularPolygonSource.h>
#include "mitkTextOverlay2D.h"
#include "mitkTextOverlay3D.h"
#include <vtkPolyDataMapper2D.h>
#include <vtkActor2D.h>
#include <vtkProperty2D.h>
#include <mitkOverlayManager.h>
#include "mitkFreeSurferParcellationTranslator.h"
#include "mitkVtkInteractorStyle.h"

#include <vector>
#include <iostream>
#include <string>

#include <MitkConnectomicsExports.h>

namespace mitk {

  /**
    * \brief Mapper for Networks
    * \ingroup Mapper
    */

  class MITKCONNECTOMICS_EXPORT ConnectomicsNetworkMapper3D : public VtkMapper
  {
  public:

    mitkClassMacro(ConnectomicsNetworkMapper3D, VtkMapper);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual vtkProp *GetVtkProp(mitk::BaseRenderer *renderer) override; //looks like deprecated.. should be replaced bz GetViewProp()
    static void SetDefaultProperties(DataNode* node, BaseRenderer* renderer = nullptr, bool overwrite = false );

    static void SetVtkMapperImmediateModeRendering(vtkMapper* mapper);

    virtual void GenerateDataForRenderer(mitk::BaseRenderer* renderer) override;

    virtual const mitk::ConnectomicsNetwork* GetInput();

  protected:

    ConnectomicsNetworkMapper3D();
    virtual ~ConnectomicsNetworkMapper3D();

    void UpdateVtkObjects();

    vtkPropAssembly *m_NetworkAssembly;

    /**
    * \brief Returns true if the properties have changed since the last data generation
    */
    bool PropertiesChanged();

    // Create vectors for customizing color and radius and return maximum
    double FillNodeParameterVector( std::vector< double > * parameterVector, std::string parameterName );
    double FillEdgeParameterVector( std::vector< double > * parameterVector, std::string parameterName );

    void FillNodeFilterBoolVector( std::vector< bool > * boolVector, std::string parameterName );
    void FillEdgeFilterBoolVector( std::vector< bool > * boolVector, std::string parameterName );

    // Property storing members
    std::string m_ChosenRenderingScheme;
    std::string m_ChosenEdgeFilter;
    std::string m_EdgeThresholdParameter;
    double m_EdgeThreshold;
    std::string m_ChosenNodeFilter;
    std::string m_NodeThresholdParameter;
    double m_NodeThreshold;
    mitk::Color m_NodeColorStart;
    mitk::Color m_NodeColorEnd;
    double m_NodeRadiusStart;
    double m_NodeRadiusEnd;
    std::string m_ChosenNodeLabel;
    mitk::Color m_EdgeColorStart;
    mitk::Color m_EdgeColorEnd;
    double m_EdgeRadiusStart;
    double m_EdgeRadiusEnd;
    std::string m_NodeRadiusParameter;
    std::string m_NodeColorParameter;
    std::string m_EdgeRadiusParameter;
    std::string m_EdgeColorParameter;

    // Balloons
    std::string m_BalloonText;
    std::string m_BalloonNodeStats;
    mitk::FreeSurferParcellationTranslator::Pointer m_Translator;
    std::string m_AllNodeLabels;

    mitk::TextOverlay3D::Pointer m_TextOverlay3D;
  };

} // namespace mitk

#endif /* ConnectomicsNetworkMapper3D_H_HEADER_INCLUDED */
