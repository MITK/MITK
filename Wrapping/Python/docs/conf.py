"""Sphinx configuration for the mitk Python wheel documentation."""

from __future__ import annotations

import importlib.metadata
import os

# -- Project information -----------------------------------------------------

project = "mitk"
copyright = "German Cancer Research Center (DKFZ)"  # noqa: A001
author = "MITK developers"


def _detect_release() -> str:
    """Resolve the project version.

    The CMake target passes ``MITK_VERSION`` via the environment when the
    docs are built from a build tree; that takes precedence. Otherwise
    fall back to ``importlib.metadata`` (works when ``mitk`` is pip-installed)
    and finally to ``"unknown"``.
    """
    env_version = os.environ.get("MITK_VERSION")
    if env_version:
        return env_version
    try:
        return importlib.metadata.version("mitk")
    except importlib.metadata.PackageNotFoundError:
        return "unknown"


release = _detect_release()
version = ".".join(release.split(".")[:2]) if release != "unknown" else "unknown"

# -- General configuration ---------------------------------------------------

extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.autosummary",
    "sphinx.ext.napoleon",
    "sphinx.ext.intersphinx",
    "sphinx_autodoc_typehints",
    "myst_parser",
    "sphinx_copybutton",
]

source_suffix = {
    ".md": "markdown",
    ".rst": "restructuredtext",
}

exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

# -- autodoc / autosummary ---------------------------------------------------

autosummary_generate = True
autodoc_default_options = {
    "members": True,
    "inherited-members": True,
    "show-inheritance": True,
    "undoc-members": False,
    # Operator and container dunders are part of the public surface of
    # several bindings (PropertyKeyPath uses ``/`` and ``[]``; PropertyView
    # is a MutableMapping; Color/Point/Vector index by integer; Image
    # exposes the NumPy array protocol). List them explicitly so autodoc
    # renders them; pure implementation dunders (__init__, __repr__, __str__,
    # __copy__, __deepcopy__, __hash__) are left to the defaults.
    "special-members": (
        "__getitem__,__setitem__,__delitem__,"
        "__contains__,__iter__,__len__,"
        "__truediv__,__array__,__eq__,__ne__"
    ),
}
autodoc_typehints = "description"
autodoc_typehints_format = "short"
always_use_bars_union = True

# -- napoleon (Google-style docstrings) --------------------------------------

napoleon_google_docstring = True
napoleon_numpy_docstring = False
napoleon_use_admonition_for_examples = True
napoleon_use_admonition_for_notes = True
napoleon_use_param = True
napoleon_use_rtype = True
# napoleon_include_init_with_doc is intentionally left at its default
# (False). Setting it True duplicates the __init__ entry because the
# autosummary class template already lists __init__ via :special-members:.

# -- MyST settings -----------------------------------------------------------

myst_enable_extensions = [
    "colon_fence",
    "deflist",
    "fieldlist",
    "linkify",
    "smartquotes",
    "substitution",
    "tasklist",
]
myst_heading_anchors = 3

# -- intersphinx -------------------------------------------------------------

intersphinx_mapping = {
    "python": ("https://docs.python.org/3", None),
    "numpy": ("https://numpy.org/doc/stable/", None),
}

# -- HTML output -------------------------------------------------------------

html_theme = "sphinx_book_theme"
html_static_path = ["_static"]
html_title = f"mitk {release}"
html_theme_options = {
    "show_toc_level": 2,
    "use_download_button": False,
    "use_fullscreen_button": False,
    "home_page_in_toc": True,
    "extra_footer": (
        '<p>This is the Python documentation for the '
        '<a href="https://docs.mitk.org/latest/">Medical Imaging Interaction '
        'Toolkit (MITK)</a>. For the full C++ reference, the developer manual, '
        'and the user manual, see '
        '<a href="https://docs.mitk.org/latest/">docs.mitk.org/latest/</a>.</p>'
    ),
}

# -- copybutton --------------------------------------------------------------

copybutton_prompt_text = r">>> |\.\.\. |\$ "
copybutton_prompt_is_regexp = True

# -- Sphinx event hooks ------------------------------------------------------


def _strip_namespace_in_docstring(app, what, name, obj, options, lines):
    """Rewrite ``mitk.mitk.*`` to ``mitk.*`` in autodoc docstring text.

    The compiled pybind11 extension lives at ``mitk.mitk`` (the package is
    ``mitk`` and the extension module inside is also named ``mitk``). The
    package ``__init__`` rewrites every class's ``__module__`` to ``"mitk"``
    but pybind11 bakes the original ``mitk.mitk.*`` paths into the
    auto-generated overload signatures it emits inside docstrings (the
    ``__init__(self: mitk.mitk.X) -> None`` lines that show up under
    "Overloaded function"). Those strings are not regenerated when
    ``__module__`` is rewritten, so fix them here at doc-build time.
    """
    for i, line in enumerate(lines):
        if "mitk.mitk." in line:
            lines[i] = line.replace("mitk.mitk.", "mitk.")


def _strip_namespace_in_signature(app, what, name, obj, options,
                                  signature, return_annotation):
    """Rewrite ``mitk.mitk.*`` to ``mitk.*`` in the parsed signature.

    Counterpart to :func:`_strip_namespace_in_docstring` for the signature
    line itself, which autodoc strips from ``__doc__`` before invoking
    ``autodoc-process-docstring``.
    """
    if signature and "mitk.mitk." in signature:
        signature = signature.replace("mitk.mitk.", "mitk.")
    if return_annotation and "mitk.mitk." in return_annotation:
        return_annotation = return_annotation.replace("mitk.mitk.", "mitk.")
    return signature, return_annotation


def setup(app):
    app.connect("autodoc-process-docstring", _strip_namespace_in_docstring)
    app.connect("autodoc-process-signature", _strip_namespace_in_signature)
