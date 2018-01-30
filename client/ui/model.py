import json

from PyQt5.QtCore import QAbstractTableModel, QDate, QTime, QVariant, Qt

import const
import device
import ui


class MyModel(QAbstractTableModel):
    def __init__(self, rows=0, parent=None):
        super().__init__(parent)

        self.parent = parent
        self.rows = rows
        self.myList = [list([None] * 6) for _ in range(rows)]

    def rowCount(self, parent, **kwargs):
        return len(self.myList)

    def columnCount(self, parent, **kwargs):
        if len(self.myList) > 0:
            return len(self.myList[0])
        return 0

    def data(self, index, role=Qt.DisplayRole):
        if not index.isValid():
            return None
        elif role == Qt.TextAlignmentRole:
            return QVariant(Qt.AlignCenter | Qt.AlignVCenter)
        elif role == Qt.DisplayRole or role == Qt.EditRole:
            value = self.myList[index.row()][index.column()]
            return value
        else:
            return None

    def flags(self, index):
        return Qt.ItemIsEditable | QAbstractTableModel.flags(self, index)

    def headerData(self, col, orientation, role):
        if orientation == Qt.Horizontal and role == Qt.DisplayRole:
            return ["Major", "Minor", "Date", "Weekday", "Time", "Command"][col]
        return None

    def setData(self, index, value, role=Qt.EditRole):
        if isinstance(value, str) and value.strip() == "":
            value = None
        if role == Qt.EditRole:
            self.myList[index.row()][index.column()] = value
            self.dataChanged.emit(index, index)
            return True
        return False

    def sort(self, col, order):
        self.layoutAboutToBeChanged.emit()
        self.myList = sorted(self.myList, key=lambda x: (x[col] is None, x[col]))
        # self.myList = sorted(self.myList, key=operator.itemgetter(col))
        if order == Qt.DescendingOrder:
            self.myList.reverse()
        self.layoutChanged.emit()

    def clear(self):
        self.myList = [list([None] * 6) for _ in range(self.rows)]

    def save(self, filename="output.txt"):
        def json_helper(obj):
            if type(obj) == QDate:
                return obj.toString("dd-MM-yyyy")
            elif type(obj) == QTime:
                return obj.toString("hh:mm")
            else:
                return TypeError

        self.nullify()

        with open(filename, "w") as f:
            json.dump(self.myList, f, default=json_helper, indent=4)

    def load(self, filename="output.txt"):
        with open(filename, "r") as f:
            self.myList = json.load(f)

        self.nullify()

        for row in self.myList:
            if row[2] is not None:
                row[2] = QDate().fromString(row[2], "dd-MM-yyyy")
            if row[4] is not None:
                row[4] = QTime.fromString(row[4], "hh:mm")

    def read_from_device(self):
        self.clear()
        info = device.get_info()

        progressbar = ui.ProgressBar("Read from device", self.parent)
        progressbar.setMaximum(info.action_count)
        progressbar.show()

        for index in range(info.action_count):
            action = device.get_action(index)

            if action.valid == 1:
                row = self.myList[index]
                row[0] = action.major.decode("utf-8")
                row[1] = str(action.minor)
                row[2] = None if action.dd == 0 else QDate(action.yy + 2000, action.mm, action.dd)
                row[3] = None if action.wd == 0 else const.WEEKDAYNAMES[action.wd - 1]
                row[4] = QTime(action.hh, action.mn)
                row[5] = "On" if action.cmd == 1 else "Off"

            progressbar.incrementValue()

        progressbar.close()

    def write_to_device(self):
        self.nullify()

        sorted_list = sorted(self.myList, key=lambda x: (x[4] is None, x[4]))

        progressbar = ui.ProgressBar("Write to device", self.parent)
        progressbar.setMaximum(self.rows)
        progressbar.show()

        device.stop_timer()

        count = 0
        for row in sorted_list:
            # major, minor, time and command must be filled
            if row[0] is None or row[1] is None or row[4] is None or row[5] is None:
                continue
            else:
                major = 0 if row[0] is None else row[0]
                minor = 0 if row[1] is None else int(row[1])
                dd = 0 if row[2] is None else row[2].day()
                mm = 0 if row[2] is None else row[2].month()
                yy = 0 if row[2] is None else row[2].year() - 2000
                wd = 0 if row[3] is None else const.WEEKDAYNAMES.index(row[3]) + 1
                hh = 0 if row[4] is None else row[4].hour()
                mn = 0 if row[4] is None else row[4].minute()
                cmd = 1 if row[5] == "On" else 0

                action = device.action_type(1, major, minor, dd, mm, yy, wd, hh, mn, cmd)
                device.set_action(count, action)

                count += 1
                progressbar.setValue(count)

        empty_action = device.action_type(0, b"0", 0, 0, 0, 0, 0, 0, 0, 0)

        for index in range(count, self.rows):
            device.set_action(index, empty_action)
            progressbar.setValue(index)

        device.start_timer()
        # device.reset()

        progressbar.close()

    # replace empty strings (left when clearing an entry) by None values
    #
    def nullify(self):
        for r, row in enumerate(self.myList):
            for c, col in enumerate(row):
                if col == "":
                    self.myList[r][c] = None
