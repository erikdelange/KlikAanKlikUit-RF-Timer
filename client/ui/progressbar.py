from PyQt5.QtCore import Qt
from PyQt5.QtWidgets import QApplication, QDialog

import ui


class ProgressBar(QDialog):
    def __init__(self, windowtitle=None, parent=None):
        super().__init__(parent)

        ui.loadUi(__file__, self)

        self.setWindowFlags(Qt.Window | Qt.CustomizeWindowHint | Qt.WindowTitleHint)

        if windowtitle is not None:
            self.setWindowTitle(windowtitle)

        self.parent = parent
        self.value = 0

    def setMaximum(self, maximum=0):
        self.progressBar.setRange(0, maximum)

    def setValue(self, value):
        self.value = value
        self.progressBar.setValue(value)
        QApplication.instance().processEvents()

    def incrementValue(self):
        self.value += 1
        self.progressBar.setValue(self.value)
        QApplication.instance().processEvents()
