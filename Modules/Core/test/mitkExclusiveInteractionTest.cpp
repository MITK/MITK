/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkExclusiveInteraction.h>

#include <mitkException.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <stdexcept>
#include <utility>

namespace
{
  using mitk::ExclusiveInteraction;

  ExclusiveInteraction::RevokeFunction Agree(int& numberOfCalls)
  {
    return [&numberOfCalls]() { ++numberOfCalls; return true; };
  }

  ExclusiveInteraction::RevokeFunction Refuse(int& numberOfCalls)
  {
    return [&numberOfCalls]() { ++numberOfCalls; return false; };
  }
}

class mitkExclusiveInteractionTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkExclusiveInteractionTestSuite);
  MITK_TEST(Acquire_WithoutHolder_IsActive);
  MITK_TEST(Acquire_EmptyRevoke_Throws);
  MITK_TEST(Acquire_WithHolder_RevokesHolder);
  MITK_TEST(Acquire_HolderRefuses_ReturnsInactiveClaim);
  MITK_TEST(Acquire_HolderReleasesWithinRevoke_Succeeds);
  MITK_TEST(Acquire_WithinRevoke_ReturnsInactiveClaim);
  MITK_TEST(Acquire_RevokeThrows_HolderKeepsSlot);
  MITK_TEST(Reset_DoesNotRevoke);
  MITK_TEST(Reset_OfRevokedClaim_KeepsCurrentClaim);
  MITK_TEST(Claim_ReleasedOnDestruction);
  MITK_TEST(Claim_DefaultConstructed_IsInactive);
  MITK_TEST(Claim_Move_TransfersClaim);
  CPPUNIT_TEST_SUITE_END();

public:

  void Acquire_WithoutHolder_IsActive()
  {
    int numberOfRevokes = 0;
    const auto claim = ExclusiveInteraction::Acquire(Agree(numberOfRevokes));

    CPPUNIT_ASSERT(claim.IsActive());
    CPPUNIT_ASSERT_EQUAL(0, numberOfRevokes);
  }

  void Acquire_EmptyRevoke_Throws()
  {
    CPPUNIT_ASSERT_THROW(auto claim = ExclusiveInteraction::Acquire(nullptr), mitk::Exception);
  }

  void Acquire_WithHolder_RevokesHolder()
  {
    int numberOfFirstRevokes = 0;
    int numberOfSecondRevokes = 0;

    const auto first = ExclusiveInteraction::Acquire(Agree(numberOfFirstRevokes));
    const auto second = ExclusiveInteraction::Acquire(Agree(numberOfSecondRevokes));

    CPPUNIT_ASSERT_EQUAL(1, numberOfFirstRevokes);
    CPPUNIT_ASSERT_EQUAL(0, numberOfSecondRevokes);
    CPPUNIT_ASSERT(!first.IsActive());
    CPPUNIT_ASSERT(second.IsActive());
  }

  void Acquire_HolderRefuses_ReturnsInactiveClaim()
  {
    int numberOfFirstRevokes = 0;
    int numberOfSecondRevokes = 0;

    const auto first = ExclusiveInteraction::Acquire(Refuse(numberOfFirstRevokes));
    const auto second = ExclusiveInteraction::Acquire(Agree(numberOfSecondRevokes));

    CPPUNIT_ASSERT_EQUAL(1, numberOfFirstRevokes);
    CPPUNIT_ASSERT(first.IsActive());
    CPPUNIT_ASSERT(!second.IsActive());

    // The refused acquisition left nothing behind that could be revoked later.
    int numberOfThirdRevokes = 0;
    const auto third = ExclusiveInteraction::Acquire(Agree(numberOfThirdRevokes));

    CPPUNIT_ASSERT_EQUAL(2, numberOfFirstRevokes);
    CPPUNIT_ASSERT_EQUAL(0, numberOfSecondRevokes);
    CPPUNIT_ASSERT(!third.IsActive());
  }

  void Acquire_HolderReleasesWithinRevoke_Succeeds()
  {
    ExclusiveInteraction::Claim first;
    first = ExclusiveInteraction::Acquire([&first]() { first.Reset(); return true; });

    int numberOfRevokes = 0;
    const auto second = ExclusiveInteraction::Acquire(Agree(numberOfRevokes));

    CPPUNIT_ASSERT(!first.IsActive());
    CPPUNIT_ASSERT(second.IsActive());

    // A holder that releases its claim frees the slot even if it answers false.
    ExclusiveInteraction::Claim third;
    third = ExclusiveInteraction::Acquire([&third]() { third.Reset(); return false; });
    CPPUNIT_ASSERT(!second.IsActive());

    const auto fourth = ExclusiveInteraction::Acquire(Agree(numberOfRevokes));
    CPPUNIT_ASSERT(fourth.IsActive());
  }

  void Acquire_WithinRevoke_ReturnsInactiveClaim()
  {
    int numberOfNestedRevokes = 0;
    ExclusiveInteraction::Claim nested;

    const auto first = ExclusiveInteraction::Acquire([&]() {
      nested = ExclusiveInteraction::Acquire(Agree(numberOfNestedRevokes));
      return true;
    });

    int numberOfRevokes = 0;
    const auto second = ExclusiveInteraction::Acquire(Agree(numberOfRevokes));

    CPPUNIT_ASSERT(!nested.IsActive());
    CPPUNIT_ASSERT(second.IsActive());
    CPPUNIT_ASSERT_EQUAL(0, numberOfNestedRevokes);
  }

  void Acquire_RevokeThrows_HolderKeepsSlot()
  {
    auto first = ExclusiveInteraction::Acquire([]() -> bool { throw std::runtime_error("revoke failed"); });

    int numberOfRevokes = 0;
    CPPUNIT_ASSERT_THROW(auto second = ExclusiveInteraction::Acquire(Agree(numberOfRevokes)), std::runtime_error);
    CPPUNIT_ASSERT(first.IsActive());

    // The failed revocation must not leave the slot marked as being revoked,
    // which would refuse every later acquisition.
    first.Reset();
    const auto third = ExclusiveInteraction::Acquire(Agree(numberOfRevokes));
    CPPUNIT_ASSERT(third.IsActive());
  }

  void Reset_DoesNotRevoke()
  {
    int numberOfFirstRevokes = 0;
    auto first = ExclusiveInteraction::Acquire(Agree(numberOfFirstRevokes));

    first.Reset();
    CPPUNIT_ASSERT(!first.IsActive());

    int numberOfSecondRevokes = 0;
    const auto second = ExclusiveInteraction::Acquire(Agree(numberOfSecondRevokes));

    CPPUNIT_ASSERT_EQUAL(0, numberOfFirstRevokes);
    CPPUNIT_ASSERT(second.IsActive());
  }

  void Reset_OfRevokedClaim_KeepsCurrentClaim()
  {
    int numberOfRevokes = 0;
    auto first = ExclusiveInteraction::Acquire(Agree(numberOfRevokes));
    const auto second = ExclusiveInteraction::Acquire(Agree(numberOfRevokes));

    first.Reset();

    CPPUNIT_ASSERT(second.IsActive());
  }

  void Claim_ReleasedOnDestruction()
  {
    int numberOfFirstRevokes = 0;

    {
      const auto first = ExclusiveInteraction::Acquire(Agree(numberOfFirstRevokes));
    }

    int numberOfSecondRevokes = 0;
    const auto second = ExclusiveInteraction::Acquire(Agree(numberOfSecondRevokes));

    CPPUNIT_ASSERT_EQUAL(0, numberOfFirstRevokes);
    CPPUNIT_ASSERT(second.IsActive());
  }

  void Claim_DefaultConstructed_IsInactive()
  {
    ExclusiveInteraction::Claim claim;
    CPPUNIT_ASSERT(!claim.IsActive());

    claim.Reset();
    CPPUNIT_ASSERT(!claim.IsActive());
  }

  void Claim_Move_TransfersClaim()
  {
    int numberOfRevokes = 0;
    auto source = ExclusiveInteraction::Acquire(Agree(numberOfRevokes));

    ExclusiveInteraction::Claim target;
    target = std::move(source);

    CPPUNIT_ASSERT(!source.IsActive());
    CPPUNIT_ASSERT(target.IsActive());

    auto constructed = std::move(target);

    CPPUNIT_ASSERT(!target.IsActive());
    CPPUNIT_ASSERT(constructed.IsActive());

    source.Reset();
    target.Reset();
    CPPUNIT_ASSERT(constructed.IsActive());
    CPPUNIT_ASSERT_EQUAL(0, numberOfRevokes);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkExclusiveInteraction)
