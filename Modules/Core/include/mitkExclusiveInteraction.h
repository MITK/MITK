/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkExclusiveInteraction_h
#define mitkExclusiveInteraction_h

#include <MitkCoreExports.h>

#include <functional>

namespace mitk
{
  /**
   * \brief Lets at most one interactive tool be armed at a time, across all views.
   *
   * A tool is armed while it waits for or processes input in the render
   * windows, for example an active segmentation tool or a measurement figure
   * that is about to be placed. Components with such a state acquire a claim
   * when they arm and release it when they disarm. Acquiring asks the holder of
   * the current claim to disarm first, so that tools of different views are
   * never armed at the same time.
   *
   * Display interaction and the interactors of data that can merely be edited,
   * like the control points of a finished planar figure, do not take part.
   *
   * Only to be used from the GUI thread.
   *
   * \ingroup Interaction
   */
  class MITKCORE_EXPORT ExclusiveInteraction
  {
  public:
    /**
     * \brief Called when another component acquires the slot.
     *
     * Disarms the owner, for example by cancelling an unfinished figure and
     * unchecking the tool button, and returns true. Returns false to keep the
     * slot instead, for example after the user declined to discard unconfirmed
     * results. Releasing the own claim from within is allowed.
     */
    using RevokeFunction = std::function<bool()>;

    /**
     * \brief Holds the slot while it is active.
     *
     * A default-constructed claim is inactive. Moving transfers it. Destruction
     * or Reset() releases it without calling its revoke function, and a claim
     * whose holder agreed to be revoked becomes inactive.
     */
    class MITKCORE_EXPORT Claim
    {
    public:
      Claim() = default;
      Claim(Claim&& other) noexcept;
      Claim& operator=(Claim&& other) noexcept;
      ~Claim();

      Claim(const Claim&) = delete;
      Claim& operator=(const Claim&) = delete;

      /** \brief True while this claim holds the slot. */
      bool IsActive() const;

      /** \brief Releases the slot. Does nothing if the claim is inactive. */
      void Reset();

    private:
      friend class ExclusiveInteraction;

      unsigned long m_Id = 0;
    };

    ExclusiveInteraction() = delete;

    /**
     * \brief Acquires the slot, revoking the current claim first.
     *
     * \param revoke Called once a later Acquire() wants the slot, see RevokeFunction.
     * \return An active claim, or an inactive one if the holder of the current
     *         claim refused to be revoked or is being revoked already, for
     *         example while its revoke function waits for the user's answer.
     * \throw mitk::Exception if \p revoke is empty.
     */
    [[nodiscard]] static Claim Acquire(RevokeFunction revoke);
  };
}

#endif
