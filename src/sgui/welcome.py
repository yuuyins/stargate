from sgui.sgqt import *
from .project import new_project, open_project
from sglib.lib import util
import os


class Welcome:
    def __init__(self, app, scaler):
        self.app = app
        self.scaler = scaler
        self.loaded = False

        self.widget = QDialog()
        self.widget.setWindowTitle("Stargate")
        self.widget.setWindowState(QtCore.Qt.WindowState.WindowMaximized)
        hlayout = QHBoxLayout(self.widget)
        rp_vlayout = QVBoxLayout()
        hlayout.addLayout(rp_vlayout)
        rp_vlayout.addWidget(QLabel("Recent Projects"))
        self.rp_list = QListWidget()
        self.rp_list.doubleClicked.connect(
            self.rp_doubleclick,
        )
        rp_vlayout.addWidget(self.rp_list)
        self.load_rp()
        buttons_vlayout = QVBoxLayout()
        hlayout.addLayout(buttons_vlayout)
        buttons_vlayout.addItem(
            QSpacerItem(10, 10, vPolicy=QSizePolicy.Policy.Expanding),
        )
        buttons_hlayout = QHBoxLayout()
        buttons_vlayout.addLayout(buttons_hlayout)
        buttons_vlayout.addItem(
            QSpacerItem(10, 10, vPolicy=QSizePolicy.Policy.Expanding),
        )
        new_button = QPushButton("New")
        new_button.setObjectName("huge_button")
        new_button.pressed.connect(self.on_new)
        buttons_hlayout.addWidget(new_button)

        open_button = QPushButton("Open")
        open_button.setObjectName("huge_button")
        open_button.pressed.connect(self.on_open)
        buttons_hlayout.addWidget(open_button)

        clone_button = QPushButton("Clone")
        clone_button.setObjectName("huge_button")
        clone_button.pressed.connect(self.on_clone)
        buttons_hlayout.addWidget(clone_button)

        self.widget.exec()

    def rp_doubleclick(self, index):
        util.set_file_setting(
            'last-project',
            str(self.rp_list.item(index.row()).text()),
        )
        self.close()

    def load_rp(self):
        """ Load a list of recent projects
        """
        history = util.get_file_setting("project-history", str, "")
        history = [
            x for x in history.split("\n")
            if (
                x.strip()
                and
                os.path.exists(x)
            )
        ]
        if history:
            self.rp_list.addItems(history)
        self.history = history

    def close(self):
        self.widget.close()
        self.loaded = True
        project = util.get_file_setting('last-project', str, None)
        assert project, project
        if project in self.history:
            self.history.remove(project)
        self.history.insert(0, project)
        history = util.set_file_setting(
            "project-history",
            "\n".join(self.history[:20]),
        )

    def on_new(self):
        if new_project(self.widget):
            self.close()

    def on_open(self):
        if open_project(self.widget):
            self.close()

    def on_clone(self):
        if clone_project(self.widget):
            self.close()

