import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import QtMultimedia 5.15
import QtQuick.Dialogs 1.3
import org.kde.kirigami 2.20 as Kirigami

Kirigami.ApplicationWindow {
    id: root
    title: i18n("Muse Music Player")
    minimumWidth: Kirigami.Units.gridUnit * 20
    minimumHeight: Kirigami.Units.gridUnit * 15

    // Main content
    pageStack.initialPage: mainPage

    FileDialog {
        id: fileDialog
        title: i18n("Choose a music file")
        folder: shortcuts.music
        nameFilters: ["Music files (*.mp3 *.m4a *.ogg *.flac *.wav)", "All files (*)"]
        onAccepted: {
            musicPlayer.setSource(fileDialog.fileUrl)
            musicPlayer.play()
        }
    }

    Kirigami.ScrollablePage {
        id: mainPage
        title: i18n("Now Playing")

        ColumnLayout {
            anchors.fill: parent
            spacing: Kirigami.Units.largeSpacing

            // Album art placeholder
            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                width: Kirigami.Units.gridUnit * 15
                height: width
                color: Kirigami.Theme.backgroundColor
                radius: 10

                QQC2.Label {
                    anchors.centerIn: parent
                    text: i18n("No Album Art")
                    color: Kirigami.Theme.textColor
                }
            }

            // Song info
            ColumnLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: Kirigami.Units.smallSpacing

                QQC2.Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: musicPlayer.currentSong
                    font.pointSize: 16
                }

                QQC2.Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: musicPlayer.currentArtist
                    color: Kirigami.Theme.textColor
                }
            }

            // Progress bar
            QQC2.ProgressBar {
                Layout.fillWidth: true
                from: 0
                to: musicPlayer.duration
                value: musicPlayer.position
            }

            // Playback controls
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: Kirigami.Units.largeSpacing

                QQC2.Button {
                    text: i18n("⏮")
                    onClicked: musicPlayer.seek(musicPlayer.position - 5000)
                }

                QQC2.Button {
                    text: musicPlayer.isPlaying ? i18n("⏸") : i18n("▶")
                    onClicked: {
                        if (musicPlayer.isPlaying) {
                            musicPlayer.pause()
                        } else {
                            musicPlayer.play()
                        }
                    }
                }

                QQC2.Button {
                    text: i18n("⏭")
                    onClicked: musicPlayer.seek(musicPlayer.position + 5000)
                }
            }

            // Volume control
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: Kirigami.Units.smallSpacing

                QQC2.Label {
                    text: i18n("🔊")
                }

                QQC2.Slider {
                    from: 0
                    to: 1
                    value: 0.5
                    onValueChanged: musicPlayer.setVolume(value)
                    Layout.fillWidth: true
                }
            }
        }
    }

    // Global drawer for navigation
    Kirigami.GlobalDrawer {
        title: i18n("Muse Music Player")
        titleIcon: "applications-multimedia"

        actions: [
            Kirigami.Action {
                text: i18n("Library")
                icon.name: "folder-music"
                onTriggered: fileDialog.open()
            },
            Kirigami.Action {
                text: i18n("Playlists")
                icon.name: "playlist-play"
            },
            Kirigami.Action {
                text: i18n("Settings")
                icon.name: "configure"
            }
        ]
    }
}
