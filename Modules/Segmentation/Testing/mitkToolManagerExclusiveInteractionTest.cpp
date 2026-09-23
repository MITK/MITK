/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkAddContourTool.h>
#include <mitkExclusiveInteraction.h>
#include <mitkImage.h>
#include <mitkLabelSetImage.h>
#include <mitkOtsuTool3D.h>
#include <mitkToolManager.h>
#include <mitkToolManagerProvider.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

namespace
{
  mitk::ExclusiveInteraction::RevokeFunction Agree(int& numberOfCalls)
  {
    return [&numberOfCalls]() { ++numberOfCalls; return true; };
  }
}

class mitkToolManagerExclusiveInteractionTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkToolManagerExclusiveInteractionTestSuite);
  MITK_TEST(ActivateTool_ClaimingTool_RevokesArmedToolOfOtherView);
  MITK_TEST(ActivateTool_ClaimingTool_RefusedByArmedToolOfOtherView);
  MITK_TEST(ActivateTool_NonClaimingTool_KeepsArmedToolOfOtherView);
  MITK_TEST(ActivateTool_SwitchToNonClaimingTool_ReleasesClaim);
  MITK_TEST(Acquire_WhileClaimingToolIsActive_DeactivatesTool);
  CPPUNIT_TEST_SUITE_END();

  mitk::DataNode::Pointer m_ReferenceNode;
  mitk::DataNode::Pointer m_WorkingNode;
  mitk::ToolManager* m_ToolManager = nullptr;
  int m_ClaimingToolId = -1;
  int m_NonClaimingToolId = -1;

public:
  void setUp() override
  {
    unsigned int dimensions[] = { 4, 4, 4 };

    auto referenceImage = mitk::Image::New();
    referenceImage->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 3, dimensions);

    auto segmentation = mitk::MultiLabelSegmentation::New();
    segmentation->Initialize(referenceImage);
    mitk::Color color;
    color.Set(1.0f, 0.0f, 0.0f);
    segmentation->AddLabel("Label", color, 0);

    m_ReferenceNode = mitk::DataNode::New();
    m_ReferenceNode->SetData(referenceImage);

    m_WorkingNode = mitk::DataNode::New();
    m_WorkingNode->SetData(segmentation);

    // The tool managers of the provider are the ones that keep a single active tool.
    m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager("mitkToolManagerExclusiveInteractionTest");
    m_ToolManager->InitializeTools();
    m_ToolManager->SetReferenceData(m_ReferenceNode);
    m_ToolManager->SetWorkingData(m_WorkingNode);

    m_ClaimingToolId = m_ToolManager->GetToolIdByToolType<mitk::AddContourTool>();
    m_NonClaimingToolId = m_ToolManager->GetToolIdByToolType<mitk::OtsuTool3D>();

    CPPUNIT_ASSERT(m_ClaimingToolId != -1);
    CPPUNIT_ASSERT(m_NonClaimingToolId != -1);
  }

  void tearDown() override
  {
    m_ToolManager->ActivateTool(-1);
    m_ToolManager->SetWorkingData(mitk::ToolManager::DataVectorType());
    m_ToolManager->SetReferenceData(mitk::ToolManager::DataVectorType());
    m_ToolManager = nullptr;

    m_WorkingNode = nullptr;
    m_ReferenceNode = nullptr;
  }

  void ActivateTool_ClaimingTool_RevokesArmedToolOfOtherView()
  {
    int numberOfRevokes = 0;
    const auto otherView = mitk::ExclusiveInteraction::Acquire(Agree(numberOfRevokes));

    CPPUNIT_ASSERT(m_ToolManager->ActivateTool(m_ClaimingToolId));
    CPPUNIT_ASSERT_EQUAL(1, numberOfRevokes);
    CPPUNIT_ASSERT(!otherView.IsActive());
  }

  void ActivateTool_ClaimingTool_RefusedByArmedToolOfOtherView()
  {
    const auto otherView = mitk::ExclusiveInteraction::Acquire([]() { return false; });

    CPPUNIT_ASSERT(!m_ToolManager->ActivateTool(m_ClaimingToolId));
    CPPUNIT_ASSERT_EQUAL(-1, m_ToolManager->GetActiveToolID());
    CPPUNIT_ASSERT(otherView.IsActive());
  }

  void ActivateTool_NonClaimingTool_KeepsArmedToolOfOtherView()
  {
    int numberOfRevokes = 0;
    const auto otherView = mitk::ExclusiveInteraction::Acquire(Agree(numberOfRevokes));

    CPPUNIT_ASSERT(m_ToolManager->ActivateTool(m_NonClaimingToolId));
    CPPUNIT_ASSERT_EQUAL(0, numberOfRevokes);
    CPPUNIT_ASSERT(otherView.IsActive());
  }

  void ActivateTool_SwitchToNonClaimingTool_ReleasesClaim()
  {
    CPPUNIT_ASSERT(m_ToolManager->ActivateTool(m_ClaimingToolId));
    CPPUNIT_ASSERT(m_ToolManager->ActivateTool(m_NonClaimingToolId));

    int numberOfRevokes = 0;
    const auto otherView = mitk::ExclusiveInteraction::Acquire(Agree(numberOfRevokes));

    CPPUNIT_ASSERT(otherView.IsActive());
    CPPUNIT_ASSERT_EQUAL(m_NonClaimingToolId, m_ToolManager->GetActiveToolID());
  }

  void Acquire_WhileClaimingToolIsActive_DeactivatesTool()
  {
    CPPUNIT_ASSERT(m_ToolManager->ActivateTool(m_ClaimingToolId));

    int numberOfRevokes = 0;
    const auto otherView = mitk::ExclusiveInteraction::Acquire(Agree(numberOfRevokes));

    CPPUNIT_ASSERT(otherView.IsActive());
    CPPUNIT_ASSERT_EQUAL(-1, m_ToolManager->GetActiveToolID());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkToolManagerExclusiveInteraction)
