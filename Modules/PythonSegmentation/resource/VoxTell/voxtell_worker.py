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

# Hosts the VoxTell predictor of mitk::VoxTellTool in a worker process.

import os
import sys

# Both are read when torch and nnU-Net are imported, so they have to be set
# before VoxTell is. The allocator setting is what the napari plugin of VoxTell
# uses, to keep the memory of the large buffers of a run from fragmenting;
# PyTorch does not support it on Windows and warns about it there. nnU-Net
# warns about paths that it needs for training only; dummies keep it quiet.
if sys.platform != 'win32':
    os.environ.setdefault('PYTORCH_CUDA_ALLOC_CONF', 'expandable_segments:True')

os.environ.setdefault('nnUNet_raw', '/tmp/nnUNet/raw')
os.environ.setdefault('nnUNet_preprocessed', '/tmp/nnUNet/preprocessed')
os.environ.setdefault('nnUNet_results', '/tmp/nnUNet/results')

import mitk_worker
import voxtell_bridge

_predictor = None

# The prompts VoxTell has embeddings of, in lower case as it looks them up.
_known_prompts = set()

# The masks of the last run, how to orient them back, and the buffer of MITK
# that write_mask() writes into.
_run = None


def load_model(model_dir, device):
    """Loads the model from a folder, or the default model if model_dir is None.

    Returns {'error': message} if that fails.
    """
    global _predictor, _known_prompts

    import torch
    from pathlib import Path
    from voxtell.inference.predictor import VoxTellPredictor

    try:
        if model_dir is not None and not Path(model_dir).is_dir():
            raise FileNotFoundError('Folder not found: ' + model_dir)

        _predictor = VoxTellPredictor(model_dir=model_dir, device=torch.device(device))
        _known_prompts = set(_predictor.list_available_embeddings())
    except Exception as e:
        return {'error': str(e) or type(e).__name__}

    return {}


def unknown_prompts(prompts):
    return [prompt for prompt in prompts if prompt.lower() not in _known_prompts]


def known_prompts():
    return sorted(_known_prompts)


def is_text_model_loaded():
    return _predictor.text_backbone is not None


def embed_prompts(prompts):
    """Embeds prompts, loading the text encoder if one of them needs it.

    VoxTell keeps the embeddings it computes for the session.
    """
    _predictor.embed_text_prompts(prompts)
    _known_prompts.update(prompt.lower() for prompt in prompts)


def _report_progress(done, total):
    # VoxTell stops when this returns False, and requires a real bool.
    return bool(mitk_worker.report_progress(done=done, total=total))


def segment(image, mask, index_to_world, prompts):
    """Segments the prompts in the image and keeps the masks for write_mask().

    The image is the array of MITK, (Z, Y, X), and mask a buffer of its shape.
    """
    global _run

    _run = None

    embeddings = _predictor.embed_text_prompts(prompts)
    data, ornt = voxtell_bridge.voxtell_prepare_input(image, index_to_world)
    masks = _predictor.predict_single_image(data, text_embeddings=embeddings, progress_callback=_report_progress)

    _known_prompts.update(prompt.lower() for prompt in prompts)
    _run = (masks, ornt, mask)


def write_mask(index):
    """Writes the mask of one prompt of the last run into the buffer, as 1 and 0."""
    masks, ornt, target = _run
    voxtell_bridge.voxtell_write_mask(masks, index, ornt, target)


def release_run():
    global _run
    _run = None
