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

#include "mitkSceneIOTestScenarioProvider.h"

#include "mitkPointSet.h"
#include "mitkImage.h"
#include "mitkImageGenerator.h"
#include "mitkSurface.h"
#include "mitkGeometryData.h"

#include <vtkPolyData.h>
#include <vtkPolygon.h>
#include <vtkCellArray.h>

// --------------- SceneIOTestScenarioProvider::Scenario ---------------

mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::Scenario::BuildDataStorage() const
{
  return (*m_ScenarioProvider.*m_ProviderMethod)(); // calls function
}

mitk::SceneIOTestScenarioProvider::Scenario::Scenario(const std::string& _key,
    const SceneIOTestScenarioProvider* _scenarioProvider,
    SceneIOTestScenarioProvider::BuilderMethodPointer _providerMethod,
    bool _isSerializable,
    const std::string& _referenceArchiveFilename,
    bool _isReferenceLoadable,
    double _comparisonPrecision)
: key(_key)
, serializable(_isSerializable)
, referenceArchiveFilename(_referenceArchiveFilename)
, referenceArchiveLoadable(_isReferenceLoadable)
, comparisonPrecision(_comparisonPrecision)
, m_ScenarioProvider(_scenarioProvider)
, m_ProviderMethod(_providerMethod)
{
}

// --------------- SceneIOTestScenarioProvider ---------------

mitk::SceneIOTestScenarioProvider::ScenarioList mitk::SceneIOTestScenarioProvider::GetAllScenarios() const
{
  return m_Scenarios;
}


void mitk::SceneIOTestScenarioProvider::AddScenario(const std::string& key,
                                                    BuilderMethodPointer creator,
                                                    bool isSerializable,
                                                    const std::string& referenceArchiveFilename,
                                                    bool isReferenceLoadable,
                                                    double comparisonPrecision)
{
  Scenario newScenario(key, this, creator, isSerializable, referenceArchiveFilename, isReferenceLoadable, comparisonPrecision);
  m_Scenarios.push_back(newScenario);
}


mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::EmptyStorage() const
{
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New();
  return storage;
}


mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::OneEmptyNode() const
{
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New();

  mitk::DataNode::Pointer node = DataNode::New();
  storage->Add(node);

  return storage;
}


mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::OneEmptyNamedNode() const
{
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New();

  mitk::DataNode::Pointer node = DataNode::New();
  node->SetName("Urmel");
  storage->Add(node);

  return storage;
}


mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::ManyTopLevelNodes() const
{
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New();

  for ( auto i = 0; i < m_HowMuchIsMany; ++i ) {
    mitk::DataNode::Pointer node = DataNode::New();
    std::stringstream s;
    s << "Node #" << i;
    node->SetName(s.str());
    storage->Add(node);
  }

  return storage;
}


mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::LineOfManyOnlyChildren() const
{
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New();

  mitk::DataNode::Pointer parent;
  for ( auto i = 0; i < m_HowMuchIsMany; ++i ) {
    mitk::DataNode::Pointer node = DataNode::New();
    std::stringstream s;
    s << "Node #" << i;
    node->SetName(s.str());
    storage->Add(node, parent);
    parent = node;
  }

  return storage;
}

#define AddNode(name) \
  storage->Add(name);

#define DefineNode(name) \
  mitk::DataNode::Pointer name = mitk::DataNode::New(); \
  name->SetName( #name );

#define DefNode0(name) \
  DefineNode(name) \
  AddNode(name)

#define DefNode1(source, name) \
  DefineNode(name) \
  storage->Add(name, source);

#define DefNode2(source1, source2, name) \
  DefineNode(name) \
  { mitk::DataStorage::SetOfObjects::Pointer sources = mitk::DataStorage::SetOfObjects::New(); \
  sources->push_back(source1); \
  sources->push_back(source2); \
  storage->Add(name, sources); }

#define DefNode3(source1, source2, source3, name) \
  DefineNode(name) \
  { mitk::DataStorage::SetOfObjects::Pointer sources = mitk::DataStorage::SetOfObjects::New(); \
  sources->push_back(source1); \
  sources->push_back(source2); \
  sources->push_back(source3); \
  storage->Add(name, sources); }



mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::ComplicatedFamilySituation() const
{
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New();

  // constructing a hierarchy with multiple levels
  // and a couple of multiple parent relations.

  // Source image : http://4.bp.blogspot.com/_57lEz2uo9yw/TTGWNhjsWPI/AAAAAAAAA5M/yMauv61ULXc/s1600/web-technology-family-tree-large.jpg

  // The red tree

  // two real ones
  DefNode0(Cpp)
  DefNode0(Basic)

  // descendants
  DefNode1(Cpp, Java)
  DefNode1(Cpp, Perl)
  DefNode1(Cpp, CSharp)

  DefNode1(Java, JSP)

  DefNode1(Perl, PHP)
  DefNode1(Perl, Ruby)
  DefNode1(Perl, Python)

  DefNode1(PHP, OOPHP)
  DefNode1(PHP, PHPLibs)

  DefNode1(Ruby, RubyRails);

  DefNode1(Basic, VBNet)

  // interesting ones
  DefNode2(Basic, VBNet, VBScript)
  DefNode2(CSharp, VBNet, ASPNet)
  DefNode2(ASPNet, VBScript, ClassicASP)

  DefNode1(ClassicASP, SHTML_SSI)
  DefNode1(ASPNet, ATLAS)

  // some green items
  DefNode0(ECMAScript)
  DefNode0(XHTML)
  DefNode1(ECMAScript, JavaScript)
  DefNode1(JavaScript, DHTML)

  // finally
  DefNode3(DHTML, XHTML, RubyRails, Ajax)

  return storage;
}


mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::Image() const
{
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New();

  { // Image of ints
    mitk::Image::Pointer image3Dints = mitk::ImageGenerator::GenerateRandomImage<int>(10, 5, 7, // dim
                                                                                      1, 0.5, 0.5,// spacing
                                                                                      1, // time steps
                                                                                      3000, -1000); // random max / min
    mitk::DataNode::Pointer node = DataNode::New();
    node->SetName("Image-Int");
    node->SetData(image3Dints);
    storage->Add(node);
  }

  { // Image of doubles
    mitk::Image::Pointer image3Ddouble = mitk::ImageGenerator::GenerateRandomImage<double>(5, 10, 8, // dim
                                                                                      1, 0.5, 0.5,// spacing
                                                                                      2, // time steps
                                                                                      3000, -1000); // random max / min
    mitk::DataNode::Pointer node = DataNode::New();
    node->SetName("Image-Double");
    node->SetData(image3Ddouble);
    storage->Add(node);
  }

  return storage;
}

mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::Surface() const
{
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New();

  { // Surface
    vtkSmartPointer<vtkPoints> points1 = vtkSmartPointer<vtkPoints>::New();
    points1->InsertNextPoint(0.0, 0.0, 0.0);
    points1->InsertNextPoint(1.0, 0.0, 0.0);
    points1->InsertNextPoint(0.0, 1.0, 0.0);
    points1->InsertNextPoint(1.0, 1.0, 0.0);

    vtkSmartPointer<vtkPolygon> polygon1 = vtkSmartPointer<vtkPolygon>::New();
    polygon1->GetPointIds()->SetNumberOfIds(4);
    polygon1->GetPointIds()->SetId(0, 0);
    polygon1->GetPointIds()->SetId(1, 1);
    polygon1->GetPointIds()->SetId(2, 2);
    polygon1->GetPointIds()->SetId(3, 3);

    vtkSmartPointer<vtkPolygon> polygon2 = vtkSmartPointer<vtkPolygon>::New();
    polygon2->GetPointIds()->SetNumberOfIds(4);
    polygon2->GetPointIds()->SetId(0, 3);
    polygon2->GetPointIds()->SetId(1, 2);
    polygon2->GetPointIds()->SetId(2, 0);
    polygon2->GetPointIds()->SetId(3, 1);

    //generate polydatas
    vtkSmartPointer<vtkCellArray> polygonArray1 = vtkSmartPointer<vtkCellArray>::New();
    polygonArray1->InsertNextCell(polygon1);

    vtkSmartPointer<vtkPolyData> polydata1 = vtkSmartPointer<vtkPolyData>::New();
    polydata1->SetPoints(points1);
    polydata1->SetPolys(polygonArray1);

    vtkSmartPointer<vtkCellArray> polygonArray2 = vtkSmartPointer<vtkCellArray>::New();
    polygonArray2->InsertNextCell(polygon2);

    vtkSmartPointer<vtkPolyData> polyDataTwo = vtkSmartPointer<vtkPolyData>::New();
    polyDataTwo->SetPoints(points1);
    polyDataTwo->SetPolys(polygonArray2);

    //generate surfaces
    mitk::Surface::Pointer surface = mitk::Surface::New();
    surface->SetVtkPolyData(polydata1);

    mitk::DataNode::Pointer node = DataNode::New();
    node->SetName("Surface");
    node->SetData(surface);
    storage->Add(node);
  }

  return storage;
}

mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::PointSet() const
{
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New();

  { // PointSet
    mitk::PointSet::Pointer ps = mitk::PointSet::New();
    mitk::PointSet::PointType p;
    mitk::FillVector3D(p, 1.0, -2.0, 33.0);
    ps->SetPoint(0, p);
    mitk::FillVector3D(p, 100.0, -200.0, 3300.0);
    ps->SetPoint(1, p);
    mitk::FillVector3D(p, 2.0, -3.0, 22.0);
    ps->SetPoint(2, p, mitk::PTCORNER); // add point spec

    mitk::DataNode::Pointer node = DataNode::New();
    node->SetName("PointSet");
    node->SetData(ps);
    storage->Add(node);
  }

  return storage;
}

mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::GeometryData() const
{
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New();

  { // GeometryData
    mitk::GeometryData::Pointer gdata = mitk::GeometryData::New();

    // define Geometry3D parameters
    mitk::AffineTransform3D::MatrixType matrix;
    matrix[0][0] = 1.1;
    matrix[1][1] = 2.2;
    matrix[2][2] = 3.3;
    mitk::AffineTransform3D::OffsetType offset;
    mitk::FillVector3D(offset, 0.1, 0.2, 0.3);
    bool isImageGeometry(false);
    unsigned int frameOfReferenceID(47);

    mitk::BaseGeometry::BoundsArrayType bounds;
    bounds[0] = std::numeric_limits<mitk::ScalarType>::min();
    bounds[1] = -52.723;
    bounds[2] = -0.002;
    bounds[3] = 918273645.18293746;
    bounds[4] = -0.002;
    bounds[5] = +52.723;

    mitk::Point3D origin;
    mitk::FillVector3D(origin, 5.1, 5.2, 5.3);
    mitk::Vector3D spacing;
    mitk::FillVector3D(spacing, 2.1, 2.2, 2.3);

    // build GeometryData from matrix/offset/etc.
    mitk::AffineTransform3D::Pointer newTransform = mitk::AffineTransform3D::New();
    newTransform->SetMatrix(matrix);
    newTransform->SetOffset(offset);

    mitk::Geometry3D::Pointer newGeometry = mitk::Geometry3D::New();
    newGeometry->SetFrameOfReferenceID(frameOfReferenceID);
    newGeometry->SetImageGeometry(isImageGeometry);

    newGeometry->SetIndexToWorldTransform(newTransform);

    newGeometry->SetBounds(bounds);
    newGeometry->SetOrigin(origin);
    newGeometry->SetSpacing(spacing);

    mitk::GeometryData::Pointer geometryData = mitk::GeometryData::New();
    geometryData->SetGeometry(newGeometry);

    mitk::DataNode::Pointer node = DataNode::New();
    node->SetName("GeometryData");
    node->SetData(geometryData);
    storage->Add(node);
  }

  return storage;
}



