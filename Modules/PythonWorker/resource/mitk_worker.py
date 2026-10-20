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

# Runtime of a worker process started by mitk::PythonWorker. The bootstrap on
# the command line reads the first line of stdin, which carries this source and
# the modules of the tool as JSON, and executes this source with the remaining
# input stream as mitk_input and the modules as mitk_modules.
#
# Afterwards each line of mitk_input is one JSON request, and each line written
# to the original stdout is one JSON response or progress notification. Library
# output must not reach that channel, so stdout is redirected to stderr, which
# MITK logs.

import faulthandler
import json
import linecache
import os
import queue
import signal
import sys
import threading
import traceback
import types

# The runtime arrives through stdin and has no file. Registered, tracebacks
# show its lines instead of reading whatever mitk_worker.py the working
# directory may hold.
_runtime_source = globals().pop('mitk_runtime', None)

if _runtime_source is not None:
    linecache.cache['mitk_worker.py'] = (len(_runtime_source), None, _runtime_source.splitlines(True), 'mitk_worker.py')

# MITK ends a worker that sends a longer message, the line break included.
_max_message_size = globals().pop('mitk_max_message_size')

_protocol = os.fdopen(os.dup(1), 'wb')
os.dup2(2, 1)
sys.stdout = sys.stderr

_devnull = os.open(os.devnull, os.O_RDONLY)
os.dup2(_devnull, 0)
os.close(_devnull)


def _release_channels_in_child():
    # MITK notices that the worker ended when the channels close. A forked
    # child of the tool would keep them open. Pointing them at the null device
    # instead of closing them keeps the file objects of the child valid.
    devnull = os.open(os.devnull, os.O_RDWR)
    os.dup2(devnull, _protocol.fileno())
    os.dup2(devnull, mitk_input.fileno())
    os.close(devnull)


if hasattr(os, 'register_at_fork'):
    os.register_at_fork(after_in_child=_release_channels_in_child)

if sys.platform == 'win32':
    import ctypes

    # A crash ends the worker at once, without a Windows Error Reporting dialog
    # that would keep the dead process alive while MITK waits for it.
    # faulthandler still prints the cause, which MITK reports. Only this
    # process and its children are affected, not the crash reporting of MITK.
    _SEM_FAILCRITICALERRORS = 0x0001
    _SEM_NOGPFAULTERRORBOX = 0x0002
    ctypes.windll.kernel32.SetErrorMode(
        ctypes.windll.kernel32.GetErrorMode() | _SEM_FAILCRITICALERRORS | _SEM_NOGPFAULTERRORBOX)

sys.stderr.reconfigure(encoding='utf-8', errors='backslashreplace', line_buffering=True)

# From 3.14 on, a C stack follows the Python traceback and pushes the cause out
# of the last lines MITK keeps. Its frames do not help a user.
if sys.version_info >= (3, 14):
    faulthandler.enable(c_stack=False)
else:
    faulthandler.enable()

# Ctrl+C in a terminal reaches MITK and its workers alike; the worker ends with
# MITK instead.
signal.signal(signal.SIGINT, signal.SIG_IGN)

# Child processes of the tool (e.g. torch compile workers) must not inherit
# what the isolated interpreter ignored.
os.environ.pop('PYTHONPATH', None)
os.environ.pop('PYTHONHOME', None)

# MITK may have activated another virtual environment before it started the
# worker. Its children must see the one the worker runs in.
if sys.prefix != sys.base_prefix:
    os.environ['VIRTUAL_ENV'] = sys.prefix
else:
    os.environ.pop('VIRTUAL_ENV', None)

_send_lock = threading.Lock()


def _json_default(value):
    # NumPy scalars and arrays
    if hasattr(value, 'tolist'):
        return value.tolist()
    raise TypeError(f'{type(value).__name__} is not JSON serializable')


def _send(message):
    # NaN, infinity and escaped lone surrogates (file names that are not UTF-8
    # decode to those on Linux) are not valid JSON for MITK. Encoding as UTF-8
    # turns a lone surrogate into '?' instead.
    text = json.dumps(message, separators=(',', ':'), default=_json_default, allow_nan=False, ensure_ascii=False)
    data = text.encode('utf-8', 'replace') + b'\n'

    if len(data) > _max_message_size:
        raise ValueError(f'the message has {len(data)} bytes, more than the {_max_message_size} that MITK accepts; '
                         'pass large data in buffers')

    with _send_lock:
        _protocol.write(data)
        _protocol.flush()


# Python's startup, e.g. the .pth files of the venv, may have written to the
# channel before stdout was redirected. MITK ignores everything up to here.
_send({'ready': True})


class _Call:
    def __init__(self, message):
        self.message = message
        self.id = message.get('id')
        self.cancel = threading.Event()
        self.dropped_progress = False


_calls = {}
_calls_lock = threading.Lock()
_current = None
_requests = queue.Queue()


def _read_requests():
    for line in mitk_input:
        try:
            message = json.loads(line)
        except ValueError:
            print(f'mitk_worker: ignoring malformed request {line[:200]!r}', file=sys.stderr)
            continue

        if 'cancel' in message:
            with _calls_lock:
                call = _calls.get(message['cancel'])
            if call is not None:
                call.cancel.set()
            continue

        call = _Call(message)

        with _calls_lock:
            _calls[call.id] = call

        _requests.put(call)

    # MITK closed the channel or is gone. On Linux and macOS, MITK makes the
    # worker the leader of a process group, which the processes it started
    # leave with it. On Windows, the job object of MITK ends them.
    if hasattr(os, 'killpg') and os.getpgrp() == os.getpid():
        os.killpg(0, signal.SIGKILL)

    os._exit(0)


def report_progress(**fields):
    """Sends progress of the current call to MITK.

    Returns False once MITK asked to cancel the call, so it fits callbacks that
    stop when they get False.
    """
    call = _current

    if call is None:
        return True

    # Runs inside callbacks of libraries, which must not fail because a field
    # is not JSON (e.g. NaN or a device object).
    try:
        _send({'id': call.id, 'progress': fields})
    except (TypeError, ValueError) as e:
        if not call.dropped_progress:
            call.dropped_progress = True
            print(f'mitk_worker: progress reports dropped: {e}', file=sys.stderr)

    return not call.cancel.is_set()


def is_cancel_requested():
    call = _current
    return call is not None and call.cancel.is_set()


_api = types.ModuleType('mitk_worker')
_api.report_progress = report_progress
_api.is_cancel_requested = is_cancel_requested
sys.modules['mitk_worker'] = _api


def _load_modules(modules):
    names = set()

    for module in modules:
        name = module['name']
        source = module['source']
        filename = f'{name}.py'

        # Lets tracebacks show the source lines.
        linecache.cache[filename] = (len(source), None, source.splitlines(True), filename)

        loaded = types.ModuleType(name)
        loaded.__file__ = filename
        sys.modules[name] = loaded
        exec(compile(source, filename, 'exec'), loaded.__dict__)
        names.add(name)

    return names


def _error(exception):
    return {
        'type': type(exception).__name__,
        'message': str(exception),
        'traceback': ''.join(traceback.format_exception(exception))
    }


try:
    _modules = _load_modules(mitk_modules)
    _load_error = None
except Exception as e:
    _modules = set()
    _load_error = _error(e)
    print(_load_error['traceback'], file=sys.stderr)

del mitk_modules


def _resolve(method):
    module, _, function = method.rpartition('.')

    if module not in _modules or not function or function.startswith('_'):
        raise AttributeError(f'{method} is not a function of the loaded modules')

    return getattr(sys.modules[module], function)


def _open_shared_memory(name):
    from multiprocessing import resource_tracker, shared_memory

    if sys.version_info >= (3, 13):
        return shared_memory.SharedMemory(name, track=False)

    # Before 3.13, attaching registers the segment with the resource tracker,
    # which would unlink it at exit. MITK owns every segment.
    register = resource_tracker.register
    resource_tracker.register = lambda *args, **kwargs: None

    try:
        return shared_memory.SharedMemory(name)
    finally:
        resource_tracker.register = register


# Segments whose memory is still referenced, e.g. by a tool that keeps the
# image of a session. They are closed once the last reference is gone.
_retained = []


def _attach(descriptors, segments):
    # Only buffers need NumPy, so calls without them work in an environment
    # that lacks it.
    if not descriptors:
        return {}

    import math
    import numpy as np

    arrays = {}

    for key, descriptor in descriptors.items():
        segment = _open_shared_memory(descriptor['name'])
        segments.append(segment)
        shape = tuple(descriptor['shape'])
        # Unlike np.ndarray(buffer=...), frombuffer holds an export of the
        # memory, which makes closing the segment fail while an array uses it.
        # The shape decides the size: segments are rounded up to whole pages.
        arrays[key] = np.frombuffer(segment.buf, dtype=descriptor['dtype'], count=math.prod(shape)).reshape(shape)

    return arrays


def _close_unreferenced(segments):
    retained = []

    for segment in segments:
        try:
            segment.close()
        except BufferError:
            retained.append(segment)

    return retained


def _invoke(message, segments):
    function = _resolve(message['method'])
    kwargs = dict(message.get('params') or {})
    kwargs.update(_attach(message.get('buffers') or {}, segments))
    return function(**kwargs)


def _run(call):
    global _retained

    if _load_error is not None:
        return {'id': call.id, 'error': _load_error}

    segments = []

    try:
        response = {'id': call.id, 'result': _invoke(call.message, segments)}
    except Exception as e:
        response = {'id': call.id, 'error': _error(e)}

    # The arrays of the call are gone with its frame unless the tool kept them.
    _retained = _close_unreferenced(_retained + segments)

    return response


threading.Thread(target=_read_requests, name='mitk_worker requests', daemon=True).start()

while True:
    _call = _requests.get()
    _current = _call
    _response = _run(_call)
    _current = None

    with _calls_lock:
        _calls.pop(_call.id, None)

    try:
        _send(_response)
    except (TypeError, ValueError) as e:
        _send({'id': _call.id, 'error': _error(e)})

    del _call, _response
