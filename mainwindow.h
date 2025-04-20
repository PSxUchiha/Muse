#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QStackedWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QFrame>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include "musiclibrary.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onPlayPauseClicked();
    void onNextClicked();
    void onPreviousClicked();
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void onPlaylistPositionChanged(int position);
    void onItemDoubleClicked(QListWidgetItem* item);
    void onAudioFilesChanged(const QStringList& files);
    void onNavigationButtonClicked(int index);
    void onMiniPlayerClicked();

private:
    void setupUI();
    void setupConnections();
    void updatePlayPauseButton();
    void showFullscreenPlayer();
    void hideFullscreenPlayer();
    void updateNowPlayingInfo();
    void updateMetadata();
    QImage extractAlbumArt(const QString &filePath);
    void setupSidebar();
    void setupPages();
    void switchToPage(int index);

    // Main UI components
    QWidget *centralWidget;
    QVBoxLayout *mainLayout;
    QMediaPlayer *mediaPlayer;
    QAudioOutput *audioOutput;
    MusicLibrary *musicLibrary;
    QListWidget *playlistWidget;
    QPushButton *playPauseButton;
    QPushButton *nextButton;
    QPushButton *previousButton;
    QSlider *positionSlider;
    QLabel *nowPlayingLabel;
    QLabel *timeLabel;
    bool wasPlaying;

    // Mini player widgets
    QWidget *miniPlayer;
    QLabel *miniAlbumArt;
    QLabel *miniTitleLabel;
    QLabel *miniArtistLabel;

    // Fullscreen player widgets
    QWidget *fullscreenPlayer;
    QLabel *fullscreenAlbumArt;
    QLabel *fullscreenTitleLabel;
    QLabel *fullscreenArtistLabel;
    QPushButton *fullscreenPlayPauseButton;
    QPushButton *fullscreenNextButton;
    QPushButton *fullscreenPreviousButton;
    QSlider *fullscreenProgressSlider;
    QLabel *fullscreenTimeLabel;
    QLabel *fullscreenDurationLabel;
    QLabel *albumArtLabel;
    QLabel *titleLabel;
    QLabel *artistLabel;
    QLabel *albumLabel;
    QPropertyAnimation *fullscreenAnimation;
    QGraphicsOpacityEffect *fullscreenOpacityEffect;

    // Sidebar and navigation
    QWidget *sidebar;
    QToolButton *menuButton;
    QStackedWidget *pages;
    QList<QPushButton*> navButtons;
    QWidget *tracksPage;
    QWidget *albumsPage;
    QWidget *artistsPage;
    QWidget *playlistsPage;
    QListWidget *tracksList;
    QListWidget *albumsList;
    QListWidget *artistsList;
    QListWidget *playlistsList;
    bool sidebarVisible;
    QSize originalWindowSize;  // Store the original window size
};

#endif // MAINWINDOW_H 