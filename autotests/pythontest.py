#!/usr/bin/env python
#-*- coding: utf-8 -*-

import sys

sys.path.append(sys.argv[1])

from PyQt5 import QtCore
from PyQt5 import QtGui

from PyKF5 import KGuiAddons

def main():
    app = QtGui.QGuiApplication(sys.argv)

    colorCollection = KGuiAddons.KColorCollection()

    colorCollection.addColor(QtCore.Qt.blue)
    colorCollection.addColor(QtCore.Qt.red)

    assert(colorCollection.count() == 2)

if __name__ == '__main__':
    main()
