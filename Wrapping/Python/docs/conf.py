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
    fall back to ``importlib.metadata`` (works when ``mitk-python`` is pip-installed)
    and finally to ``"unknown"``.
    """
    env_version = os.environ.get("MITK_VERSION")
    if env_version:
        return env_version
    try:
        return importlib.metadata.version("mitk-python")
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
        '<a href="https://docs.mitk.org/2026.06/">Medical Imaging Interaction '
        'Toolkit (MITK)</a>. For the full C++ reference, the developer manual, '
        'and the user manual, see '
        '<a href="https://docs.mitk.org/2026.06/">docs.mitk.org/2026.06/</a>.</p>'
    ),
}

# -- copybutton --------------------------------------------------------------

copybutton_prompt_text = r">>> |\.\.\. |\$ "
copybutton_prompt_is_regexp = True

# -- Sphinx event hooks ------------------------------------------------------


import re as _re

# Strip pybind11's inner ``mitk.mitk.*`` path. The compiled extension
# lives at ``mitk.mitk`` (the package is ``mitk`` and the extension
# module inside is also named ``mitk``); the top-level ``mitk`` package
# rewrites every class's ``__module__`` to ``"mitk"`` at import time,
# but pybind11 bakes the original ``mitk.mitk.*`` paths into the
# auto-generated overload signatures emitted inside docstrings (the
# ``__init__(self: mitk.mitk.X) -> None`` lines that show up under
# "Overloaded function").
_PYBIND_NAMESPACE_RE = _re.compile(r"\bmitk\.mitk\.")

# Hide private submodule paths in pure-Python subpackages. ``mitk.mxn.layout``
# is structured as a public ``__init__.py`` that re-exports from underscored
# helper modules (``_model``, ``_builders``, ...); the dataclasses inside
# carry ``__module__`` values like ``mitk.mxn.layout._model``, which leaks
# into signatures and ``Raises:`` rendering. This pattern collapses any
# ``mitk.<pkg>._<priv>.`` reference to ``mitk.<pkg>.`` so the rendered docs
# match the listed autosummary path.
_PRIVATE_SUBMODULE_RE = _re.compile(r"\b(mitk(?:\.[A-Za-z0-9_]+)*)\._[A-Za-z0-9_]+\.")


def _normalise(text):
    text = _PYBIND_NAMESPACE_RE.sub("mitk.", text)
    # Loop because each pass only collapses one ``._<priv>.`` segment; a
    # hypothetical ``mitk._a.b._c.X`` path needs two passes to reach
    # ``mitk.b.X``. Today only one private segment is in play, but keep
    # the loop so we don't silently leak if that ever changes.
    while True:
        rewritten = _PRIVATE_SUBMODULE_RE.sub(r"\1.", text)
        if rewritten == text:
            break
        text = rewritten
    return text


_PRIVATE_PARAM_RE = _re.compile(r"^:(?:param|type)\s+_[A-Za-z]\w*\s*:")


def _strip_namespace_in_docstring(app, what, name, obj, options, lines):
    """Rewrite buried namespace paths in autodoc docstring text and drop
    ``:param _xxx:`` / ``:type _xxx:`` lines for private dataclass storage
    fields if any extension injects them at this stage.
    """
    i = 0
    while i < len(lines):
        line = lines[i]
        if _PRIVATE_PARAM_RE.match(line):
            del lines[i]
            continue
        normalised = _normalise(line)
        if normalised != line:
            lines[i] = normalised
        i += 1


def _strip_underscore_params(signature):
    """Drop single-underscore parameters (private dataclass fields) from a
    rendered signature string.

    The DSL dataclasses use a ``create()``-as-factory pattern: the dataclass
    ``__init__`` still accepts the private storage fields (``_groups``,
    ``_links``) so that ``dataclasses.replace`` works, but those parameters
    are an implementation detail and shouldn't appear in the user-facing
    signature. Walks the signature with balanced bracket-depth tracking so
    default values containing commas (``_links=(('selection', 'main'),)``)
    are handled correctly.
    """
    if not signature or "_" not in signature:
        return signature
    if not (signature.startswith("(") and signature.endswith(")")):
        return signature
    inner = signature[1:-1]
    parts = []
    depth = 0
    start = 0
    for i, ch in enumerate(inner):
        if ch in "([{":
            depth += 1
        elif ch in ")]}":
            depth -= 1
        elif ch == "," and depth == 0:
            parts.append(inner[start:i])
            start = i + 1
    parts.append(inner[start:])
    kept = []
    for part in parts:
        stripped = part.lstrip()
        if stripped.startswith("*") or not stripped:
            kept.append(part)
            continue
        head = stripped.split(":", 1)[0].split("=", 1)[0].strip()
        if head.startswith("_") and not head.startswith("__"):
            continue
        kept.append(part)
    return "(" + ",".join(kept) + ")"


def _strip_namespace_in_signature(app, what, name, obj, options,
                                  signature, return_annotation):
    """Counterpart to :func:`_strip_namespace_in_docstring` for the parsed
    signature line, which autodoc strips from ``__doc__`` before invoking
    ``autodoc-process-docstring``. Also filters out single-underscore
    parameters that the DSL dataclasses expose for ``replace()`` support
    but are not part of the documented user surface.
    """
    if signature:
        signature = _normalise(signature)
        signature = _strip_underscore_params(signature)
    if return_annotation:
        return_annotation = _normalise(return_annotation)
    return signature, return_annotation


def _normalise_reftitles(app, doctree, docname):
    """Rewrite ``reftitle`` attributes on resolved cross-references.

    Sphinx populates the HTML ``title="..."`` (hover tooltip) on each xref
    from the role's target name, which for autodoc-typehints comes
    straight from ``obj.__module__`` / ``obj.__qualname__``. The visible
    body is already cleaned by :func:`_strip_namespace_in_docstring` and
    :func:`_strip_namespace_in_signature`; this hook brings the hover
    tooltip into line.
    """
    from docutils import nodes

    for node in doctree.findall(nodes.reference):
        title = node.get("reftitle")
        if title:
            new_title = _normalise(title)
            if new_title != title:
                node["reftitle"] = new_title


def _drop_underscore_parameter_items(app, doctree, docname):
    """Strip ``_xxx`` entries out of every rendered "Parameters" list.

    autodoc's description-mode typehint handler injects a list item per
    field in ``__annotations__`` into the Parameters field_list of every
    documented object. That fires after ``autodoc-process-signature`` and
    ``autodoc-process-docstring``, so private dataclass storage fields
    (``_groups``, ``_links``) leak back in even though we have removed
    them from the signature. The doctree is the last point at which we
    can scrub them without touching the package source.
    """
    from docutils import nodes
    from sphinx import addnodes

    name_node_types = (addnodes.literal_strong, nodes.strong)

    for field in list(doctree.findall(nodes.field)):
        name_node = field.next_node(nodes.field_name)
        if name_node is None or name_node.astext() != "Parameters":
            continue
        for bullet_list in field.findall(nodes.bullet_list):
            for list_item in list(bullet_list.findall(nodes.list_item)):
                name_text_node = list_item.next_node(
                    condition=lambda n: isinstance(n, name_node_types)
                )
                if name_text_node is None:
                    continue
                param_name = name_text_node.astext().strip()
                if param_name.startswith("_") and not param_name.startswith("__"):
                    list_item.parent.remove(list_item)


def setup(app):
    app.connect("autodoc-process-docstring", _strip_namespace_in_docstring)
    # Priority < 500 (the default) so this fires before
    # ``sphinx_autodoc_typehints.process_signature``. The typehints handler
    # re-derives the signature from the live ``obj`` using
    # ``inspect.signature(obj)`` and emits the result via
    # ``emit_firstresult``; if we run after it, our return value is shadowed
    # by its non-None result and the underscore-parameter filtering is lost.
    app.connect("autodoc-process-signature", _strip_namespace_in_signature,
                priority=100)
    app.connect("doctree-resolved", _normalise_reftitles)
    app.connect("doctree-resolved", _drop_underscore_parameter_items)
