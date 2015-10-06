load(common_pre)

TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += src/blackmisc
SUBDIRS += resources/swiftresources.pro
OTHER_FILES += mkspecs/features/*.prf
OTHER_FILES += mkspecs/features/*.pri

contains(BLACK_CONFIG, BlackSound) {
    SUBDIRS += src/blacksound
}

contains(BLACK_CONFIG, BlackInput) {
    SUBDIRS += src/blackinput
}

contains(BLACK_CONFIG, BlackCore) {
    SUBDIRS += src/blackcore
    contains(BLACK_CONFIG, BlackGui) {
        SUBDIRS += src/blackgui
        contains(BLACK_CONFIG, SwiftGui) {
            SUBDIRS += src/swiftguistandard/swiftguistandard.pro
        }
        contains(BLACK_CONFIG, SwiftCore) {
            SUBDIRS += src/swiftcore/swiftcore.pro
        }
        contains(BLACK_CONFIG, SwiftData) {
            SUBDIRS += src/swiftdata/swiftdata.pro
        }
        contains(BLACK_CONFIG, SwiftLauncher) {
            SUBDIRS += src/swiftlauncher/swiftlauncher.pro
        }
    }
}

win32 {
    if (contains(BLACK_CONFIG, FSX) | contains(BLACK_CONFIG, FS9)) {
        SUBDIRS += src/plugins/simulator/fscommon/plugin_fscommon.pro
    }

    contains(BLACK_CONFIG, FSX) {
        SUBDIRS += src/plugins/simulator/fsx/plugin_fsx.pro
        SUBDIRS += src/plugins/simulator/fsx_config/plugin_fsx_config.pro
    }
    contains(BLACK_CONFIG, FS9) {
        SUBDIRS += src/plugins/simulator/fs9/plugin_fs9.pro
    }
}

contains(BLACK_CONFIG, XPlane) {
    SUBDIRS += src/xbus/xbus.pro
    SUBDIRS += src/plugins/simulator/xplane/plugin_xplane.pro
    SUBDIRS += src/plugins/simulator/xplane_config/plugin_xplane_config.pro
}

contains(BLACK_CONFIG, Samples) {
    SUBDIRS += samples/cli_client/sample_cli_client.pro
    SUBDIRS += samples/blackmiscquantities/sample_quantities_aviation.pro
    SUBDIRS += samples/blackmiscdbus/sample_blackmisc_dbus.pro
    SUBDIRS += samples/blackmisc/sample_blackmisc.pro
    SUBDIRS += samples/blackmiscsim/sample_blackmisc_sim.pro
    SUBDIRS += samples/hotkey/sample_hotkey.pro
}

contains(BLACK_CONFIG, Unittests) {
    SUBDIRS += tests/blackmisc/test_blackmisc.pro
    SUBDIRS += tests/blackcore/test_blackcore.pro
    SUBDIRS += tests/blackgui/test_blackgui.pro
}

contains(BLACK_CONFIG, Doxygen) {
    SUBDIRS += docs/doxygen.pro
}

include(install.pri)
