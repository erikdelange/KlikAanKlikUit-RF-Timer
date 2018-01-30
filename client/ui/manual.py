from PyQt5.QtCore import Qt, pyqtSlot
from PyQt5.QtWidgets import QDialog

import device
import const
import ui


class Manual(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)

        ui.loadUi(__file__, self)

        self.setWindowFlags(Qt.Window | Qt.WindowTitleHint | Qt.CustomizeWindowHint)
        self.parent = parent

        self.lstMajor.addItems(const.MAJORNAMES)
        self.lstMinor.addItems(const.MINORNAMES)
        self.lstCommand.addItems(const.COMMANDNAMES)

        self.lstMajor.setCurrentRow(0)
        self.lstMinor.setCurrentRow(0)
        self.lstCommand.setCurrentRow(0)

        self.cmdExecute.setFocus(True)

    @pyqtSlot()
    def on_cmdExecute_clicked(self):
        major = ord(self.lstMajor.currentItem().text())
        minor = int(self.lstMinor.currentItem().text())
        command = 1 if self.lstCommand.currentItem().text() == "On" else 0
        device.switch(major, minor, command)

    @pyqtSlot()
    def on_cmdBack_clicked(self):
        self.parent.show_page1()
