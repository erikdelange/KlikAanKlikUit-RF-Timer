""" Loader for .ui files created in Qt Designer. """

import os.path
import sys

from PyQt5 import uic

import const


def loadUi(module_path, widget, basename=None):
    """
    Load the ui file associated with the given module path and widget class.
    For example: if the widget's class is called MyWidget then ui file
    mywidget.ui will be loaded. The optional parameter basename overrides
    the derivation of the ui filename from the class name.

    Usage: loadUi(__file__, self)

    :param      module_path | str
    :param      widget      | QWidget
    :param      basename    | str optional
    :return     none
    """
    if getattr(sys, 'frozen', False):
        # If script is frozen read the ui files from a directory named 'ui'.
        # BEWARE: this directory-name is hard-coded!
        # Do not forget to copy all *.ui files to 'ui' in the build directory.
        basepath = os.path.join(const.APP_DIR, "ui")
    else:
        basepath = os.path.dirname(module_path)

    if basename is None:
        basename = widget.__class__.__name__.lower()

    uifile = os.path.join(basepath, "ui_%s.ui" % basename)

    uic.loadUi(uifile, widget)
