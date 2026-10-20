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

# Functions called by mitkPythonWorkerTest.

import os
import sys
import time

import mitk_worker

_kept = None


def echo(**kwargs):
    return kwargs


def fail(message):
    raise ValueError(message)


def not_a_number():
    return {'score': float('nan')}


def pollute():
    print('print() must not reach the protocol channel')
    os.write(1, b'neither must a raw write to descriptor 1\n')
    sys.__stdout__.write('nor sys.__stdout__\n')
    sys.__stdout__.flush()
    if sys.platform == 'win32':
        import ctypes
        STD_OUTPUT_HANDLE = -11
        kernel32 = ctypes.windll.kernel32
        kernel32.GetStdHandle.restype = ctypes.c_void_p
        message = b'nor a native write to the standard output handle\n'
        written = ctypes.c_ulong()
        kernel32.WriteFile(ctypes.c_void_p(kernel32.GetStdHandle(STD_OUTPUT_HANDLE)), message, len(message),
                           ctypes.byref(written), None)
    return 'clean'


def describe(array):
    return {'dtype': array.dtype.str[1:], 'shape': list(array.shape), 'sum': float(array.sum())}


def fill(target, value):
    target[...] = value


def keep(array):
    global _kept
    _kept = array


def write_kept(value):
    _kept[...] = value


def sum_kept():
    return float(_kept.sum())


def release_kept():
    global _kept
    _kept = None


def count(total, delay):
    for done in range(1, total + 1):
        time.sleep(delay)
        if not mitk_worker.report_progress(done=done, total=total):
            return {'cancelled_at': done}
    return {'completed': total}


def ignore_cancel(steps):
    # A cancellation is a request; a function may finish anyway.
    for done in range(1, steps + 1):
        time.sleep(0.05)
        mitk_worker.report_progress(done=done, total=steps)
    return 'done'


def _no_window():
    # The worker itself has no console, so a child would open a window.
    import subprocess
    return subprocess.CREATE_NO_WINDOW if sys.platform == 'win32' else 0


def orphan_exits(runtime_path):
    """Starts a worker the way MITK does and closes its input, as a crash of
    MITK would. Returns its exit code, or None if it did not exit."""
    import json
    import subprocess

    # The bootstrap of mitk::PythonWorker, which keeps it private.
    bootstrap = ("import json,os;"
                 "i=os.fdopen(os.dup(0),'rb');"
                 "m=json.loads(i.readline());"
                 "exec(compile(m['runtime'],'mitk_worker.py','exec'),"
                 "{'__name__':'__main__','mitk_input':i,'mitk_modules':m['modules'],"
                 "'mitk_max_message_size':m['max_message_size']})")

    with open(runtime_path, encoding='utf-8') as file:
        runtime = file.read()

    child = subprocess.Popen([sys.executable, '-I', '-c', bootstrap], stdin=subprocess.PIPE,
                             stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, creationflags=_no_window())
    startup = {'runtime': runtime, 'modules': [], 'max_message_size': 1024}
    child.stdin.write(json.dumps(startup).encode('utf-8') + b'\n')
    child.stdin.close()

    try:
        return child.wait(timeout=10)
    except subprocess.TimeoutExpired:
        child.kill()
        return None


def get_pid():
    return os.getpid()


def spawn_sleeper():
    """Starts a child process that sleeps for a minute and returns its process ID."""
    import subprocess

    # Like multiprocessing: in a virtual environment on Windows, sys.executable
    # is a launcher that would start the interpreter as a child of its own.
    python = getattr(sys, '_base_executable', sys.executable)

    child = subprocess.Popen([python, '-c', 'import time; time.sleep(60)'], stdin=subprocess.DEVNULL,
                             stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, creationflags=_no_window())
    return child.pid


def create_venv(path):
    import subprocess
    subprocess.run([sys.executable, '-m', 'venv', '--without-pip', path], check=True, creationflags=_no_window())


def run_and_kill_host(host, python, module):
    """Starts the test host, which keeps a worker with a child of its own, and
    kills it as a crash would end MITK. Returns the process IDs of the worker
    and of its child."""
    import subprocess

    process = subprocess.Popen([host, python, module], stdin=subprocess.DEVNULL, stdout=subprocess.PIPE,
                               creationflags=_no_window())
    try:
        line = process.stdout.readline()
    finally:
        process.kill()
        process.wait()

    if not line:
        raise RuntimeError('The test host did not report its worker.')

    worker, child = (int(pid) for pid in line.split())
    return {'worker': worker, 'child': child}


def exit(code):
    os._exit(code)


def _skip_crash_reporting():
    # A crash handler like apport takes about a second per crash and leaves a
    # report behind. Neither applies to a process that is not dumpable.
    if sys.platform.startswith('linux'):
        import ctypes

        PR_SET_DUMPABLE = 4
        ctypes.CDLL(None).prctl(PR_SET_DUMPABLE, 0, 0, 0, 0)


def crash():
    import faulthandler

    _skip_crash_reporting()
    faulthandler._sigsegv()


def access_violation():
    # A real crash, unlike faulthandler._sigsegv(), which suppresses the crash
    # reporting of Windows itself.
    import ctypes

    _skip_crash_reporting()

    if sys.platform == 'win32':
        # ctypes turns a crash inside a call into an OSError. A native thread
        # that starts at address 0 crashes outside of any call.
        ctypes.windll.kernel32.CreateThread(None, 0, None, None, 0, None)
        time.sleep(30)
    else:
        ctypes.string_at(0)


def lone_surrogate():
    # What os.listdir() returns on Linux for a file name that is not UTF-8.
    return 'a\udcffb'


def large_result(size):
    return 'x' * size


def write_oversized(size):
    # A broken runtime, which writes without the check of _send(). MITK must
    # end it instead of collecting the message.
    protocol = mitk_worker.report_progress.__globals__['_protocol']
    protocol.write(b'x' * size)
    protocol.flush()
    time.sleep(30)


def write_endless_error_line(size):
    # Like a progress bar, which redraws its line with carriage returns only.
    for _ in range(size // 100):
        sys.stderr.write('\r' + 'x' * 99)
    sys.stderr.write('\n')
    sys.stderr.flush()
    return 'done'


def write_unreadable():
    # Only the runtime writes to the protocol channel. A broken line has to
    # end the worker instead of leaving the call waiting.
    protocol = mitk_worker.report_progress.__globals__['_protocol']
    protocol.write(b'not json\n')
    protocol.flush()
    time.sleep(30)


def fork_and_exit():
    # The child keeps a copy of every descriptor of the worker.
    if os.fork() == 0:
        time.sleep(30)
        os._exit(0)
    os._exit(4)


def hog_interpreter_later():
    import re
    import threading

    def hog():
        time.sleep(0.2)
        # Backtracking in C, which never releases the GIL, so the reader thread
        # of the runtime cannot react to the end of its input any more.
        re.match('(a*)*b', 'a' * 40)

    threading.Thread(target=hog, daemon=True).start()
