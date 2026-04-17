"""Thin Python layer for MultiLabelSegmentation input normalization and
type-dispatched transfer_labels wrappers.

This module is imported by __init__.py and its _patch() function is called
to monkey-patch the C++ MultiLabelSegmentation class with normalizing
wrappers.
"""


def _normalize_value(v):
    """Convert int or Label to int."""
    if isinstance(v, int):
        return v
    return v.value


def _normalize_values(seq):
    """Convert iterable of int|Label to list of int."""
    return [_normalize_value(v) for v in seq]


def _normalize_mapping(pairs):
    """Convert iterable of (int|Label, int|Label) to list of (int, int)."""
    return [(_normalize_value(a), _normalize_value(b)) for a, b in pairs]


def _normalize_style(s, enum_cls):
    """Accept string aliases for MergeStyle / OverwriteStyle enums."""
    if isinstance(s, enum_cls):
        return s
    if isinstance(s, str):
        return getattr(enum_cls, s.upper())
    return s


def transfer_labels(source, destination, label_mapping=None,
                    merge_style=None, overwrite_style=None, **kwargs):
    """Type-dispatched transfer_labels wrapper.

    Dispatches to the segmentation or image variant based on
    ``isinstance(source, MultiLabelSegmentation)``.
    """
    import mitk

    if label_mapping is None:
        raise ValueError(
            "label_mapping is required; pass a list of (source_value, destination_value) tuples")
    label_mapping = _normalize_mapping(label_mapping)

    if merge_style is None:
        merge_style = mitk.MergeStyle.REPLACE
    else:
        merge_style = _normalize_style(merge_style, mitk.MergeStyle)

    if overwrite_style is None:
        overwrite_style = mitk.OverwriteStyle.REGARD_LOCKS
    else:
        overwrite_style = _normalize_style(overwrite_style, mitk.OverwriteStyle)

    if isinstance(source, mitk.MultiLabelSegmentation):
        mitk.mitk._transfer_labels_seg(
            source, destination,
            label_mapping=label_mapping,
            merge_style=merge_style,
            overwrite_style=overwrite_style)
    else:
        destination_labels = kwargs.get("destination_labels", [])
        source_background = kwargs.get("source_background", mitk.MultiLabelSegmentation.UNLABELED_VALUE)
        destination_background = kwargs.get("destination_background", mitk.MultiLabelSegmentation.UNLABELED_VALUE)
        destination_background_locked = kwargs.get(
            "destination_background_locked", False)
        mitk.mitk._transfer_labels_image(
            source, destination,
            destination_labels=destination_labels,
            label_mapping=label_mapping,
            source_background=source_background,
            destination_background=destination_background,
            destination_background_locked=destination_background_locked,
            merge_style=merge_style,
            overwrite_style=overwrite_style)


def transfer_labels_at_time_step(source, destination, time_step,
                                 label_mapping=None,
                                 merge_style=None, overwrite_style=None,
                                 **kwargs):
    """Type-dispatched transfer_labels_at_time_step wrapper."""
    import mitk

    if label_mapping is None:
        raise ValueError(
            "label_mapping is required; pass a list of (source_value, destination_value) tuples")
    label_mapping = _normalize_mapping(label_mapping)

    if merge_style is None:
        merge_style = mitk.MergeStyle.REPLACE
    else:
        merge_style = _normalize_style(merge_style, mitk.MergeStyle)

    if overwrite_style is None:
        overwrite_style = mitk.OverwriteStyle.REGARD_LOCKS
    else:
        overwrite_style = _normalize_style(overwrite_style, mitk.OverwriteStyle)

    if isinstance(source, mitk.MultiLabelSegmentation):
        mitk.mitk._transfer_labels_at_time_step_seg(
            source, destination,
            time_step=time_step,
            label_mapping=label_mapping,
            merge_style=merge_style,
            overwrite_style=overwrite_style)
    else:
        destination_labels = kwargs.get("destination_labels", [])
        source_background = kwargs.get("source_background", mitk.MultiLabelSegmentation.UNLABELED_VALUE)
        destination_background = kwargs.get("destination_background", mitk.MultiLabelSegmentation.UNLABELED_VALUE)
        destination_background_locked = kwargs.get(
            "destination_background_locked", False)
        mitk.mitk._transfer_labels_at_time_step_image(
            source, destination,
            destination_labels=destination_labels,
            time_step=time_step,
            label_mapping=label_mapping,
            source_background=source_background,
            destination_background=destination_background,
            destination_background_locked=destination_background_locked,
            merge_style=merge_style,
            overwrite_style=overwrite_style)


def _patch(MultiLabelSegmentation):
    """Monkey-patch C++ class so methods auto-normalize int|Label inputs."""
    _methods_single = [
        "remove_label", "erase_label",
    ]
    _methods_multi = [
        "remove_labels", "erase_labels",
    ]

    for name in _methods_single:
        orig = getattr(MultiLabelSegmentation, name)
        def _make_wrapper(fn):
            def wrapper(self, value, *args, **kwargs):
                return fn(self, _normalize_value(value), *args, **kwargs)
            wrapper.__name__ = fn.__name__
            wrapper.__doc__ = fn.__doc__
            return wrapper
        setattr(MultiLabelSegmentation, name, _make_wrapper(orig))

    for name in _methods_multi:
        orig = getattr(MultiLabelSegmentation, name)
        def _make_wrapper(fn):
            def wrapper(self, values, *args, **kwargs):
                return fn(self, _normalize_values(values), *args, **kwargs)
            wrapper.__name__ = fn.__name__
            wrapper.__doc__ = fn.__doc__
            return wrapper
        setattr(MultiLabelSegmentation, name, _make_wrapper(orig))

    # rename_label: (value, name, color)
    _orig_rename = MultiLabelSegmentation.rename_label
    def _rename_wrapper(self, value, name, color):
        return _orig_rename(self, _normalize_value(value), name, color)
    _rename_wrapper.__name__ = _orig_rename.__name__
    MultiLabelSegmentation.rename_label = _rename_wrapper

    # merge_labels: (target, sources, overwrite_style)
    _orig_merge = MultiLabelSegmentation.merge_labels
    def _merge_wrapper(self, target, sources, overwrite_style=None):
        import mitk
        if overwrite_style is None:
            overwrite_style = mitk.OverwriteStyle.REGARD_LOCKS
        else:
            overwrite_style = _normalize_style(overwrite_style, mitk.OverwriteStyle)
        return _orig_merge(
            self, _normalize_value(target),
            _normalize_values(sources),
            overwrite_style=overwrite_style)
    _merge_wrapper.__name__ = _orig_merge.__name__
    MultiLabelSegmentation.merge_labels = _merge_wrapper

    # get_labels: (values)
    _orig_get_labels = MultiLabelSegmentation.get_labels
    def _get_labels_wrapper(self, values):
        return _orig_get_labels(self, _normalize_values(values))
    _get_labels_wrapper.__name__ = _orig_get_labels.__name__
    MultiLabelSegmentation.get_labels = _get_labels_wrapper

    # split_labels_by_group: (labels)
    _orig_split_group = MultiLabelSegmentation.split_labels_by_group
    def _split_group_wrapper(self, labels):
        return _orig_split_group(self, _normalize_values(labels))
    _split_group_wrapper.__name__ = _orig_split_group.__name__
    MultiLabelSegmentation.split_labels_by_group = _split_group_wrapper

    # split_labels_by_class_name: (group, labels=None)
    _orig_split_class = MultiLabelSegmentation.split_labels_by_class_name
    def _split_class_wrapper(self, group, labels=None):
        if labels is not None:
            labels = _normalize_values(labels)
        return _orig_split_class(self, group, labels)
    _split_class_wrapper.__name__ = _orig_split_class.__name__
    MultiLabelSegmentation.split_labels_by_class_name = _split_class_wrapper

    # relabel_to: (label_mapping, *, dest_seg=None, keep_untouched_labels=False,
    #              merge_style=None, overwrite_style=None)
    def _relabel_to(self, label_mapping, *, dest_seg=None,
                    keep_untouched_labels=False,
                    merge_style=None, overwrite_style=None):
        """Return a segmentation whose pixel content is the result of applying
        *label_mapping* to *self*.

        Parameters
        ----------
        label_mapping:
            Iterable of ``(source_value, destination_value)`` pairs.  Each
            entry is a ``(int|Label, int|Label)`` 2-tuple.
        dest_seg:
            Target ``MultiLabelSegmentation``.  If *None* (default), *self* is
            cloned and used as the target (see *keep_untouched_labels* for how
            the clone is prepared).  All destination label values referenced in
            *label_mapping* must already exist in *dest_seg*.
        keep_untouched_labels:
            Only valid when *dest_seg* is *None*.  Controls what happens to
            labels that are **not** listed as a source in *label_mapping*:

            * ``False`` (default) — the clone's group images are cleared before
              the transfer.  Only pixels of mapped labels appear in the result.
            * ``True`` — the clone retains its original pixel content.  Mapped
              labels are remapped in-place; every other label is left untouched.
              Raises ``ValueError`` when combined with an explicit *dest_seg*
              because the caller already controls the state of that object.
        merge_style:
            ``MergeStyle`` enum value or lowercase string alias
            (``"replace"`` / ``"merge"``).  Defaults to ``REPLACE``.
        overwrite_style:
            ``OverwriteStyle`` enum value or lowercase string alias
            (``"regard_locks"`` / ``"ignore_locks"``).  Defaults to
            ``IGNORE_LOCKS``.

        Returns
        -------
        MultiLabelSegmentation
            The populated *dest_seg* (or the auto-created clone when
            *dest_seg* was *None*).
        """
        import mitk

        if keep_untouched_labels and dest_seg is not None:
            raise ValueError(
                "keep_untouched_labels=True is only allowed when dest_seg is None. "
                "When an explicit dest_seg is provided the caller controls its "
                "content; clear it manually beforehand if a clean slate is needed."
            )

        mapping = _normalize_mapping(label_mapping)

        if merge_style is None:
            merge_style = mitk.MergeStyle.REPLACE
        else:
            merge_style = _normalize_style(merge_style, mitk.MergeStyle)

        if overwrite_style is None:
            overwrite_style = mitk.OverwriteStyle.IGNORE_LOCKS
        else:
            overwrite_style = _normalize_style(overwrite_style, mitk.OverwriteStyle)

        if dest_seg is None:
            dest_seg = self.clone()
            if not keep_untouched_labels:
                dest_seg.clear_group_images()

        mitk.mitk._transfer_labels_seg(
            self, dest_seg,
            label_mapping=mapping,
            merge_style=merge_style,
            overwrite_style=overwrite_style,
        )
        return dest_seg

    MultiLabelSegmentation.relabel_to = _relabel_to
