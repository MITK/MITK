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


#include <vtkPropAssembly.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkCellArray.h>
#include <vtkColorTransferFunction.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyData.h>
#include <vtkActor.h>

#include <mitkProperties.h>
#include <mitkExceptionMacro.h>
#include <mitkException.h>

#include "mitkMAPRegistrationWrapper.h"
#include "mitkRegistrationWrapperMapperBase.h"
#include "mitkRegVisColorStyleProperty.h"
#include "mitkRegVisHelper.h"
#include "mitkRegVisPropertyTags.h"

mitk::MITKRegistrationWrapperMapperBase::MITKRegistrationWrapperMapperBase()
{

}


mitk::MITKRegistrationWrapperMapperBase::~MITKRegistrationWrapperMapperBase()
{

}

void mitk::MITKRegistrationWrapperMapperBase::GenerateDataForRenderer( mitk::BaseRenderer *renderer )
{
    mitk::DataNode::Pointer node = this->GetDataNode();

    if (node.IsNull())
      return;

    bool isVisible = true;
    node->GetVisibility(isVisible, renderer);

    if (!isVisible)
        return;

    RegWrapperLocalStorage *localStorage = m_LSH.GetLocalStorage(renderer);

    //check if updates occured in the node or on the display
    bool outdatedRendererGeometry = RendererGeometryIsOutdated(renderer,localStorage->m_LastUpdateTime);

    if ( (localStorage->m_LastUpdateTime < node->GetMTime())
         || (localStorage->m_LastUpdateTime < node->GetPropertyList()->GetMTime()) //was a property modified?
         || (localStorage->m_LastUpdateTime < node->GetPropertyList(renderer)->GetMTime())
         || outdatedRendererGeometry)
    {
        MITK_DEBUG << "UPDATE NEEDED FOR _ " << renderer->GetName();

        bool isGridActive = false;
        node->GetBoolProperty(mitk::nodeProp_RegVisGrid,isGridActive);

        bool isGlyphActive = false;
        node->GetBoolProperty(mitk::nodeProp_RegVisGlyph,isGlyphActive);

        bool isPointsActive = false;
        node->GetBoolProperty(mitk::nodeProp_RegVisPoints,isPointsActive);

        bool showStartGrid = false;
        node->GetBoolProperty(mitk::nodeProp_RegVisGridShowStart,showStartGrid);

        bool isGridActiveOutdated = mitk::PropertyIsOutdated(node,mitk::nodeProp_RegVisGrid,localStorage->m_LastUpdateTime);
        bool isGlyphActiveOutdated = mitk::PropertyIsOutdated(node,mitk::nodeProp_RegVisGlyph,localStorage->m_LastUpdateTime);
        bool isPointsActiveOutdated = mitk::PropertyIsOutdated(node,mitk::nodeProp_RegVisPoints,localStorage->m_LastUpdateTime);
        bool showStartGridOutdated = mitk::PropertyIsOutdated(node,mitk::nodeProp_RegVisGridShowStart,localStorage->m_LastUpdateTime);

        mitk::BaseData::Pointer baseData = node->GetData();

        if (baseData.IsNull())
            return;

        const mitk::MAPRegistrationWrapper* regWrapper = dynamic_cast<const mitk::MAPRegistrationWrapper*>(baseData.GetPointer());
        if (regWrapper == nullptr)
            return;

        //////////////////////////////////////////////////////////////////////////
        //1. Check the FOV and presentation styles
        bool outdatedFOV = mitk::GridIsOutdated(node,localStorage->m_LastUpdateTime);
        if (outdatedFOV ||isGridActiveOutdated || isGlyphActiveOutdated || isPointsActiveOutdated ||  outdatedRendererGeometry)
          {  // we need to generate the grids/presentation again
            const map::core::RegistrationKernelBase<3,3>* regKernel= mitk::GetRelevantRegKernelOfNode(node);

            if(!regKernel)
              {
                mitkThrow() << "No reg kernel for visualization";
              }

            mitk::BaseGeometry::ConstPointer gridDesc;
            unsigned int gridFrequ =5;

            if (!GetGeometryDescription(renderer,gridDesc, gridFrequ))
            {
              return;
            };

            if(isGridActive)
            {
              localStorage->m_DeformedGridData = mitk::Generate3DDeformationGrid(gridDesc, gridFrequ, regKernel);
              localStorage->m_StartGridData = mitk::Generate3DDeformationGrid(gridDesc,gridFrequ);
              localStorage->m_DeformedGridMapper->SetInputData(localStorage->m_DeformedGridData);
              localStorage->m_StartGridMapper->SetInputData(localStorage->m_StartGridData);
            }
            else if (isGlyphActive)
            {
              localStorage->m_DeformedGridData = mitk::Generate3DDeformationGlyph(gridDesc, regKernel);
              localStorage->m_StartGridData = nullptr;
              localStorage->m_DeformedGridMapper->SetInputData(localStorage->m_DeformedGridData);
            }
            else
            {
              mitkThrow() << "No reg kernel visualization style activated.";
            }
          }

        //////////////////////////////////////////////////////////////////////////
        //2.Check if the mappers or actors must be modified
        bool isColorStyleOutdated = mitk::PropertyIsOutdated(node,mitk::nodeProp_RegVisColorStyle,localStorage->m_LastUpdateTime);
        bool isColorUniOutdated = mitk::PropertyIsOutdated(node,mitk::nodeProp_RegVisColorUni,localStorage->m_LastUpdateTime);
        bool isColor1Outdated = mitk::PropertyIsOutdated(node,mitk::nodeProp_RegVisColor1Value,localStorage->m_LastUpdateTime);
        bool isColor2Outdated = mitk::PropertyIsOutdated(node,mitk::nodeProp_RegVisColor2Value,localStorage->m_LastUpdateTime);
        bool isColor3Outdated = mitk::PropertyIsOutdated(node,mitk::nodeProp_RegVisColor3Value,localStorage->m_LastUpdateTime);
        bool isColor4Outdated = mitk::PropertyIsOutdated(node,mitk::nodeProp_RegVisColor4Value,localStorage->m_LastUpdateTime);
        bool isColor2MagOutdated = mitk::PropertyIsOutdated(node,mitk::nodeProp_RegVisColor2Magnitude,localStorage->m_LastUpdateTime);
        bool isColor3MagOutdated = mitk::PropertyIsOutdated(node,mitk::nodeProp_RegVisColor3Magnitude,localStorage->m_LastUpdateTime);
        bool isColor4MagOutdated = mitk::PropertyIsOutdated(node,mitk::nodeProp_RegVisColor4Magnitude,localStorage->m_LastUpdateTime);
        bool isColorInterpolateOutdated = mitk::PropertyIsOutdated(node,mitk::nodeProp_RegVisColorInterpolate,localStorage->m_LastUpdateTime);

        if(isColorStyleOutdated || isColorUniOutdated || isColor1Outdated ||
           isColor2Outdated || isColor2MagOutdated || isColor3Outdated || isColor3MagOutdated ||
           isColor4Outdated || isColor4MagOutdated || isColorInterpolateOutdated)
          {
            localStorage->m_DeformedGridMapper->ScalarVisibilityOn();
            localStorage->m_DeformedGridMapper->SetScalarModeToUsePointData();
            localStorage->m_DeformedGridMapper->SelectColorArray( "VectorMagnitude" );

            mitk::RegVisColorStyleProperty* colorStyleProp = nullptr;
            node->GetProperty(colorStyleProp, mitk::nodeProp_RegVisColorStyle);

            float color1[3] = {0.0,0.0,0.0};
            node->GetColor( color1, nullptr, mitk::nodeProp_RegVisColor1Value );
            float color2[3] = {0.25,0.25,0.25};
            node->GetColor( color2, nullptr, mitk::nodeProp_RegVisColor2Value );
            float color3[3] = {0.5,0.5,0.5};
            node->GetColor( color3, nullptr, mitk::nodeProp_RegVisColor3Value );
            float color4[3] = {1.0,1.0,1.0};
            node->GetColor( color4, nullptr, mitk::nodeProp_RegVisColor4Value );

            double mag2 = 0;
            node->GetPropertyValue(mitk::nodeProp_RegVisColor2Magnitude, mag2);
            double mag3 = 0;
            node->GetPropertyValue(mitk::nodeProp_RegVisColor3Magnitude, mag3);
            double mag4 = 0;
            node->GetPropertyValue(mitk::nodeProp_RegVisColor4Magnitude, mag4);

            bool interpolate = true;
            node->GetBoolProperty(mitk::nodeProp_RegVisColorInterpolate,interpolate);

            //default :color by vector magnitude
            localStorage->m_DeformedGridMapper->SelectColorArray( "VectorMagnitude" );
            localStorage->m_DeformedGridMapper->SetUseLookupTableScalarRange(true);

            localStorage->m_LUT = vtkSmartPointer<vtkColorTransferFunction>::New();

            if (!colorStyleProp || colorStyleProp->GetValueAsId()==0)
            { //uni color mode
                float temprgb[3] = {1.0,1.0,1.0};
                node->GetColor( temprgb, nullptr, mitk::nodeProp_RegVisColorUni );
                localStorage->m_LUT->AddRGBSegment(0.0,temprgb[0],temprgb[1],temprgb[2],1.0,temprgb[0],temprgb[1],temprgb[2]);
                localStorage->m_LUT->Build();
                localStorage->m_DeformedGridMapper->SetLookupTable(localStorage->m_LUT);
            }
            else
            {
              localStorage->m_LUT->AddRGBPoint(0.0,color1[0],color1[1],color1[2]);
              localStorage->m_LUT->AddRGBPoint(mag2,color2[0],color2[1],color2[2]);
              localStorage->m_LUT->AddRGBPoint(mag3,color3[0],color3[1],color3[2]);
              localStorage->m_LUT->AddRGBPoint(mag4,color4[0],color4[1],color4[2]);
              if (!interpolate)
              {
                localStorage->m_LUT->AddRGBPoint(0.99*mag2,color1[0],color1[1],color1[2]);
                localStorage->m_LUT->AddRGBPoint(0.99*mag3,color2[0],color2[1],color2[2]);
                localStorage->m_LUT->AddRGBPoint(0.99*mag4,color3[0],color3[1],color3[2]);
              };
            }

            localStorage->m_LUT->Build();
            localStorage->m_DeformedGridMapper->SetLookupTable(localStorage->m_LUT);
            localStorage->m_DeformedGridMapper->Update();
        }

        bool isGridStartColorOutdated = mitk::PropertyIsOutdated(node,mitk::nodeProp_RegVisGridStartColor,localStorage->m_LastUpdateTime);

        if(isGridStartColorOutdated)
          {
            localStorage->m_StartGridMapper->ScalarVisibilityOn();
            localStorage->m_StartGridMapper->SetScalarModeToUsePointFieldData();

            float temprgb[3];
            if (node->GetColor( temprgb, nullptr, mitk::nodeProp_RegVisGridStartColor ))
              {
                      double trgb[3] = { (double) temprgb[0], (double) temprgb[1], (double) temprgb[2] };
                      localStorage->m_StartGridActor->GetProperty()->SetColor(trgb);
              }
          }

        //////////////////////////////////////////////////////////////////////////
        //3. Check if Assembly must be updated
        if(isGridActiveOutdated||isGlyphActiveOutdated||isPointsActiveOutdated||showStartGridOutdated)
          {
            localStorage->m_RegAssembly = vtkSmartPointer<vtkPropAssembly>::New();

            if (isGridActive)
              {
                localStorage->m_RegAssembly->AddPart(localStorage->m_DeformedGridActor);
                if (showStartGrid)
                  {
                    localStorage->m_RegAssembly->AddPart(localStorage->m_StartGridActor);
                  }
              }
            else if (isGlyphActive)
            {
              localStorage->m_RegAssembly->AddPart(localStorage->m_DeformedGridActor);
            }
          }

        localStorage->m_LastUpdateTime.Modified();

    }
}


void mitk::MITKRegistrationWrapperMapperBase::SetDefaultProperties(mitk::DataNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
    Superclass::SetDefaultProperties(node, renderer, overwrite);
}

vtkProp* mitk::MITKRegistrationWrapperMapperBase::GetVtkProp(mitk::BaseRenderer *renderer)
{
    return m_LSH.GetLocalStorage(renderer)->m_RegAssembly;
}

mitk::MITKRegistrationWrapperMapperBase::RegWrapperLocalStorage::RegWrapperLocalStorage()
{
    m_DeformedGridActor = vtkSmartPointer<vtkActor>::New();
    m_DeformedGridMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_DeformedGridActor->SetMapper(m_DeformedGridMapper);

    m_StartGridActor = vtkSmartPointer<vtkActor>::New();
    m_StartGridMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    m_StartGridActor->SetMapper(m_StartGridMapper);

    m_RegAssembly = vtkSmartPointer<vtkPropAssembly>::New();

    m_LUT = vtkSmartPointer<vtkColorTransferFunction>::New();

    m_DeformedGridData = nullptr;
    m_StartGridData = nullptr;
}

