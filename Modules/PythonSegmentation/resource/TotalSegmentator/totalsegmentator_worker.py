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

# Hosts TotalSegmentator for mitk::TotalSegmentatorTool in a worker process.
#
# TotalSegmentator is imported only where it is needed: version() runs before
# the release is known to be the right one, and info() reads the pure data of
# its task registry without torch, so that the tasks are known quickly.

import importlib.metadata

# The speeds of MITK and the sub modes of a task in the task config.
_SUB_MODES = {'standard': 'default', 'fast': 'fast', 'fastest': 'fastest'}


def version():
    """The installed release of TotalSegmentator, or None."""
    try:
        return importlib.metadata.version('TotalSegmentator')
    except importlib.metadata.PackageNotFoundError:
        return None


def _task_ids(config, mode):
    """The ids of the models a task runs in a sub mode, or None if it has no such mode."""
    if 'sub_modes' in config:
        config = config['sub_modes'].get(mode)

        if config is None:
            return None
    elif mode != 'default':
        return None

    ids = config['task_id']
    return ids if isinstance(ids, list) else [ids]


def _obtainable(task_id, weights, licensed_ids):
    """Whether TotalSegmentator can download the weights of a model."""
    entry = weights.get(task_id)

    if entry is None:
        return False

    if task_id in licensed_ids or entry.get('commercial'):
        return True

    # Models of TotalSegmentator's own work in progress have no release yet.
    return entry.get('version') not in (None, 'TODO')


def info():
    """The tasks whose models TotalSegmentator can get, with what MITK needs to know of them."""
    from totalsegmentator.map_tasks_config import TASK_CONFIGS, TASK_ID_WEIGHTS_CONFIGS
    from totalsegmentator.map_to_binary import commercial_models
    from totalsegmentator.registry import TASKS, get_task_classes, requires_license, task_modality

    licensed_ids = set(commercial_models.values())
    tasks = []

    for name in TASKS:
        config = TASK_CONFIGS.get(name)

        if config is None:
            continue

        speeds = []

        for speed, mode in _SUB_MODES.items():
            ids = _task_ids(config, mode)

            if ids is None or not all(_obtainable(i, TASK_ID_WEIGHTS_CONFIGS, licensed_ids) for i in ids):
                continue

            resample = config['sub_modes'][mode].get('resample') if 'sub_modes' in config else None
            speeds.append({'name': speed, 'resolution': resample if isinstance(resample, (int, float)) else None})

        if not any(speed['name'] == 'standard' for speed in speeds):
            continue

        tasks.append({
            'name': name,
            'modality': task_modality(name),
            'license_required': requires_license(name),
            # Only these tasks take a subset of their classes.
            'roi_subset': name.startswith('total'),
            'classes': {str(index): class_name for index, class_name in get_task_classes(name).items()},
            'speeds': speeds
        })

    return {'version': version(), 'tasks': tasks}


def missing_weights(task, speed):
    """The ids of the models of a task and speed that are not downloaded yet.

    TotalSegmentator downloads further models it needs on its own during a run,
    like the one that locates a subset of classes.
    """
    from pathlib import Path
    from totalsegmentator.config import get_weights_dir
    from totalsegmentator.map_tasks_config import TASK_CONFIGS, TASK_ID_WEIGHTS_CONFIGS

    weights_dir = get_weights_dir()
    missing = []

    for task_id in _task_ids(TASK_CONFIGS[task], _SUB_MODES[speed]) or []:
        entry = TASK_ID_WEIGHTS_CONFIGS[task_id]
        folder = Path(entry.get('rel_path') or '') / entry['foldername']

        if not (weights_dir / folder).exists():
            missing.append(task_id)

    return missing


def download_weights(task_id):
    from totalsegmentator.libs import download_pretrained_weights
    download_pretrained_weights(task_id)


def segment(image, labels, index_to_world, task, speed, roi_subset, device):
    """Segments an image and writes the class indices of the result into labels.

    The image is the array of MITK, (Z, Y, X), with the 4x4 index-to-world
    matrix of its geometry, and labels a buffer of its shape. Returns the
    classes present in the result, {index: name}.
    """
    import gc
    import nibabel as nib
    import numpy as np
    import torch
    from totalsegmentator.python_api import totalsegmentator
    from totalsegmentator.registry import get_task_classes

    # MITK worlds are LPS, NIfTI worlds RAS. The index axes of a NIfTI image are
    # (x, y, z), and TotalSegmentator returns the result in the orientation and
    # shape of its input.
    affine = np.diag([-1.0, -1.0, 1.0, 1.0]) @ np.asarray(index_to_world, dtype=np.float64)
    nifti = nib.Nifti1Image(np.asarray(image).transpose(2, 1, 0), affine)

    try:
        result = totalsegmentator(nifti, None, ml=True, task=task, fast=speed == 'fast', fastest=speed == 'fastest',
                                  roi_subset=roi_subset or None, device=device)

        data = np.asanyarray(result.dataobj)

        if data.shape != nifti.shape:
            raise RuntimeError(f'TotalSegmentator returned a result of shape {data.shape} for an image of shape {nifti.shape}.')

        labels[...] = data.transpose(2, 1, 0)
        present = sorted(int(value) for value in np.unique(data) if value != 0)
    finally:
        # The next run of the worker may come much later.
        result = data = None
        gc.collect()

        if torch.cuda.is_available():
            torch.cuda.empty_cache()

    classes = get_task_classes(task)
    return {str(index): classes.get(index, f'{task}_{index}') for index in present}
