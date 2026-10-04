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

# Fills the cache of the Hugging Face Hub with what a library would otherwise
# download on first use, where it could not tell how far it is. Files are
# downloaded one after the other and the progress is reported to MITK in bytes
# (see mitk::PostInstallStepProgress). Nothing is reported if all files are
# cached already.

import inspect
import os
import threading
import time


def _mitk_remove_stale_partial_files(repo_ids):
    # huggingface_hub does not resume a download. A download that is stopped
    # without a chance to clean up, the way MITK stops one that the user
    # cancels, leaves its partial file behind for good. A file that has not
    # been written to for a while is such a leftover, whereas one of a download
    # running elsewhere is not.
    try:
        from huggingface_hub import constants
        from huggingface_hub.file_download import repo_folder_name
    except ImportError:
        return

    for repo_id in repo_ids:
        blobs = os.path.join(constants.HF_HUB_CACHE, repo_folder_name(repo_id=repo_id, repo_type='model'), 'blobs')

        try:
            names = os.listdir(blobs)
        except OSError:
            continue

        for name in names:
            path = os.path.join(blobs, name)

            try:
                if name.endswith('.incomplete') and time.time() - os.path.getmtime(path) > 60:
                    os.remove(path)
            except OSError:
                pass


def mitk_download_from_hub(targets):
    """Downloads the files of the given repositories that are not cached yet.

    targets is a list of (repo_id, allow_patterns) pairs, with the patterns as
    snapshot_download() of huggingface_hub takes them.
    """
    from huggingface_hub import hf_hub_download, snapshot_download
    from huggingface_hub.utils import disable_progress_bars
    from tqdm import tqdm

    # Its own progress bars would only clutter the log.
    disable_progress_bars()

    if 'dry_run' not in inspect.signature(snapshot_download).parameters or \
       'tqdm_class' not in inspect.signature(hf_hub_download).parameters:
        # Too old to tell the sizes up front or to report the bytes of a file.
        print('MITK_PROGRESS 0 0', flush=True)

        for repo_id, patterns in targets:
            snapshot_download(repo_id, allow_patterns=patterns)

        return

    files = []

    for repo_id, patterns in targets:
        try:
            infos = snapshot_download(repo_id, allow_patterns=patterns, dry_run=True)
        except Exception as error:
            # Without network access, what is cached already has to do. If
            # nothing is, the error that brought us here is the one to report.
            try:
                snapshot_download(repo_id, allow_patterns=patterns, local_files_only=True)
            except Exception:
                raise error

            continue

        files += [(repo_id, info.filename, info.file_size or 0) for info in infos if info.will_download]

    if not files:
        return

    _mitk_remove_stale_partial_files({repo_id for repo_id, _, _ in files})

    total = sum(size for _, _, size in files)
    lock = threading.Lock()
    bars = []
    completed = 0
    size_in_flight = 0

    class Bar(tqdm):
        # Disabled, tqdm stops counting as well, so the bar counts on its own.
        # Depending on how a file is transferred, there are several bars per
        # file, each of which counts all of its bytes.
        def __init__(self, *args, **kwargs):
            kwargs['disable'] = True
            super().__init__(*args, **kwargs)
            self.mitk_count = kwargs.get('initial') or 0

            with lock:
                bars.append(self)

        def update(self, n=1):
            if n:
                self.mitk_count += n

            return super().update(n)

    def progress():
        with lock:
            in_flight = max((bar.mitk_count for bar in bars), default=0)
            return completed + min(in_flight, size_in_flight)

    stopped = threading.Event()

    def report_until_stopped():
        reported = 0

        while not stopped.wait(0.25):
            done = progress()

            if done != reported:
                print(f'MITK_PROGRESS {done} {total}', flush=True)
                reported = done

    print(f'MITK_PROGRESS 0 {total}', flush=True)

    reporter = threading.Thread(target=report_until_stopped, daemon=True)
    reporter.start()

    try:
        for repo_id, filename, size in files:
            with lock:
                bars.clear()
                size_in_flight = size

            hf_hub_download(repo_id, filename, tqdm_class=Bar)

            with lock:
                bars.clear()
                size_in_flight = 0
                completed += size
    finally:
        stopped.set()
        reporter.join()

    print(f'MITK_PROGRESS {total} {total}', flush=True)
