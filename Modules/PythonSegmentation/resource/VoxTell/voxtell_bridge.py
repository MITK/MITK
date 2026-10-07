# ============================================================================
#
# The Medical Imaging Interaction Toolkit (MITK)
#
# Copyright (c) German Cancer Research Center (DKFZ)
# All rights reserved.
#
# Use of this source code is governed by a 3-clause BSD license that can be
# found in the LICENSE file.
#
# ============================================================================

# Glue between a MITK image and the array contract of VoxTell. VoxTell takes the
# array that the reader of nnU-Net returns for a NIfTI file: reoriented to the
# closest canonical RAS orientation, as float32 of shape (1, Z, Y, X). It knows
# nothing about geometry, so the reorientation a NIfTI reader does implicitly has
# to be done here from the geometry of the MITK image. That keeps the tool
# independent of the file format the image was loaded from.

import numpy as np


def voxtell_ras_affine(image, time_step=0):
    """Returns the index-to-world matrix of an mitk.Image the way nibabel expects it.

    The voxel index is in (x, y, z) order and the world is RAS. MITK worlds are LPS
    and column j of the direction matrix is the world direction of index axis j.
    """
    spacing = np.asarray(image.get_spacing(time_step), dtype=np.float64)
    origin = np.asarray(image.get_origin(time_step), dtype=np.float64)
    direction = np.asarray(image.get_direction(time_step), dtype=np.float64)

    affine = np.eye(4)
    affine[:3, :3] = direction * spacing
    affine[:3, 3] = origin

    return np.diag([-1.0, -1.0, 1.0, 1.0]) @ affine


def voxtell_apply_orientation(array, ornt):
    """Applies an orientation to the first three axes of an array, returning a view.

    Same as nibabel.orientations.apply_orientation: axes that run against their
    canonical direction are flipped, then the axes are put into canonical order.
    """
    result = array

    for axis, flip in enumerate(ornt[:, 1]):
        if flip == -1:
            result = np.flip(result, axis=axis)

    return result.transpose(np.argsort(ornt[:, 0]))


def voxtell_inverse_orientation(ornt):
    """Returns the orientation that undoes voxtell_apply_orientation(., ornt)."""
    inverse = np.empty_like(ornt)

    for axis, (target, flip) in enumerate(ornt):
        inverse[int(target)] = (axis, flip)

    return inverse


def voxtell_prepare_input(image, time_step=0):
    """Returns the array VoxTell expects for an mitk.Image, and how to undo it.

    The result is float32 of shape (1, Z, Y, X) in closest canonical RAS
    orientation, exactly what the reader of nnU-Net returns for a NIfTI file of
    the image. The second value is the orientation that was applied.
    """
    # The function the reader of VoxTell uses as well, so both agree on what the
    # closest canonical orientation of an oblique image is.
    from nibabel.orientations import io_orientation

    ornt = io_orientation(voxtell_ras_affine(image, time_step))

    # MITK exposes (Z, Y, X), orientations are defined on (x, y, z).
    xyz = image.as_numpy(writeable=False, time_step=time_step).transpose(2, 1, 0)
    ras_xyz = voxtell_apply_orientation(xyz, ornt)
    data = np.ascontiguousarray(ras_xyz.transpose(2, 1, 0), dtype=np.float32)

    return data[np.newaxis], ornt


def voxtell_write_mask(masks, index, ornt, target):
    """Writes one of the masks of VoxTell into a (Z, Y, X) buffer of the image, as 1 and 0.

    The masks are in the space of the array that voxtell_prepare_input returned
    along with the orientation. Masks can overlap, so each gets a buffer of its
    own rather than a label in a shared one.
    """
    mask = masks[index].transpose(2, 1, 0)
    target[...] = voxtell_apply_orientation(mask, voxtell_inverse_orientation(ornt)).transpose(2, 1, 0) != 0
