from PyQt5.QtCore import QDate, QTime, Qt
from PyQt5.QtWidgets import QComboBox, QDateEdit, QStyledItemDelegate, QTableView, QTimeEdit, QHeaderView

import device
import const
import ui


class Editor(QTableView):
    def __init__(self, parent=None):
        super().__init__(parent)

        # Create a 6 column table for entering actions
        # Every column has the appropriate widget (combobox, date- or time picker)
        # The delete key clears a field

        self.setSortingEnabled(True)

        self.majorDelegate = MajorDelegate()
        self.setItemDelegateForColumn(0, self.majorDelegate)

        self.minorDelegate = MinorDelegate()
        self.setItemDelegateForColumn(1, self.minorDelegate)

        self.dateDelegate = DateDelegate()
        self.setItemDelegateForColumn(2, self.dateDelegate)

        self.weekdayDelegate = WeekdayDelegate()
        self.setItemDelegateForColumn(3, self.weekdayDelegate)

        self.timeDelegate = TimeDelegate()
        self.setItemDelegateForColumn(4, self.timeDelegate)

        self.commandDelegate = CommandDelegate()
        self.setItemDelegateForColumn(5, self.commandDelegate)

        self.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)

        self.show()

    def refreshModel(self):
        info = device.get_info()
        self.setModel(ui.model.MyModel(rows=info.memory_size // const.SIZEOFACTION, parent=self))

    def keyPressEvent(self, event):
        if event.key() == Qt.Key_Delete:
            self.model().setData(self.currentIndex(), None)
        super().keyPressEvent(event)


class MajorDelegate(QStyledItemDelegate):
    def __init__(self, parent=None):
        super().__init__(parent)

    def createEditor(self, parent, option, index):
        editor = MajorEditor(parent)
        return editor


class MajorEditor(QComboBox):
    def __init__(self, parent=None):
        super().__init__(parent)
        # precede values with an empty string, used to clear the value as the field is not editable
        self.addItems([""] + const.MAJORNAMES)


class MinorDelegate(QStyledItemDelegate):
    def __init__(self, parent=None):
        super().__init__(parent)

    def createEditor(self, parent, option, index):
        editor = MinorEditor(parent)
        return editor


class MinorEditor(QComboBox):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.addItems([""] + const.MINORNAMES)


class DateDelegate(QStyledItemDelegate):
    def __init__(self, parent=None):
        super().__init__(parent)

    def createEditor(self, parent, option, index):
        editor = DateEditor(parent)
        return editor


class DateEditor(QDateEdit):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setCalendarPopup(True)
        self.setDate(QDate.currentDate())


class WeekdayDelegate(QStyledItemDelegate):
    def __init__(self, parent=None):
        super().__init__(parent)

    def createEditor(self, parent, option, index):
        editor = WeekdayEditor(parent)
        return editor


class WeekdayEditor(QComboBox):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.addItems([""] + const.WEEKDAYNAMES)


class TimeDelegate(QStyledItemDelegate):
    def __init__(self, parent=None):
        super().__init__(parent)

    def createEditor(self, parent, option, index):
        editor = TimeEditor(parent)
        return editor


class TimeEditor(QTimeEdit):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setCalendarPopup(True)
        self.setTime(QTime.currentTime())


class CommandDelegate(QStyledItemDelegate):
    def __init__(self, parent=None):
        super().__init__(parent)

    def createEditor(self, parent, option, index):
        editor = CommandEditor(parent)
        return editor


class CommandEditor(QComboBox):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.addItems([""] + const.COMMANDNAMES)
