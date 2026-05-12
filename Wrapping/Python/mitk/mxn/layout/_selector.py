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

"""``MxNWindowSelector`` -- pandas-style filter + bulk transform over a layout tree.

A selector is bound to a *source* tree (``MxNLayoutDocument`` or ``Split``).
Filters return a new selector; terminals return either a value (read) or a
new instance of the source type (update). All immutable -- the source tree
is never mutated.

The selector is local-only: it knows nothing about HTTP, transports, or
workbench state. It operates on layout *documents* as values.
"""

from __future__ import annotations

from collections.abc import Callable, Iterable, Iterator
from dataclasses import replace
from typing import Generic, TypeVar, Union

from . import _validation as _v
from ._model import (
    Group,
    LayoutWindow,
    LinkLike,
    MxNLayoutDocument,
    Split,
    _normalise_link_value,
)
from ._vocabulary import LinkDimension, ViewDirection


_Source = TypeVar("_Source", MxNLayoutDocument, Split)


def _coerce_view_direction(value: ViewDirection | str) -> ViewDirection:
    if isinstance(value, ViewDirection):
        return value
    if isinstance(value, str):
        return ViewDirection(value)
    raise ValueError(f"view direction must be ViewDirection or str, got {type(value).__name__}")


def _root_of(source: _Source) -> Split:
    return source.root if isinstance(source, MxNLayoutDocument) else source


def _rewrap(source: _Source, new_root: Split) -> _Source:
    """Rebuild the source with a new root, preserving document metadata."""
    if isinstance(source, MxNLayoutDocument):
        return replace(source, root=new_root)  # type: ignore[return-value]
    return new_root  # type: ignore[return-value]


def _rewrite_tree(
    node: Split | LayoutWindow,
    transform: Callable[[LayoutWindow], LayoutWindow],
    keep: Callable[[LayoutWindow], bool],
) -> Split | LayoutWindow:
    """Walk the tree and replace each leaf for which ``keep`` returns True
    with ``transform(leaf)``; pass through other leaves. Splits are
    rebuilt structurally (size/orientation preserved).
    """
    if isinstance(node, LayoutWindow):
        return transform(node) if keep(node) else node
    new_children = tuple(_rewrite_tree(c, transform, keep) for c in node.children)
    if new_children == node.children:
        return node
    return replace(node, children=new_children)


def _ensure_group(source: _Source, group_name: str) -> _Source:
    """When the source is a document, ensure ``group_name`` is in its
    groups registry; on ``Split`` sources this is a no-op (groups live on
    the document).
    """
    if not isinstance(source, MxNLayoutDocument):
        return source
    if group_name in source.groups:
        return source
    new_groups = dict(source.groups)
    new_groups[group_name] = Group(group_name)
    new_storage = tuple(sorted(new_groups.items()))
    return replace(source, _groups=new_storage)  # type: ignore[return-value]


class MxNWindowSelector(Generic[_Source]):
    """Filter + bulk-transform over a tree's windows.

    Each filter call returns a new selector; each terminal call returns
    a value (read) or a new ``T`` (update). All operations are immutable.
    """

    __slots__ = ("_source", "_predicate")

    def __init__(
        self,
        source: _Source,
        predicate: Callable[[LayoutWindow], bool] | None = None,
    ) -> None:
        self._source = source
        self._predicate: Callable[[LayoutWindow], bool] = predicate or (lambda _w: True)

    # ---- internals ------------------------------------------------------

    def _and(
        self, extra: Callable[[LayoutWindow], bool]
    ) -> "MxNWindowSelector[_Source]":
        prior = self._predicate
        return MxNWindowSelector(self._source, lambda w: prior(w) and extra(w))

    def _selected_windows(self) -> list[LayoutWindow]:
        return [w for w in _root_of(self._source).windows() if self._predicate(w)]

    # ---- filtering ------------------------------------------------------

    def where(
        self,
        *,
        view_direction: ViewDirection | str | None = None,
        ids: Iterable[str] | None = None,
        display_names: Iterable[str | None] | None = None,
        predicate: Callable[[LayoutWindow], bool] | None = None,
        group: str | Group | None = None,
        dim: LinkDimension | str = LinkDimension.SELECTION,
    ) -> "MxNWindowSelector[_Source]":
        """Compose multiple filter predicates AND-style.

        All keyword arguments are optional; the resulting selector keeps
        only windows that satisfy every supplied predicate.
        """
        result: MxNWindowSelector[_Source] = self
        if view_direction is not None:
            target_view = _coerce_view_direction(view_direction)
            result = result._and(lambda w, v=target_view: w.view_direction == v)
        if ids is not None:
            id_set = set(ids)
            result = result._and(lambda w, s=id_set: w.id in s)
        if display_names is not None:
            name_set = set(display_names)
            result = result._and(lambda w, s=name_set: w.name in s)
        if group is not None:
            target_group = group.name if isinstance(group, Group) else group
            dim_key = _v.validate_dimension_key(dim)
            result = result._and(
                lambda w, g=target_group, d=dim_key: w.linked_to(g, dim=d)
            )
        if predicate is not None:
            result = result._and(predicate)
        return result

    def by_id(self, *ids: str) -> "MxNWindowSelector[_Source]":
        """Keep only windows whose ids match any of ``ids``."""
        return self.where(ids=ids)

    def by_display_name(
        self, *names: str | None
    ) -> "MxNWindowSelector[_Source]":
        """Keep only windows whose display names match any of ``names``.

        ``None`` matches windows that have no display label set.
        """
        return self.where(display_names=names)

    def by_view(
        self, *view_directions: ViewDirection | str
    ) -> "MxNWindowSelector[_Source]":
        """Keep only windows in any of the supplied view directions."""
        coerced = [_coerce_view_direction(v) for v in view_directions]
        coerced_set = set(coerced)
        return self._and(lambda w, s=coerced_set: w.view_direction in s)

    # ---- terminals: read ------------------------------------------------

    def to_list(self) -> list[LayoutWindow]:
        return self._selected_windows()

    def ids(self) -> list[str]:
        return [w.id for w in self._selected_windows()]

    def __iter__(self) -> Iterator[LayoutWindow]:
        return iter(self._selected_windows())

    def __len__(self) -> int:
        return len(self._selected_windows())

    # ---- terminals: bulk update ----------------------------------------

    def link_to(
        self,
        link: LinkLike,
        *,
        dim: LinkDimension | str = LinkDimension.SELECTION,
    ) -> _Source:
        """Re-link every selected window to ``link`` on dimension ``dim``.

        On a document source, materialises a default ``Group`` entry for
        a previously-unknown group name so strict-mode invariants hold on
        the returned document. On a ``Split`` source there is no group
        registry to update.
        """
        new_value = _normalise_link_value(link)
        dim_key = _v.validate_dimension_key(dim)

        ids = {w.id for w in self._selected_windows()}

        def transform(window: LayoutWindow) -> LayoutWindow:
            return window.with_link(dim_key, new_value)

        new_root = _rewrite_tree(_root_of(self._source), transform, lambda w: w.id in ids)
        assert isinstance(new_root, Split)
        new_source = _rewrap(self._source, new_root)
        return _ensure_group(new_source, new_value)

    def with_view_direction(self, vd: ViewDirection | str) -> _Source:
        target = _coerce_view_direction(vd)
        ids = {w.id for w in self._selected_windows()}

        def transform(window: LayoutWindow) -> LayoutWindow:
            return replace(window, view_direction=target)

        new_root = _rewrite_tree(_root_of(self._source), transform, lambda w: w.id in ids)
        assert isinstance(new_root, Split)
        return _rewrap(self._source, new_root)

    def with_display_name(
        self,
        name: Union[str, None, Callable[[LayoutWindow], str | None]],
    ) -> _Source:
        ids = {w.id for w in self._selected_windows()}

        if callable(name):
            fn = name

            def transform(window: LayoutWindow) -> LayoutWindow:
                return window.with_name(fn(window))

        else:
            value: str | None = name

            def transform(window: LayoutWindow) -> LayoutWindow:
                return window.with_name(value)

        new_root = _rewrite_tree(_root_of(self._source), transform, lambda w: w.id in ids)
        assert isinstance(new_root, Split)
        return _rewrap(self._source, new_root)

    def map(
        self, fn: Callable[[LayoutWindow], LayoutWindow]
    ) -> _Source:
        """Escape hatch -- replace each selected window with ``fn(window)``.

        The selector preserves no invariants beyond what
        ``MxNLayoutDocument.create`` enforces on construction; if the
        caller introduces a duplicate id, ``MxNLayoutDocument.validate()``
        will catch it on the next ``validate()`` call (or implicitly via
        ``MxNLayoutDocument.create`` if the result is rebuilt).
        """
        ids = {w.id for w in self._selected_windows()}
        new_root = _rewrite_tree(_root_of(self._source), fn, lambda w: w.id in ids)
        assert isinstance(new_root, Split)
        return _rewrap(self._source, new_root)
