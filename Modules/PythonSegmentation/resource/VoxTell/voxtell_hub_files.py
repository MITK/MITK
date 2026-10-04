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

# The files of the Hugging Face Hub that VoxTell loads, named the way VoxTell
# itself names them, so that MITK can download them up front and show how far
# it is. Each function returns a list of (repo_id, allow_patterns) pairs.

import json
import os


def voxtell_prompt_list_files():
    """The embeddings of the prompts VoxTell knows, which it loads with any model."""
    from voxtell.utils.embedding_bank import DEFAULT_HF_REPO, hf_embedding_path

    return [(DEFAULT_HF_REPO, [hf_embedding_path()])]


def voxtell_model_files():
    """The default model, which VoxTell loads unless it is given a folder."""
    if os.environ.get('VOXTELL_MODEL'):
        return []

    # VoxTell has no public name for its default model yet.
    from voxtell.inference.predictor import _DEFAULT_MODEL, _DEFAULT_MODEL_REPO

    return [(_DEFAULT_MODEL_REPO, [_DEFAULT_MODEL + '/*'])]


def voxtell_text_model_files():
    """The text model, which VoxTell loads for prompts it does not know."""
    import inspect
    from voxtell.inference.predictor import VoxTellPredictor

    name = inspect.signature(VoxTellPredictor.__init__).parameters['text_encoding_model'].default

    # What transformers loads of it: the configuration, the tokenizer and the
    # weights, but no other formats of the weights the repository may offer.
    return [(name, ['*.json', '*.txt', '*.jinja', '*.safetensors'])]


def voxtell_text_model_cached():
    """Whether the text model can be loaded without downloading anything."""
    from huggingface_hub import snapshot_download

    for repo_id, patterns in voxtell_text_model_files():
        try:
            folder = snapshot_download(repo_id, allow_patterns=patterns, local_files_only=True)
        except Exception:
            return False

        # The cache does not always know whether a snapshot is complete, for
        # example after an interrupted download. The weights are what counts,
        # and the index that transformers writes along with them lists them.
        index = os.path.join(folder, 'model.safetensors.index.json')

        if os.path.isfile(index):
            with open(index, encoding='utf-8') as file:
                weights = set(json.load(file)['weight_map'].values())
        else:
            weights = {'model.safetensors'}

        if not all(os.path.isfile(os.path.join(folder, weight)) for weight in weights):
            return False

    return True
