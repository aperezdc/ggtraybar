#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim:fenc=utf-8
#
# Copyright © 2014 Adrian Perez <aperez@igalia.com>
#
# Distributed under terms of the MIT license.

pkgs = (
    "glib-2.0",
    "gtk+-3.0",
    "gtk+-x11-3.0",
    "libwnck-3.0",
    "keybinder-3.0",
    "x11",
    "libbamf3",
    "gio-2.0",
)

from subprocess import check_output
from shlex import split as sh_split

def pkg_config_flags(*items):
    command = ["pkg-config"]
    [command.append(i) for i in items]
    command.append("--cflags")
    return sh_split(check_output(command))

def FlagsForFile(path, **kwarg):
    return { 'flags': pkg_config_flags(*pkgs), 'do_cache': True }
