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

#include <sofa/component/visualmodel/VisualModelImpl.h>
#include <sofa/helper/system/gl.h>
#include <vtkSmartPointer.h>
#include <SimulationExports.h>

class vtkOpenGLTexture;
class vtkRenderer;

namespace mitk
{
  class Simulation_EXPORT VtkModel : public sofa::component::visualmodel::VisualModelImpl
  {
  public:
    SOFA_CLASS(VtkModel, sofa::component::visualmodel::VisualModelImpl);

    void internalDraw(const sofa::core::visual::VisualParams* vparams, bool transparent);
    bool loadTextures();
    void SetVtkRenderer(vtkRenderer* renderer);
    void updateBuffers();

  private:
    static bool IsGlewInitialized;

    VtkModel();
    ~VtkModel();

    VtkModel(MyType&);
    MyType& operator=(const MyType&);

    void CreateIndexBuffer();
    void CreateVertexBuffer();
    void DrawGroup(int group, bool);
    void DrawGroups(bool transparent);
    void DrawNormals();
    void InitIndexBuffer();
    void InitVertexBuffer();
    void UpdateIndexBuffer();
    void UpdateVertexBuffer();

    bool m_BuffersCreated;
    size_t m_LastNumberOfVertices;
    size_t m_LastNumberOfTriangles;
    size_t m_LastNumberOfQuads;
    GLuint m_VertexBuffer;
    GLuint m_IndexBuffer;
    std::map<unsigned int, vtkSmartPointer<vtkOpenGLTexture> > m_Textures;
    vtkRenderer* m_VtkRenderer;
  };
}

#endif
