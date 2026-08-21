/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkBaseDataTestImplementation.h>
#include <mitkBaseRenderer.h>
#include <mitkDataNode.h>
#include <mitkMapperProviderBase.h>
#include <mitkMapperProviderRegistry.h>
#include <mitkProperties.h>
#include <mitkStringProperty.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <memory>

namespace
{
  class TestData : public mitk::BaseDataTestImplementation
  {
  public:
    mitkClassMacro(TestData, mitk::BaseDataTestImplementation);
    itkFactorylessNewMacro(Self);
  };

  class TestDataSub : public TestData
  {
  public:
    mitkClassMacro(TestDataSub, TestData);
    itkFactorylessNewMacro(Self);
  };

  class TestMapperBase : public mitk::Mapper
  {
  public:
    mitkClassMacro(TestMapperBase, mitk::Mapper);

    void MitkRender(mitk::BaseRenderer *, mitk::VtkPropRenderer::RenderType) override {}
    void ApplyColorAndOpacityProperties(mitk::BaseRenderer *, vtkActor *) override {}
  };

  class TestMapperA : public TestMapperBase
  {
  public:
    mitkClassMacro(TestMapperA, TestMapperBase);
    itkFactorylessNewMacro(Self);

    static void SetDefaultProperties(mitk::DataNode *node,
                                     mitk::BaseRenderer *renderer = nullptr,
                                     bool overwrite = false)
    {
      node->AddProperty("test.marker", mitk::StringProperty::New("A"), renderer, overwrite);
      node->AddProperty("test.appliedA", mitk::BoolProperty::New(true), renderer, overwrite);
    }
  };

  class TestMapperB : public TestMapperBase
  {
  public:
    mitkClassMacro(TestMapperB, TestMapperBase);
    itkFactorylessNewMacro(Self);
  };

  class TestMapperSub : public TestMapperBase
  {
  public:
    mitkClassMacro(TestMapperSub, TestMapperBase);
    itkFactorylessNewMacro(Self);

    static void SetDefaultProperties(mitk::DataNode *node,
                                     mitk::BaseRenderer *renderer = nullptr,
                                     bool overwrite = false)
    {
      node->AddProperty("test.marker", mitk::StringProperty::New("Sub"), renderer, overwrite);
      node->AddProperty("test.appliedSub", mitk::BoolProperty::New(true), renderer, overwrite);
    }
  };

  /** Claims "test.marker" unconditionally instead of deferring to it. */
  class TestMapperOverriding : public TestMapperBase
  {
  public:
    mitkClassMacro(TestMapperOverriding, TestMapperBase);
    itkFactorylessNewMacro(Self);

    static void SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer = nullptr, bool = false)
    {
      node->SetProperty("test.marker", mitk::StringProperty::New("Overriding"), renderer);
    }
  };

  /** A second unconditional claimant, to tell ranking order apart. */
  class TestMapperRanked : public TestMapperBase
  {
  public:
    mitkClassMacro(TestMapperRanked, TestMapperBase);
    itkFactorylessNewMacro(Self);

    static void SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer = nullptr, bool = false)
    {
      node->SetProperty("test.marker", mitk::StringProperty::New("Ranked"), renderer);
    }
  };

  class DecliningProvider : public mitk::MapperProviderBase<TestMapperSub, TestDataSub>
  {
  public:
    using MapperProviderBase::MapperProviderBase;

    mitk::Mapper::Pointer CreateMapper(mitk::DataNode *) const override
    {
      return nullptr;
    }
  };

  /**
   * A single provider object serving both mapper slots through two service
   * registrations, the pattern documented by mitk::IMapperProvider.
   */
  class DualSlotProvider : public mitk::IMapperProvider
  {
  public:
    DualSlotProvider()
    {
      auto *context = us::GetModuleContext();

      m_Registration2D = this->Register(context, mitk::BaseRenderer::Standard2D);
      m_Registration3D = this->Register(context, mitk::BaseRenderer::Standard3D);
    }

    ~DualSlotProvider() override
    {
      if (m_Registration3D.IsAvailable())
        m_Registration3D.Unregister();

      if (m_Registration2D.IsAvailable())
        m_Registration2D.Unregister();
    }

    void UnregisterSlot2D() { m_Registration2D.Unregister(); }

    mitk::Mapper::Pointer CreateMapper(mitk::DataNode *node) const override
    {
      auto mapper = TestMapperA::New();
      mapper->SetDataNode(node);
      return mapper.GetPointer();
    }

    void SetDefaultProperties(mitk::DataNode *) const override {}

  private:
    us::ServiceRegistration<mitk::IMapperProvider> Register(us::ModuleContext *context, MapperSlotId slotId)
    {
      us::ServiceProperties props;
      props[mitk::IMapperProvider::PROP_BASEDATA_TYPE()] = std::string(TestData::GetStaticNameOfClass());
      props[mitk::IMapperProvider::PROP_SLOT_ID()] = static_cast<int>(slotId);

      return context->RegisterService<mitk::IMapperProvider>(this, props);
    }

    us::ServiceRegistration<mitk::IMapperProvider> m_Registration2D;
    us::ServiceRegistration<mitk::IMapperProvider> m_Registration3D;
  };

  mitk::DataNode::Pointer CreateNode(mitk::BaseData *data)
  {
    auto node = mitk::DataNode::New();
    node->SetData(data);
    return node;
  }
}

class mitkMapperProviderRegistryTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkMapperProviderRegistryTestSuite);
  MITK_TEST(CreateMapper_ExactMatch);
  MITK_TEST(CreateMapper_SubclassFallsBackToBaseRegistration);
  MITK_TEST(CreateMapper_SpecificityBeatsRanking);
  MITK_TEST(CreateMapper_RankingBreaksTiesWithinClassLevel);
  MITK_TEST(CreateMapper_EqualRankingFirstRegistrationWins);
  MITK_TEST(CreateMapper_DecliningProviderFallsThrough);
  MITK_TEST(CreateMapper_UnregistrationReelectsRunnerUp);
  MITK_TEST(CreateMapper_UnregisteringOneSlotKeepsTheOther);
  MITK_TEST(CreateMapper_SlotIsolation);
  MITK_TEST(CreateMapper_NullData);
  MITK_TEST(ApplyDefaultProperties_BaseClassAppliedFirst);
  MITK_TEST(ApplyDefaultProperties_MostDerivedOverridesBaseClass);
  MITK_TEST(ApplyDefaultProperties_HigherRankingAppliedLast);
  MITK_TEST(ApplyDefaultProperties_NullData);
  CPPUNIT_TEST_SUITE_END();

public:
  void CreateMapper_ExactMatch()
  {
    mitk::MapperProviderBase<TestMapperA, TestData> provider(mitk::BaseRenderer::Standard2D);

    auto node = CreateNode(TestData::New());
    auto mapper = this->Registry().CreateMapper(node, mitk::BaseRenderer::Standard2D);

    CPPUNIT_ASSERT(mapper.IsNotNull());
    CPPUNIT_ASSERT(dynamic_cast<TestMapperA *>(mapper.GetPointer()) != nullptr);
    CPPUNIT_ASSERT(mapper->GetDataNode() == node.GetPointer());
  }

  void CreateMapper_SubclassFallsBackToBaseRegistration()
  {
    mitk::MapperProviderBase<TestMapperA, TestData> provider(mitk::BaseRenderer::Standard2D);

    auto node = CreateNode(TestDataSub::New());
    auto mapper = this->Registry().CreateMapper(node, mitk::BaseRenderer::Standard2D);

    CPPUNIT_ASSERT(mapper.IsNotNull());
    CPPUNIT_ASSERT(dynamic_cast<TestMapperA *>(mapper.GetPointer()) != nullptr);
  }

  void CreateMapper_SpecificityBeatsRanking()
  {
    mitk::MapperProviderBase<TestMapperA, TestData> baseProvider(mitk::BaseRenderer::Standard2D, 100);
    mitk::MapperProviderBase<TestMapperSub, TestDataSub> subProvider(mitk::BaseRenderer::Standard2D);

    auto node = CreateNode(TestDataSub::New());
    auto mapper = this->Registry().CreateMapper(node, mitk::BaseRenderer::Standard2D);

    CPPUNIT_ASSERT(mapper.IsNotNull());
    CPPUNIT_ASSERT(dynamic_cast<TestMapperSub *>(mapper.GetPointer()) != nullptr);
  }

  void CreateMapper_RankingBreaksTiesWithinClassLevel()
  {
    mitk::MapperProviderBase<TestMapperA, TestData> lowRankedProvider(mitk::BaseRenderer::Standard2D);
    mitk::MapperProviderBase<TestMapperB, TestData> highRankedProvider(mitk::BaseRenderer::Standard2D, 10);

    auto node = CreateNode(TestData::New());
    auto mapper = this->Registry().CreateMapper(node, mitk::BaseRenderer::Standard2D);

    CPPUNIT_ASSERT(mapper.IsNotNull());
    CPPUNIT_ASSERT(dynamic_cast<TestMapperB *>(mapper.GetPointer()) != nullptr);
  }

  void CreateMapper_EqualRankingFirstRegistrationWins()
  {
    mitk::MapperProviderBase<TestMapperA, TestData> firstProvider(mitk::BaseRenderer::Standard2D);
    mitk::MapperProviderBase<TestMapperB, TestData> secondProvider(mitk::BaseRenderer::Standard2D);

    auto node = CreateNode(TestData::New());
    auto mapper = this->Registry().CreateMapper(node, mitk::BaseRenderer::Standard2D);

    CPPUNIT_ASSERT(mapper.IsNotNull());
    CPPUNIT_ASSERT(dynamic_cast<TestMapperA *>(mapper.GetPointer()) != nullptr);
  }

  void CreateMapper_DecliningProviderFallsThrough()
  {
    DecliningProvider decliningProvider(mitk::BaseRenderer::Standard2D);
    mitk::MapperProviderBase<TestMapperA, TestData> baseProvider(mitk::BaseRenderer::Standard2D);

    auto node = CreateNode(TestDataSub::New());
    auto mapper = this->Registry().CreateMapper(node, mitk::BaseRenderer::Standard2D);

    CPPUNIT_ASSERT(mapper.IsNotNull());
    CPPUNIT_ASSERT(dynamic_cast<TestMapperA *>(mapper.GetPointer()) != nullptr);
  }

  void CreateMapper_UnregistrationReelectsRunnerUp()
  {
    auto node = CreateNode(TestData::New());

    auto highRankedProvider =
      std::make_unique<mitk::MapperProviderBase<TestMapperB, TestData>>(mitk::BaseRenderer::Standard2D, 10);
    auto lowRankedProvider =
      std::make_unique<mitk::MapperProviderBase<TestMapperA, TestData>>(mitk::BaseRenderer::Standard2D);

    auto mapper = this->Registry().CreateMapper(node, mitk::BaseRenderer::Standard2D);
    CPPUNIT_ASSERT(dynamic_cast<TestMapperB *>(mapper.GetPointer()) != nullptr);

    highRankedProvider.reset();

    mapper = this->Registry().CreateMapper(node, mitk::BaseRenderer::Standard2D);
    CPPUNIT_ASSERT(dynamic_cast<TestMapperA *>(mapper.GetPointer()) != nullptr);

    lowRankedProvider.reset();

    mapper = this->Registry().CreateMapper(node, mitk::BaseRenderer::Standard2D);
    CPPUNIT_ASSERT(mapper.IsNull());
  }

  void CreateMapper_UnregisteringOneSlotKeepsTheOther()
  {
    DualSlotProvider provider;

    auto node = CreateNode(TestData::New());

    CPPUNIT_ASSERT(this->Registry().CreateMapper(node, mitk::BaseRenderer::Standard2D).IsNotNull());
    CPPUNIT_ASSERT(this->Registry().CreateMapper(node, mitk::BaseRenderer::Standard3D).IsNotNull());

    provider.UnregisterSlot2D();

    CPPUNIT_ASSERT(this->Registry().CreateMapper(node, mitk::BaseRenderer::Standard2D).IsNull());
    CPPUNIT_ASSERT(this->Registry().CreateMapper(node, mitk::BaseRenderer::Standard3D).IsNotNull());
  }

  void CreateMapper_SlotIsolation()
  {
    mitk::MapperProviderBase<TestMapperA, TestData> provider(mitk::BaseRenderer::Standard2D);

    auto node = CreateNode(TestData::New());
    auto mapper = this->Registry().CreateMapper(node, mitk::BaseRenderer::Standard3D);

    CPPUNIT_ASSERT(mapper.IsNull());
  }

  void CreateMapper_NullData()
  {
    mitk::MapperProviderBase<TestMapperA, TestData> provider(mitk::BaseRenderer::Standard2D);

    auto node = mitk::DataNode::New();
    auto mapper = this->Registry().CreateMapper(node, mitk::BaseRenderer::Standard2D);

    CPPUNIT_ASSERT(mapper.IsNull());
    CPPUNIT_ASSERT(this->Registry().CreateMapper(nullptr, mitk::BaseRenderer::Standard2D).IsNull());
  }

  void ApplyDefaultProperties_BaseClassAppliedFirst()
  {
    auto node = CreateNode(TestDataSub::New());

    mitk::MapperProviderBase<TestMapperA, TestData> baseProvider(mitk::BaseRenderer::Standard2D);
    mitk::MapperProviderBase<TestMapperSub, TestDataSub> subProvider(mitk::BaseRenderer::Standard2D);

    this->Registry().ApplyDefaultProperties(node);

    // Both mappers add "test.marker" without overwriting, so the base class
    // value stands and the more derived one defers to it.
    std::string marker;
    CPPUNIT_ASSERT(node->GetStringProperty("test.marker", marker));
    CPPUNIT_ASSERT_EQUAL(std::string("A"), marker);

    bool applied = false;
    CPPUNIT_ASSERT(node->GetBoolProperty("test.appliedA", applied) && applied);
    applied = false;
    CPPUNIT_ASSERT(node->GetBoolProperty("test.appliedSub", applied) && applied);
  }

  void ApplyDefaultProperties_MostDerivedOverridesBaseClass()
  {
    auto node = CreateNode(TestDataSub::New());

    mitk::MapperProviderBase<TestMapperA, TestData> baseProvider(mitk::BaseRenderer::Standard2D);
    mitk::MapperProviderBase<TestMapperOverriding, TestDataSub> subProvider(mitk::BaseRenderer::Standard2D);

    this->Registry().ApplyDefaultProperties(node);

    std::string marker;
    CPPUNIT_ASSERT(node->GetStringProperty("test.marker", marker));
    CPPUNIT_ASSERT_EQUAL(std::string("Overriding"), marker);
  }

  void ApplyDefaultProperties_HigherRankingAppliedLast()
  {
    auto node = CreateNode(TestData::New());

    mitk::MapperProviderBase<TestMapperOverriding, TestData> lowRankedProvider(mitk::BaseRenderer::Standard2D);
    mitk::MapperProviderBase<TestMapperRanked, TestData> highRankedProvider(mitk::BaseRenderer::Standard2D, 10);

    this->Registry().ApplyDefaultProperties(node);

    std::string marker;
    CPPUNIT_ASSERT(node->GetStringProperty("test.marker", marker));
    CPPUNIT_ASSERT_EQUAL(std::string("Ranked"), marker);
  }

  void ApplyDefaultProperties_NullData()
  {
    mitk::MapperProviderBase<TestMapperA, TestData> provider(mitk::BaseRenderer::Standard2D);

    auto node = mitk::DataNode::New();
    this->Registry().ApplyDefaultProperties(node);

    CPPUNIT_ASSERT(node->GetProperty("test.marker") == nullptr);

    this->Registry().ApplyDefaultProperties(nullptr);
  }

private:
  mitk::MapperProviderRegistry &Registry() const
  {
    return mitk::MapperProviderRegistry::GetInstance();
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkMapperProviderRegistry)
