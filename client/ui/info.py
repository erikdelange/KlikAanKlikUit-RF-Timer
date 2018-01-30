from PyQt5.QtCore import Qt, pyqtSlot
from PyQt5.QtWidgets import QDialog

import const
import device
import ui


class Info(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)

        ui.loadUi(__file__, self)

        self.setWindowFlags(Qt.Window | Qt.WindowTitleHint | Qt.CustomizeWindowHint)
        self.parent = parent

    def refresh(self):
        device_info = device.get_info()
        device_datetime = device.get_datetime()

        self.txtHwVersion.setText(device_info.hw_version)
        self.txtSwVersion.setText(device_info.sw_version)
        self.txtMemorySize.setText(str(device_info.memory_size))
        self.txtActionCount.setText(str(device_info.action_count))
        self.txtMaxActions.setText(str(device_info.memory_size // const.SIZEOFACTION))
        self.txtDate.setText(str(device_datetime.date))
        self.txtTime.setText(str(device_datetime.time))
        self.txtWeekday.setText(str(device_datetime.weekday))
        self.lblWeekdayName.setText(const.WEEKDAYNAMES[device_datetime.weekday - 1])

    @pyqtSlot()
    def on_cmdBack_clicked(self):
        self.parent.show_page1()
