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

#include "mitkVtkModel.h"
#include <mitkLogMacros.h>
#include <sofa/core/visual/VisualParams.h>
#include <vtkImageData.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#include <vtkOpenGLTexture.h>

bool mitk::VtkModel::IsGlewInitialized = false;

mitk::VtkModel::VtkModel()
  : m_BuffersCreated(false),
    m_LastNumberOfVertices(0),
    m_LastNumberOfTriangles(0),
    m_LastNumberOfQuads(0),
    m_VertexBuffer(0),
    m_IndexBuffer(0),
    m_VtkRenderer(NULL)
{
  if (!IsGlewInitialized)
  {
    GLenum error = glewInit();

    if (error != GLEW_OK)
      MITK_ERROR("glewInit") << glewGetErrorString(error);
    else
      IsGlewInitialized = true;
  }
}

mitk::VtkModel::~VtkModel()
{
  /*if (m_BuffersCreated)
  {
    glDeleteBuffers(1, &m_IndexBuffer);
    glDeleteBuffers(1, &m_VertexBuffer);
  }*/
}

void mitk::VtkModel::CreateIndexBuffer()
{
  glGenBuffers(1, &m_IndexBuffer);
  this->InitIndexBuffer();
}

void mitk::VtkModel::CreateVertexBuffer()
{
  glGenBuffers(1, &m_VertexBuffer);
  this->InitVertexBuffer();
}

void mitk::VtkModel::DrawGroup(int group, bool)
{
  using sofa::core::loader::Material;
  using sofa::defaulttype::ResizableExtVector;
  using sofa::defaulttype::Vec4f;

  const VecCoord& vertices = this->getVertices();
  const ResizableExtVector<Deriv>& normals = this->getVnormals();
  const ResizableExtVector<Triangle>& triangles = this->getTriangles();
  const ResizableExtVector<Quad>& quads = this->getQuads();

  FaceGroup faceGroup;

  if (group == -1)
  {
    faceGroup.nbt = triangles.size();
    faceGroup.nbq = quads.size();
  }
  else
  {
    faceGroup = groups.getValue()[group];
  }

  Material material = faceGroup.materialId != -1
    ? materials.getValue()[faceGroup.materialId]
    : this->material.getValue();

  if (material.useTexture && material.activated)
  {
    m_Textures[faceGroup.materialId]->Load(m_VtkRenderer);

    glEnable(GL_TEXTURE_2D);
    glTexCoordPointer(2, GL_FLOAT, 0, reinterpret_cast<const GLvoid*>(vertices.size() * sizeof(VecCoord::value_type) + normals.size() * sizeof(Deriv)));
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  }

  Vec4f ambient = material.useAmbient ? material.ambient : Vec4f();
  Vec4f diffuse = material.useDiffuse ? material.diffuse : Vec4f();
  Vec4f specular = material.useSpecular ? material.specular : Vec4f();
  Vec4f emissive = material.useEmissive ? material.emissive : Vec4f();
  float shininess = material.useShininess ? std::min(material.shininess, 128.0f) : 45.0f;

  if (shininess == 0.0f)
  {
    specular.clear();
    shininess = 1.0f;
  }

  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient.ptr());
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse.ptr());
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular.ptr());
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emissive.ptr());
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

  if (faceGroup.nbt != 0)
    glDrawElements(GL_TRIANGLES, faceGroup.nbt * 3, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid*>(faceGroup.tri0 * sizeof(Triangle)));

  if (faceGroup.nbq != 0)
    glDrawElements(GL_QUADS, faceGroup.nbq * 4, GL_UNSIGNED_INT, reinterpret_cast<const GLvoid*>(triangles.size() * sizeof(Triangle) + faceGroup.quad0 * sizeof(Quad)));

  if (material.useTexture && material.activated)
  {
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);

    m_Textures[faceGroup.materialId]->PostRender(m_VtkRenderer);
  }
}

void mitk::VtkModel::DrawGroups(bool transparent)
{
  using sofa::core::objectmodel::Data;
  using sofa::helper::ReadAccessor;
  using sofa::helper::vector;

  ReadAccessor<Data<vector<FaceGroup> > > groups = this->groups;

  if (groups.empty())
  {
    this->DrawGroup(-1, transparent);
  }
  else
  {
    int numGroups = static_cast<int>(groups.size());

    for (int i = 0; i < numGroups; ++i)
      this->DrawGroup(i, transparent);
  }
}

void mitk::VtkModel::InitIndexBuffer()
{
  using sofa::defaulttype::ResizableExtVector;

  const ResizableExtVector<Triangle>& triangles = this->getTriangles();
  const ResizableExtVector<Quad>& quads = this->getQuads();

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(Triangle) + quads.size() * sizeof(Quad), NULL, GL_DYNAMIC_DRAW);

  this->UpdateIndexBuffer();
}

void mitk::VtkModel::InitVertexBuffer()
{
  using sofa::defaulttype::ResizableExtVector;

  const VecCoord& vertices = this->getVertices();
  const ResizableExtVector<Deriv> normals = this->getVnormals();
  const VecTexCoord& texCoords = this->getVtexcoords();

  GLsizeiptr sizeOfVertices = vertices.size() * sizeof(VecCoord::value_type);
  GLsizeiptr sizeOfNormals = normals.size() * sizeof(Deriv);
  GLsizeiptr sizeOfTexCoords = texCoords.size() * sizeof(VecTexCoord::value_type);

  glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeOfVertices + sizeOfNormals + sizeOfTexCoords, NULL, GL_DYNAMIC_DRAW);

  this->UpdateVertexBuffer();
}

void mitk::VtkModel::internalDraw(const sofa::core::visual::VisualParams* vparams, bool transparent)
{
  using sofa::core::visual::DisplayFlags;
  using sofa::defaulttype::ResizableExtVector;

  if (!IsGlewInitialized)
    return;

  const DisplayFlags& displayFlags = vparams->displayFlags();

  if (!displayFlags.getShowVisualModels())
    return;

  glEnable(GL_LIGHTING);
  glColor3f(1.0f, 1.0f, 1.0f);
  glPolygonMode(GL_FRONT_AND_BACK, displayFlags.getShowWireFrame() ? GL_LINE : GL_FILL);

  const VecCoord& vertices = this->getVertices();
  const ResizableExtVector<Deriv>& normals = this->getVnormals();
  const VecTexCoord& texCoords = this->getVtexcoords();

  glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer);
  glVertexPointer(3, GL_FLOAT, 0, NULL);
  glNormalPointer(GL_FLOAT, 0, reinterpret_cast<const GLvoid*>(vertices.size() * sizeof(VecCoord::value_type)));
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);

  this->DrawGroups(transparent);

  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  if (displayFlags.getShowWireFrame())
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  glDisable(GL_LIGHTING);

  if (displayFlags.getShowNormals())
    this->DrawNormals();
}

void mitk::VtkModel::DrawNormals()
{
  using sofa::defaulttype::ResizableExtVector;

  const VecCoord& vertices = this->getVertices();
  const ResizableExtVector<Deriv>& normals = this->getVnormals();
  size_t numVertices = vertices.size();
  Coord normal;

  glBegin(GL_LINES);

  for (size_t i = 0; i < numVertices; ++i)
  {
    glVertex3fv(vertices[i].ptr());
    normal = vertices[i] + normals[i];
    glVertex3fv(normal.ptr());
  }

  glEnd();
}

bool mitk::VtkModel::loadTextures()
{
  using sofa::helper::system::DataRepository;
  using sofa::helper::vector;
  using sofa::core::loader::Material;

  m_Textures.clear();

  std::vector<unsigned int> materialIndices;

  const vector<Material>& materials = this->materials.getValue();
  unsigned int numMaterials = materials.size();

  for (unsigned int i = 0; i < numMaterials; ++i)
  {
    const Material& material = materials[i];

    if (material.useTexture && material.activated)
      materialIndices.push_back(i);
  }

  bool retValue = true;
  size_t numTextures = materialIndices.size();

  for (size_t i = 0; i < numTextures; ++i)
  {
    std::string filename = materials[materialIndices[i]].textureFilename;

    if (!DataRepository.findFile(filename))
    {
      MITK_ERROR("VtkModel") << "File \"" << filename << "\" not found!";
      retValue = false;
      continue;
    }

    vtkSmartPointer<vtkImageReader2> imageReader = vtkSmartPointer<vtkImageReader2>::Take(vtkImageReader2Factory::CreateImageReader2(filename.c_str()));

    if (imageReader == NULL)
    {
      MITK_ERROR("VtkModel") << "File \"" << filename << "\" has unknown image format!";
      retValue = false;
      continue;
    }

    imageReader->SetFileName(filename.c_str());
    imageReader->UpdateWholeExtent();

    vtkSmartPointer<vtkOpenGLTexture> texture = vtkSmartPointer<vtkOpenGLTexture>::New();
    texture->SetInputConnection(imageReader->GetOutputPort());
    texture->InterpolateOn();

    m_Textures.insert(std::make_pair(materialIndices[i], texture));
  }

  return retValue;
}

void mitk::VtkModel::SetVtkRenderer(vtkRenderer* renderer)
{
  m_VtkRenderer = renderer;
}

void mitk::VtkModel::updateBuffers()
{
  using sofa::defaulttype::ResizableExtVector;

  const VecCoord& vertices = this->getVertices();
  const ResizableExtVector<Triangle>& triangles = this->getTriangles();
  const ResizableExtVector<Quad>& quads = this->getQuads();

  if (!IsGlewInitialized)
    return;

  if (!m_BuffersCreated)
  {
    this->CreateVertexBuffer();
    this->CreateIndexBuffer();

    m_BuffersCreated = true;
  }
  else
  {
    if (m_LastNumberOfVertices != vertices.size())
      this->InitVertexBuffer();
    else
      this->UpdateVertexBuffer();

    if (m_LastNumberOfTriangles != triangles.size() || m_LastNumberOfQuads != quads.size())
      this->InitIndexBuffer();
    else
      this->UpdateIndexBuffer();
  }

  m_LastNumberOfVertices = vertices.size();
  m_LastNumberOfTriangles = triangles.size();
  m_LastNumberOfQuads = quads.size();
}

void mitk::VtkModel::UpdateIndexBuffer()
{
  using sofa::defaulttype::ResizableExtVector;

  const ResizableExtVector<Triangle>& triangles = this->getTriangles();
  const ResizableExtVector<Quad>& quads = this->getQuads();

  GLsizeiptr sizeOfTriangleIndices = triangles.size() * sizeof(Triangle);
  GLsizeiptr sizeOfQuadIndices = quads.size() * sizeof(Quad);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeOfTriangleIndices, triangles.getData());
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sizeOfTriangleIndices, sizeOfQuadIndices, quads.getData());
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void mitk::VtkModel::UpdateVertexBuffer()
{
  using sofa::defaulttype::ResizableExtVector;

  const VecCoord& vertices = this->getVertices();
  const ResizableExtVector<Deriv> normals = this->getVnormals();
  const VecTexCoord& texCoords = this->getVtexcoords();

  GLsizeiptr sizeOfVertices = vertices.size() * sizeof(VecCoord::value_type);
  GLsizeiptr sizeOfNormals = normals.size() * sizeof(Deriv);

  glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeOfVertices, vertices.getData());
  glBufferSubData(GL_ARRAY_BUFFER, sizeOfVertices, sizeOfNormals, normals.getData());

  if (!m_Textures.empty())
  {
    GLsizeiptr sizeOfTexCoords = texCoords.size() * sizeof(VecTexCoord::value_type);
    glBufferSubData(GL_ARRAY_BUFFER, sizeOfVertices + sizeOfNormals, sizeOfTexCoords, texCoords.getData());
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);
}
