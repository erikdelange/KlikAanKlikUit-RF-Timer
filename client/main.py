import os

from PyQt5.QtWidgets import QApplication

import const
import ui


def main(argv=None):
    app = QApplication(argv)
    app.setApplicationName("Timer")

    # Create the main window.
    window = ui.MainWindow()
    window.show()

    # Run the application.
    return app.exec_()


if __name__ == "__main__":
    import sys

    if getattr(sys, "frozen", False):
        const.APPDIR = os.path.dirname(sys.executable)
    else:
        const.APPDIR = os.path.dirname(__file__)

    sys.exit(main(sys.argv))
