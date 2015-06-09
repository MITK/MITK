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

#ifndef mitkVtkModel_h
#define mitkVtkModel_h

#include <mitkDataNode.h>
#include <mitkPoint.h>
#include <mitkSurface.h>
#include <mitkVector.h>
#include <sofa/component/visualmodel/VisualModelImpl.h>
#include <sofa/helper/system/gl.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <MitkSimulationExports.h>

class vtkOpenGLTexture;
class vtkRenderer;

namespace mitk
{
  class MITKSIMULATION_EXPORT VtkModel : public sofa::component::visualmodel::VisualModelImpl
  {
  public:
    enum Mode
    {
      OpenGL,
      Surface
    };

    SOFA_CLASS(VtkModel, sofa::component::visualmodel::VisualModelImpl);

    void internalDraw(const sofa::core::visual::VisualParams* vparams, bool transparent) override;
    bool loadTextures() override;
    void SetVtkRenderer(vtkRenderer* renderer);
    void updateBuffers() override;

    DataNode::Pointer GetDataNode() const;

    Mode GetMode() const;
    void SetMode(Mode mode);

  private:
    VtkModel();
    ~VtkModel();

    VtkModel(MyType&);
    MyType& operator=(const MyType&);

    void CreateIndexBuffer();
    void CreateVertexBuffer();
    void DrawGroup(int group, bool transparent);
    void DrawOpenGLGroup(int group, bool transparent);
    void DrawSurfaceGroup(int group, bool transparent);
    void DrawGroups(bool transparent);
    void DrawNormals();
    void InitIndexBuffer();
    void InitVertexBuffer();
    void UpdateIndexBuffer();
    void UpdateVertexBuffer();
    void ValidateBoundBuffers();

    bool m_GlewIsInitialized;
    bool m_BuffersWereCreated;
    size_t m_LastNumberOfVertices;
    size_t m_LastNumberOfTriangles;
    size_t m_LastNumberOfQuads;
    GLuint m_VertexBuffer;
    GLuint m_IndexBuffer;
    std::map<unsigned int, vtkSmartPointer<vtkOpenGLTexture> > m_Textures;
    vtkRenderer* m_VtkRenderer;
    Mode m_Mode;
    vtkSmartPointer<vtkPoints> m_Points;
    vtkSmartPointer<vtkCellArray> m_Polys;
    vtkSmartPointer<vtkFloatArray> m_Normals;
    vtkSmartPointer<vtkFloatArray> m_TexCoords;
    vtkSmartPointer<vtkPolyData> m_PolyData;
    Surface::Pointer m_Surface;
    DataNode::Pointer m_DataNode;
  };
}

#endif
