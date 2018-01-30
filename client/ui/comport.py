from PyQt5.QtCore import Qt, pyqtSlot
from PyQt5.QtWidgets import QApplication, QDialog, QListWidgetItem, QMessageBox
from serial.tools import list_ports

import device
import ui


class Comport(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)

        ui.loadUi(__file__, self)

        self.setWindowFlags(Qt.Window | Qt.WindowTitleHint | Qt.CustomizeWindowHint)
        self.parent = parent
        self.refresh_list()

    def refresh_list(self):
        self.lstPorts.clear()

        for item in list(list_ports.comports()):
            self.lstPorts.addItem(item[0])

        if self.lstPorts.count() > 0:
            self.lstPorts.setCurrentRow(0)

    @pyqtSlot(QListWidgetItem)
    def on_lstPorts_itemDoubleClicked(self, item):
        port = item.text()
        if device.connect(port) is False:
            QMessageBox.critical(self.parent, QApplication.applicationName(), "Could not connect to {}".format(port))
        else:
            self.parent.show_page1(first=True)

    @pyqtSlot()
    def on_cmdRefresh_clicked(self):
        self.refresh_list()
