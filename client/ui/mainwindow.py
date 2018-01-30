from PyQt5.QtCore import pyqtSlot
from PyQt5.QtWidgets import QApplication, QFileDialog, QMainWindow

import device
import ui


class MainWindow(QMainWindow):
    def __init__(self, parent=None):
        super().__init__(parent)

        ui.loadUi(__file__, self)

        self.menubar.setVisible(False)

        self.page0 = ui.Comport(self)
        self.page1 = ui.Editor(self)
        self.page2 = ui.Info(self)
        self.page3 = ui.Manual(self)

        self.stack.addWidget(self.page0)
        self.stack.addWidget(self.page1)
        self.stack.addWidget(self.page2)
        self.stack.addWidget(self.page3)

        self.stack.setCurrentIndex(0)

    def closeEvent(self, event):
        device.close()
        super().closeEvent(event)

    @pyqtSlot()
    def on_actionNew_triggered(self):
        self.page1.model().clear()

    @pyqtSlot()
    def on_actionLoad_triggered(self):
        filename = QFileDialog().getOpenFileName(self, "Load Schedule", filter="Text (*.txt)")
        if len(filename[0]) > 0:
            self.page1.model().load(filename[0])

    @pyqtSlot()
    def on_actionSave_triggered(self):
        filename = QFileDialog().getSaveFileName(self, "Save Schedule", filter="Text (*.txt)")
        if len(filename[0]) > 0:
            self.page1.model().save(filename[0])

    @pyqtSlot()
    def on_actionRead_from_Device_triggered(self):
        self.page1.model().read_from_device()

    @pyqtSlot()
    def on_actionWrite_to_Device_triggered(self):
        self.page1.model().write_to_device()

    @pyqtSlot()
    def on_actionInfo_triggered(self):
        self.show_page2()

    @pyqtSlot()
    def on_actionStart_Timer_triggered(self):
        device.start_timer()

    @pyqtSlot()
    def on_actionStop_Timer_triggered(self):
        device.stop_timer()

    @pyqtSlot()
    def on_actionManual_triggered(self):
        self.show_page3()

    @pyqtSlot()
    def on_actionSet_Date_triggered(self):
        device.set_datetime()

    @pyqtSlot()
    def on_actionSet_Info_triggered(self):
        # Not implemented
        pass

    def show_page1(self, first=False):
        if first is True:
            widget = self.stack.widget(1)
            widget.refreshModel()
        self.setWindowTitle(QApplication.applicationName())
        self.menubar.setVisible(True)
        self.stack.setCurrentIndex(1)

    def show_page2(self):
        self.setWindowTitle("Device info")
        self.menubar.setVisible(False)
        self.stack.setCurrentIndex(2)
        widget = self.stack.widget(2)
        widget.refresh()

    def show_page3(self):
        self.setWindowTitle("Manually control device")
        self.menubar.setVisible(False)
        self.stack.setCurrentIndex(3)
