/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMxNGroupJoinMode_h
#define QmitkMxNGroupJoinMode_h

/**
 * \brief How a cell's existing links are treated when it joins a group - the
 *        drop-mode selector for assigning windows to a group in the MxN editor.
 *
 * Replace is the default and the effect of a plain drop: the cell wholly joins
 * the target group, so every other link is cleared first (its selection reverts
 * to the default group). FillEmpty sets only the cell's currently-unlinked axes,
 * leaving its existing links untouched. MergeOverwriteCollisions sets the group's
 * axes (overwriting a colliding link) but keeps the cell's links on axes the
 * group does not cover.
 */
enum class QmitkMxNGroupJoinMode
{
  Replace,
  FillEmpty,
  MergeOverwriteCollisions
};

#endif
