/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkExclusiveInteraction.h>

#include <mitkExceptionMacro.h>

#include <utility>

namespace
{
  // Zero means that no claim holds the slot.
  unsigned long CurrentClaimId = 0;
  unsigned long LastClaimId = 0;
  mitk::ExclusiveInteraction::RevokeFunction CurrentRevoke;
  bool IsRevoking = false;

  class RevokingGuard
  {
  public:
    RevokingGuard() { IsRevoking = true; }
    ~RevokingGuard() { IsRevoking = false; }

    RevokingGuard(const RevokingGuard&) = delete;
    RevokingGuard& operator=(const RevokingGuard&) = delete;
  };
}

mitk::ExclusiveInteraction::Claim::Claim(Claim&& other) noexcept
  : m_Id(std::exchange(other.m_Id, 0))
{
}

mitk::ExclusiveInteraction::Claim& mitk::ExclusiveInteraction::Claim::operator=(Claim&& other) noexcept
{
  if (this != &other)
  {
    this->Reset();
    m_Id = std::exchange(other.m_Id, 0);
  }

  return *this;
}

mitk::ExclusiveInteraction::Claim::~Claim()
{
  this->Reset();
}

bool mitk::ExclusiveInteraction::Claim::IsActive() const
{
  return m_Id != 0 && m_Id == CurrentClaimId;
}

void mitk::ExclusiveInteraction::Claim::Reset()
{
  if (this->IsActive())
  {
    CurrentClaimId = 0;
    CurrentRevoke = nullptr;
  }

  m_Id = 0;
}

mitk::ExclusiveInteraction::Claim mitk::ExclusiveInteraction::Acquire(RevokeFunction revoke)
{
  if (!revoke)
    mitkThrow() << "Cannot acquire exclusive interaction without a revoke function.";

  if (IsRevoking)
    return {};

  if (CurrentClaimId != 0)
  {
    const auto holderId = CurrentClaimId;

    // Call a copy, as the holder may release its claim from within, which
    // resets the stored function.
    const auto revokeHolder = CurrentRevoke;
    bool isRevoked = false;

    {
      RevokingGuard guard;
      isRevoked = revokeHolder();
    }

    if (!isRevoked && CurrentClaimId == holderId)
      return {};
  }

  Claim claim;
  claim.m_Id = ++LastClaimId;

  CurrentClaimId = claim.m_Id;
  CurrentRevoke = std::move(revoke);

  return claim;
}
