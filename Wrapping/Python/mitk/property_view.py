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

# property_view.py: PropertyView implementation for MITK
# Part of WP-15: Property Interface Bindings

from collections.abc import MutableMapping, Iterator
from typing import Any


class PropertyView(MutableMapping):
    """Live, lazy view over an object's properties.

    A ``MutableMapping`` that delegates every operation to the wrapped
    ``IPropertyOwner`` (typically an :py:class:`mitk.Image` or
    :py:class:`mitk.MultiLabelSegmentation`). It is not a snapshot: the
    view always reflects the current state of the underlying C++ object.

    Notes:
        Reading (``view[key]``, iteration, ``in``) works for all
        properties, both owned and provided. Writing (``view[key] =
        val``, ``del view[key]``) only works for *owned* properties and
        raises :py:class:`mitk.PropertyNotOwnedError` for read-only
        (provided but not owned) properties. Use
        ``obj.property_is_owned(key)`` to check before writing.

        ``PropertyView`` provides no synchronization. If properties are
        modified concurrently (for example from a C++ background
        thread), keys obtained during iteration may become stale before
        they are used. Callers that need a consistent snapshot should
        copy the view into a dict: ``snapshot = dict(view.items())``.

    Examples:
        >>> img.properties["DICOM.PatientName"] = "Doe^John"
        >>> "DICOM.PatientName" in img.properties
        True
        >>> for key, value in img.properties.items():
        ...     print(key, value)
    """

    __slots__ = ("_owner",)

    def __init__(self, owner: Any) -> None:
        """Wrap an object implementing the property-owner interface.

        Args:
            owner: An object exposing ``get_property``, ``set_property``,
                ``remove_property``, and ``property_keys``.
        """
        self._owner = owner

    def __getitem__(self, key: str) -> Any:
        """Return the coerced Python value for *key*.

        Args:
            key: Property name.

        Returns:
            ``str``, ``bool``, ``int``, ``float``, an ``(r, g, b)`` tuple
            for color properties, or a :py:class:`mitk.BaseProperty` for
            types without a known Python equivalent.

        Raises:
            KeyError: If *key* is not set on the underlying object.
        """
        prop = self._owner.get_property(key)
        if prop is None:
            raise KeyError(key)
        return prop

    def __setitem__(self, key: str, value: Any) -> None:
        """Set *key* to *value* on the underlying object.

        Args:
            key: Property name.
            value: Value to store. Auto-wrapped into the appropriate
                ``BaseProperty`` subtype.

        Raises:
            PropertyNotOwnedError: If the property is provided read-only
                (not owned) by the underlying object.
        """
        self._owner.set_property(key, value)

    def __delitem__(self, key: str) -> None:
        """Remove *key* from the underlying object.

        Args:
            key: Property name.

        Raises:
            KeyError: If *key* is not set.
            PropertyNotOwnedError: If the property is provided read-only.
        """
        if self._owner.get_property(key) is None:
            raise KeyError(key)
        self._owner.remove_property(key)

    def __contains__(self, key: object) -> bool:
        if not isinstance(key, str):
            return False
        return self._owner.get_property(key) is not None

    def __iter__(self) -> Iterator[str]:
        return iter(self._owner.property_keys)

    def __len__(self) -> int:
        return len(self._owner.property_keys)

    def __repr__(self) -> str:
        keys = self._owner.property_keys
        if len(keys) <= 5:
            return f"PropertyView({{{', '.join(repr(k) for k in keys)}}})"
        return f"PropertyView({len(keys)} properties)"
