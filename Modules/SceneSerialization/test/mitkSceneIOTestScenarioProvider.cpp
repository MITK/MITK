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

#include "mitkGeometryData.h"
#include "mitkImage.h"
#include "mitkImageGenerator.h"
#include "mitkPointSet.h"
#include "mitkProperties.h"
#include "mitkSurface.h"

#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkPolygon.h>

namespace
{
  std::string VeryLongText =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit.Donec a diam lectus.Sed sit amet ipsum mauris.Maecenas "
    "congue ligula ac quam viverra nec consectetur ante hendrerit.Donec et mollis dolor.Praesent et diam eget libero "
    "egestas mattis sit amet vitae augue.Nam tincidunt congue enim, ut porta lorem lacinia consectetur.Donec ut libero "
    "sed arcu vehicula ultricies a non tortor.Lorem ipsum dolor sit amet, consectetur adipiscing elit.Aenean ut "
    "gravida lorem.Ut turpis felis, pulvinar a semper sed, adipiscing id dolor.Pellentesque auctor nisi id magna "
    "consequat sagittis.Curabitur dapibus enim sit amet elit pharetra tincidunt feugiat nisl imperdiet.Ut convallis "
    "libero in urna ultrices accumsan.Donec sed odio eros.Donec viverra mi quis quam pulvinar at malesuada arcu "
    "rhoncus.Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.In rutrum accumsan "
    "ultricies.Mauris vitae nisi at sem facilisis semper ac in est.\n"
    "Vivamus fermentum semper porta.Nunc diam velit, adipiscing ut tristique vitae, sagittis vel odio.Maecenas "
    "convallis ullamcorper ultricies.Curabitur ornare, ligula semper consectetur sagittis, nisi diam iaculis velit, id "
    "fringilla sem nunc vel mi.Nam dictum, odio nec pretium volutpat, arcu ante placerat erat, non tristique elit urna "
    "et turpis.Quisque mi metus, ornare sit amet fermentum et, tincidunt et orci.Fusce eget orci a orci congue "
    "vestibulum.Ut dolor diam, elementum et vestibulum eu, porttitor vel elit.Curabitur venenatis pulvinar tellus "
    "gravida ornare.Sed et erat faucibus nunc euismod ultricies ut id justo.Nullam cursus suscipit nisi, et ultrices "
    "justo sodales nec.Fusce venenatis facilisis lectus ac semper.Aliquam at massa ipsum.Quisque bibendum purus "
    "convallis nulla ultrices ultricies.Nullam aliquam, mi eu aliquam tincidunt, purus velit laoreet tortor, viverra "
    "pretium nisi quam vitae mi.Fusce vel volutpat elit.Nam sagittis nisi dui.\r\n"
    "Suspendisse lectus leo, consectetur in tempor sit amet, placerat quis neque.Etiam luctus porttitor lorem, sed "
    "suscipit est rutrum non.Curabitur lobortis nisl a enim congue semper.Aenean commodo ultrices imperdiet.Vestibulum "
    "ut justo vel sapien venenatis tincidunt.Phasellus eget dolor sit amet ipsum dapibus condimentum vitae quis "
    "lectus.Aliquam ut massa in turpis dapibus convallis.Praesent elit lacus, vestibulum at malesuada et, ornare et "
    "est.Ut augue nunc, sodales ut euismod non, adipiscing vitae orci.Mauris ut placerat justo.Mauris in ultricies "
    "enim.Quisque nec est eleifend nulla ultrices egestas quis ut quam.Donec sollicitudin lectus a mauris pulvinar id "
    "aliquam urna cursus.Cras quis ligula sem, vel elementum mi.Phasellus non ullamcorper urna.\t\n"
    "Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos.In euismod ultrices "
    "facilisis.Vestibulum porta sapien adipiscing augue congue id pretium lectus molestie.Proin quis dictum nisl.Morbi "
    "id quam sapien, sed vestibulum sem.Duis elementum rutrum mauris sed convallis.Proin vestibulum magna mi.Aenean "
    "tristique hendrerit magna, ac facilisis nulla hendrerit ut.Sed non tortor sodales quam auctor elementum.Donec "
    "hendrerit nunc eget elit pharetra pulvinar.Suspendisse id tempus tortor.Aenean luctus, elit commodo laoreet "
    "commodo, justo nisi consequat massa, sed vulputate quam urna quis eros.Donec vel.";
}

// --------------- SceneIOTestScenarioProvider::Scenario ---------------

mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::Scenario::BuildDataStorage() const
{
  return (*m_ScenarioProvider.*m_ProviderMethod)(); // calls function
}

mitk::SceneIOTestScenarioProvider::Scenario::Scenario(const std::string &_key,
                                                      const SceneIOTestScenarioProvider *_scenarioProvider,
                                                      SceneIOTestScenarioProvider::BuilderMethodPointer _providerMethod,
                                                      bool _isSerializable,
                                                      const std::string &_referenceArchiveFilename,
                                                      bool _isReferenceLoadable,
                                                      double _comparisonPrecision)
  : key(_key),
    serializable(_isSerializable),
    referenceArchiveFilename(_referenceArchiveFilename),
    referenceArchiveLoadable(_isReferenceLoadable),
    comparisonPrecision(_comparisonPrecision),
    m_ScenarioProvider(_scenarioProvider),
    m_ProviderMethod(_providerMethod)
{
}

// --------------- SceneIOTestScenarioProvider ---------------

mitk::SceneIOTestScenarioProvider::ScenarioList mitk::SceneIOTestScenarioProvider::GetAllScenarios() const
{
  return m_Scenarios;
}

void mitk::SceneIOTestScenarioProvider::AddScenario(const std::string &key,
                                                    BuilderMethodPointer creator,
                                                    bool isSerializable,
                                                    const std::string &referenceArchiveFilename,
                                                    bool isReferenceLoadable,
                                                    double comparisonPrecision)
{
  Scenario newScenario(
    key, this, creator, isSerializable, referenceArchiveFilename, isReferenceLoadable, comparisonPrecision);
  m_Scenarios.push_back(newScenario);
}

mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::EmptyStorage() const
{
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New().GetPointer();
  return storage;
}

mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::OneEmptyNode() const
{
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New().GetPointer();

  mitk::DataNode::Pointer node = DataNode::New();
  storage->Add(node);

  return storage;
}

mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::OneEmptyNamedNode() const
{
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New().GetPointer();

  mitk::DataNode::Pointer node = DataNode::New();
  node->SetName("Urmel");
  storage->Add(node);

  return storage;
}

mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::ManyTopLevelNodes() const
{
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New().GetPointer();

  for (auto i = 0; i < m_HowMuchIsMany; ++i)
  {
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
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New().GetPointer();

  mitk::DataNode::Pointer parent;
  for (auto i = 0; i < m_HowMuchIsMany; ++i)
  {
    mitk::DataNode::Pointer node = DataNode::New();
    std::stringstream s;
    s << "Node #" << i;
    node->SetName(s.str());
    storage->Add(node, parent);
    parent = node;
  }

  return storage;
}

#define AddNode(name) storage->Add(name);

#define DefineNode(name)                                                                                               \
  mitk::DataNode::Pointer name = mitk::DataNode::New();                                                                \
  name->SetName(#name);

#define DefNode0(name) DefineNode(name) AddNode(name)

#define DefNode1(source, name) DefineNode(name) storage->Add(name, source);

#define DefNode2(source1, source2, name)                                                                               \
  DefineNode(name)                                                                                                     \
  {                                                                                                                    \
    mitk::DataStorage::SetOfObjects::Pointer sources = mitk::DataStorage::SetOfObjects::New();                         \
    sources->push_back(source1);                                                                                       \
    sources->push_back(source2);                                                                                       \
    storage->Add(name, sources);                                                                                       \
  }

#define DefNode3(source1, source2, source3, name)                                                                      \
  DefineNode(name)                                                                                                     \
  {                                                                                                                    \
    mitk::DataStorage::SetOfObjects::Pointer sources = mitk::DataStorage::SetOfObjects::New();                         \
    sources->push_back(source1);                                                                                       \
    sources->push_back(source2);                                                                                       \
    sources->push_back(source3);                                                                                       \
    storage->Add(name, sources);                                                                                       \
  }

mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::ComplicatedFamilySituation() const
{
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New().GetPointer();

  // constructing a hierarchy with multiple levels
  // and a couple of multiple parent relations.

  // Anybody, feel free to make this something
  // meaningful and/or visualize it :-)

  DefNode0(Color) DefNode0(White)

    DefNode1(Color, Green) DefNode1(Color, Election) DefNode1(Color, Red)

      DefNode1(Green, Yellow)

        DefNode1(Election, Looser) DefNode1(Election, FreeBeer) DefNode1(Election, Winner)

          DefNode1(Looser, Tears) DefNode1(Looser, Anger)

            DefNode1(FreeBeer, OpenSource);

  DefNode1(White, Sweet)

    DefNode2(White, Sweet, Sugar) DefNode2(Red, Sweet, Tomatoe) DefNode2(Tomatoe, Sugar, Ketchup)

      DefNode1(Ketchup, BBQSauce) DefNode1(Tomatoe, ATLAS)

        DefNode0(Fish) DefNode0(OperatingSystem) DefNode1(Fish, Bird) DefNode1(Bird, Penguin)

          DefNode3(Penguin, OperatingSystem, OpenSource, Linux)

            return storage;
}

mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::Image() const
{
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New().GetPointer();

  { // Image of ints
    mitk::Image::Pointer image3Dints = mitk::ImageGenerator::GenerateRandomImage<int>(10,
                                                                                      5,
                                                                                      7, // dim
                                                                                      1,
                                                                                      0.5,
                                                                                      0.5, // spacing
                                                                                      1,   // time steps
                                                                                      3000,
                                                                                      -1000); // random max / min
    mitk::DataNode::Pointer node = DataNode::New();
    node->SetName("Image-Int");
    node->SetData(image3Dints);
    storage->Add(node);
  }

  { // Image of doubles
    mitk::Image::Pointer image3Ddouble = mitk::ImageGenerator::GenerateRandomImage<double>(5,
                                                                                           10,
                                                                                           8, // dim
                                                                                           1,
                                                                                           0.5,
                                                                                           0.5, // spacing
                                                                                           2,   // time steps
                                                                                           3000,
                                                                                           -1000); // random max / min
    mitk::DataNode::Pointer node = DataNode::New();
    node->SetName("Image-Double");
    node->SetData(image3Ddouble);
    storage->Add(node);
  }

  return storage;
}

mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::Surface() const
{
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New().GetPointer();

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

    // generate polydatas
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

    // generate surfaces
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
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New().GetPointer();

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
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New().GetPointer();

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

mitk::DataStorage::Pointer mitk::SceneIOTestScenarioProvider::SpecialProperties() const
{
  mitk::DataStorage::Pointer storage = StandaloneDataStorage::New().GetPointer();

  mitk::DataNode::Pointer node = DataNode::New();
  node->SetName("Camion");

  node->SetProperty("Livre", StringProperty::New(VeryLongText)); // defined at the top of this file
  node->SetProperty(VeryLongText.c_str(), StringProperty::New("Shorty"));

  node->GetPropertyList("Chapitre1")->SetProperty("Page 1", StringProperty::New(VeryLongText));
  node->GetPropertyList("Chapitre1")->SetProperty("Page 2", StringProperty::New(VeryLongText));
  node->GetPropertyList("Chapitre 2")->SetProperty("Page", StringProperty::New(VeryLongText));
  node->GetPropertyList("Chapitre 3")->SetProperty("Page", StringProperty::New(VeryLongText));
  node->GetPropertyList(VeryLongText)->SetProperty("Page", StringProperty::New(VeryLongText));

  // not working (NaN etc.)
  // node->SetProperty("NotAFloat", FloatProperty::New( sqrt(-1.0) ) );
  node->SetProperty("sqrt(2)", FloatProperty::New(-sqrt(2.0)));
  node->SetProperty("sqrt(3)", FloatProperty::New(sqrt(3.0)));

  // most values work fine, just min/max double produces serialization precision errors
  node->SetProperty("sqrt(4)", DoubleProperty::New(-sqrt(4.0)));
  node->SetProperty("sqrt(5)", DoubleProperty::New(sqrt(5.0)));
  // node->SetProperty("maxd", DoubleProperty::New( std::numeric_limits<double>::max() ) );
  node->SetProperty("zero", DoubleProperty::New(0.0));
  node->SetProperty("minzero", DoubleProperty::New(-0.0));
  // node->SetProperty("mind", DoubleProperty::New( std::numeric_limits<double>::min() ) );

  storage->Add(node);

  return storage;
}
