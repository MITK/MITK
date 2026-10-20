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

# Worker module for mitk::Torch::SelectDevice(), which decides on the device.


def query_cuda_device(index):
    """Returns the properties of a CUDA device, or None if PyTorch cannot use it."""
    try:
        import torch

        if not torch.cuda.is_available():
            return None

        properties = torch.cuda.get_device_properties(index)

        return {
            'name': torch.cuda.get_device_name(index),
            'total_memory_mb': properties.total_memory // (1024 ** 2),
            'major': properties.major,
            'minor': properties.minor
        }
    except Exception:
        return None
