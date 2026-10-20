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

# Hosts the nnInteractive inference session of mitk::nnInteractiveTool in a
# worker process. Either session kind works on the image and target buffer that
# set_image() received: nnInteractive writes each prediction into the target
# buffer, which is shared with MITK.
#
# A client-only installation has neither PyTorch nor the local inference
# backend, so those are imported where needed only.

import os

_session = None
_remote = False


class RemoteConnectionLost(Exception):
    """The remote session is gone: the lease expired, or the server is at capacity or unreachable."""


def _guard_remote(function):
    def guarded(*args, **kwargs):
        if not _remote:
            return function(*args, **kwargs)

        import httpx
        from nnInteractive.inference.remote import ServerAtCapacityError, SessionExpiredError

        try:
            return function(*args, **kwargs)
        except (httpx.HTTPError, ServerAtCapacityError, SessionExpiredError) as e:
            raise RemoteConnectionLost(repr(e)) from e

    guarded.__name__ = function.__name__
    return guarded


def _capabilities():
    supported = getattr(_session, 'supported_interactions', None)

    # Without metadata, keep every interaction usable.
    if supported is None:
        interactions = {'point': True, 'box': True, 'scribble': True, 'lasso': True, 'mask': True}
    else:
        interactions = {
            'point': bool(supported.get('points', False)),
            'box': bool(supported.get('bbox2d', False) or supported.get('bbox3d', False)),
            'scribble': bool(supported.get('scribble', False)),
            'lasso': bool(supported.get('lasso', False)),
            'mask': bool(getattr(_session, 'supports_initial_label', True))
        }

    return {
        'interactions': interactions,
        'license': getattr(_session, 'license', None) or '',
        'undo': bool(getattr(_session, 'supports_undo', False))
    }


def is_local_inference_available():
    """Whether the full distribution is installed, not just nninteractive-client.

    Both share the import namespace, so this reads the distribution metadata.
    """
    import importlib.metadata

    try:
        importlib.metadata.version('nnInteractive')
        return True
    except importlib.metadata.PackageNotFoundError:
        return False


def _find_inference_class(checkpoint_path):
    # nnInteractive 2.5.0 names the class in inference_info.json; older
    # checkpoints use inference_session_class.json. Either file holds a bare
    # string or a dict with an 'inference_class' key.
    import nnInteractive
    from batchgenerators.utilities.file_and_folder_operations import join, load_json
    from nnunetv2.utilities.find_class_by_name import recursive_find_python_class

    inference_class = None
    info = checkpoint_path / 'inference_info.json'

    if info.is_file():
        data = load_json(info)
        if isinstance(data, dict):
            inference_class = data.get('inference_class')
        elif isinstance(data, str):
            inference_class = data

    if inference_class is None:
        legacy = checkpoint_path / 'inference_session_class.json'
        if legacy.is_file():
            data = load_json(legacy)
            inference_class = data['inference_class'] if isinstance(data, dict) else data

    if inference_class is None:
        inference_class = 'nnInteractiveInferenceSession'

    return recursive_find_python_class(join(nnInteractive.__path__[0], 'inference'), inference_class, 'nnInteractive.inference')


def start_local(device, model_source, local_model_path, model_checkpoint, use_torch_compile, do_autozoom, interactions_storage):
    """Loads the model and starts a local session.

    Returns {'error': message} for the failures a user can fix, otherwise the
    capabilities of the session.
    """
    global _session, _remote

    from importlib.metadata import version
    from pathlib import Path

    import torch

    print(f'nnInteractive version: {version("nnInteractive")}')

    if model_source == 'local':
        print('Model source: local folder')
        checkpoint_path = Path(local_model_path)

        if not checkpoint_path.is_dir():
            raise RuntimeError(f'nnInteractive checkpoint folder not found: {checkpoint_path}')
    else:
        # ensure_model_available() is idempotent and works offline once the
        # checkpoint was downloaded. An empty checkpoint means the default.
        print('Model source: managed (nnInteractive model management)')

        try:
            from nnInteractive.model_management import ensure_model_available, get_default_model_id
            model_id = model_checkpoint or get_default_model_id()
            print(f'Model checkpoint: {model_id}')
            checkpoint_path = Path(ensure_model_available(model_id))
        except ImportError:
            return {'error': 'The installed nnInteractive does not provide model management. '
                             'Upgrade nnInteractive to v2.5.0 or newer.'}
        except Exception as e:
            return {'error': 'Could not obtain the nnInteractive model checkpoint. Check your '
                             'internet connection, or set a local checkpoint folder in '
                             f'Preferences -> Segmentation -> nnInteractive. ({e})'}

    # nnU-Net requires these for training only; they silence its warnings.
    os.environ.setdefault('nnUNet_raw', '/tmp/nnUNet/raw')
    os.environ.setdefault('nnUNet_preprocessed', '/tmp/nnUNet/preprocessed')
    os.environ.setdefault('nnUNet_results', '/tmp/nnUNet/results')

    inference_class = _find_inference_class(checkpoint_path)

    session = inference_class(
        device=torch.device(device),
        use_torch_compile=use_torch_compile,
        torch_n_threads=os.cpu_count(),
        verbose=False,
        do_autozoom=do_autozoom,
        interactions_storage=interactions_storage
    )
    session.initialize_from_trained_model_folder(checkpoint_path)

    _session = session
    _remote = False

    return {'capabilities': _capabilities()}


def _attempt_remote(server_url, server_display, api_key):
    import httpx
    from nnInteractive.inference.remote import (
        nnInteractiveRemoteInferenceSession, ServerAtCapacityError, SessionExpiredError)

    try:
        return nnInteractiveRemoteInferenceSession(server_url=server_url, api_key=api_key), '', False
    except ServerAtCapacityError:
        return None, 'The nnInteractive server is at capacity. Please try again later.', False
    except SessionExpiredError:
        return None, 'The nnInteractive server rejected the session request. Please try again.', False
    except httpx.HTTPStatusError as e:
        if e.response.status_code == 401:
            return None, 'The nnInteractive server rejected the API key. Check the API key in the nnInteractive preferences.', False
        if 'text/html' in e.response.headers.get('content-type', ''):
            return None, f'The server at {server_display} returned a web page instead of a response.', True
        return None, f'The nnInteractive server returned an error (HTTP {e.response.status_code}).', False
    except (httpx.ConnectError, httpx.ConnectTimeout):
        return None, (f'Could not reach the nnInteractive server at {server_display}. '
                      'Check the server URL and make sure the server is running.'), False
    except httpx.HTTPError as e:
        return None, f'Could not connect to the nnInteractive server at {server_display}: {e}', False
    except (ValueError, KeyError):
        return None, f'The server at {server_display} returned an unexpected response.', True


def _connect_remote(server_url, server_display, api_key):
    # A response that looks like proxy interception gets one retry with the
    # host appended to no_proxy, but only if a proxy is configured for the
    # scheme of the server and the host is not bypassed already. httpx reads
    # the proxy environment when the session creates its client, so no_proxy is
    # restored right after the retry.
    import urllib.parse
    import urllib.request

    session, error, proxy_suspected = _attempt_remote(server_url, server_display, api_key)

    if session is not None or not proxy_suspected:
        return session, error, False

    parts = urllib.parse.urlsplit(server_url)
    host = parts.hostname or ''
    proxies = urllib.request.getproxies()
    scheme = parts.scheme or 'http'

    if not host or (scheme not in proxies and 'all' not in proxies) or urllib.request.proxy_bypass(host):
        return None, error + ' Check the server URL.', False

    old = os.environ.get('no_proxy')
    base = old if old is not None else os.environ.get('NO_PROXY')
    os.environ['no_proxy'] = host if not base else base + ',' + host

    try:
        session, retry_error, _ = _attempt_remote(server_url, server_display, api_key)
    finally:
        if old is None:
            os.environ.pop('no_proxy', None)
        else:
            os.environ['no_proxy'] = old

    if session is not None:
        return session, '', True

    return None, (f'An HTTP proxy seems to intercept the connection to {server_display}, '
                  'and connecting directly (bypassing the proxy) failed as well: ' + retry_error), False


def start_remote(server_url, server_display, api_key):
    """Claims a session on an nninteractive-server.

    Returns {'error': message} if that fails, otherwise the capabilities of
    the session and whether the configured proxy had to be bypassed.
    """
    global _session, _remote

    try:
        session, error, proxy_bypassed = _connect_remote(server_url, server_display, api_key or None)
    except ImportError:
        return {'error': 'The installed nnInteractive does not support remote mode. Reinstall or upgrade nnInteractive.'}

    if session is None:
        return {'error': error}

    _session = session
    _remote = True

    return {'capabilities': _capabilities(), 'proxy_bypassed': proxy_bypassed}


@_guard_remote
def set_image(image, target, spacing):
    if _remote:
        # The client writes prediction diffs into the array itself.
        _session.set_image(image[None], {'spacing': spacing})
        _session.set_target_buffer(target)
    else:
        import torch
        _session.set_image(image[None], {'spacing': spacing})
        _session.set_target_buffer(torch.from_numpy(target))


@_guard_remote
def set_autozoom(enabled):
    _session.set_do_autozoom(enabled)


@_guard_remote
def add_point(coordinates, include):
    _session.add_point_interaction(coordinates, include_interaction=include)


@_guard_remote
def add_bbox(bbox, include):
    _session.add_bbox_interaction(bbox, include_interaction=include)


@_guard_remote
def add_scribble(mask, include, bbox=None):
    kwargs = {} if bbox is None else {'interaction_bbox': bbox}
    _session.add_scribble_interaction(mask, include_interaction=include, **kwargs)


@_guard_remote
def add_lasso(mask, include, bbox=None):
    kwargs = {} if bbox is None else {'interaction_bbox': bbox}
    _session.add_lasso_interaction(mask, include_interaction=include, **kwargs)


@_guard_remote
def add_initial_seg(mask, run_prediction):
    import numpy as np
    _session.add_initial_seg_interaction(mask.astype(np.uint8), run_prediction=run_prediction)


@_guard_remote
def reset():
    _session.reset_interactions()


@_guard_remote
def undo():
    return bool(_session.undo())


def close():
    """Releases the lease of a remote session right away instead of waiting for the reaper."""
    if _remote and _session is not None:
        _session.close()
