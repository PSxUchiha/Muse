#include "mainwindow.h"
#include "theme.h"
#include <QStyle>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QApplication>
#include <QScreen>
#include <QMouseEvent>
#include <QDebug>
#include <QResizeEvent>
#include <QPainter>
#include <QStyleFactory>
#include <QStyleOption>
#include <QStylePainter>
#include <QGraphicsBlurEffect>
#include <QMediaMetaData>
#include <QStackedWidget>
#include <QProcess>
#include <taglib/taglib.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tfile.h>
#include <taglib/apefile.h>
#include <taglib/asffile.h>
#include <taglib/flacfile.h>
#include <taglib/mp4file.h>
#include <taglib/mpcfile.h>
#include <taglib/mpegfile.h>
#include <taglib/oggfile.h>
#include <taglib/opusfile.h>
#include <taglib/wavfile.h>
#include <taglib/wavpackfile.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/id3v2tag.h>
#include <taglib/id3v2frame.h>
#include <taglib/id3v2header.h>
#include <taglib/flacpicture.h>
#include <taglib/mp4coverart.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Use system theme
    qApp->setStyle(QApplication::style()->objectName());
    
    // Initialize media player and audio output first
    mediaPlayer = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    mediaPlayer->setAudioOutput(audioOutput);
    
    // Initialize music library
    musicLibrary = new MusicLibrary(this);
    connect(musicLibrary, &MusicLibrary::audioFilesChanged, this, &MainWindow::onAudioFilesChanged);
    
    setupUI();
    setupConnections();

    // Start scanning for music files
    qDebug() << "Starting music library scan...";
    musicLibrary->scanMusicDirectory();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    // Create central widget and main layout
    centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    setCentralWidget(centralWidget);

    // Create top bar with menu button
    QWidget *topBar = new QWidget;
    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(10, 5, 10, 5);

    // Create menu button and move it to the left
    menuButton = new QToolButton(this);
    menuButton->setIcon(QIcon::fromTheme("application-menu"));
    menuButton->setFixedSize(32, 32);
    menuButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    menuButton->setStyleSheet(Theme::TOOL_BUTTON_STYLE);
    connect(menuButton, &QToolButton::clicked, this, [this]() {
        sidebarVisible = !sidebarVisible;
        sidebar->setVisible(sidebarVisible);
    });

    topLayout->addWidget(menuButton);
    
    topLayout->addStretch();
    mainLayout->addWidget(topBar);

    // Setup sidebar
    setupSidebar();
    QHBoxLayout *contentLayout = new QHBoxLayout;
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    contentLayout->addWidget(sidebar);
    sidebar->hide(); // Initially hidden
    sidebarVisible = false;

    // Create main content area
    QWidget *mainContent = new QWidget;
    QVBoxLayout *mainContentLayout = new QVBoxLayout(mainContent);
    mainContentLayout->setContentsMargins(0, 0, 0, 0);
    mainContentLayout->setSpacing(0);

    // Setup pages
    setupPages();
    mainContentLayout->addWidget(pages);

    contentLayout->addWidget(mainContent);
    mainLayout->addLayout(contentLayout);

    // Create mini player
    miniPlayer = new QWidget(this);
    miniPlayer->setFixedHeight(80);
    miniPlayer->setStyleSheet(Theme::MINI_PLAYER_STYLE);

    QHBoxLayout *miniLayout = new QHBoxLayout(miniPlayer);
    miniLayout->setContentsMargins(10, 5, 10, 5);
    miniLayout->setSpacing(10);

    // Mini player controls - moved to the left
    QWidget *miniControls = new QWidget(miniPlayer);
    QHBoxLayout *miniControlsLayout = new QHBoxLayout(miniControls);
    miniControlsLayout->setContentsMargins(0, 0, 0, 0);
    miniControlsLayout->setSpacing(10);

    previousButton = new QPushButton(miniPlayer);
    previousButton->setIcon(QIcon::fromTheme("media-skip-backward"));
    previousButton->setFixedSize(32, 32);
    previousButton->setStyleSheet(Theme::BUTTON_STYLE);
    miniControlsLayout->addWidget(previousButton);

    playPauseButton = new QPushButton(miniPlayer);
    playPauseButton->setIcon(QIcon::fromTheme("media-playback-start"));
    playPauseButton->setFixedSize(32, 32);
    playPauseButton->setStyleSheet(Theme::BUTTON_STYLE);
    miniControlsLayout->addWidget(playPauseButton);

    nextButton = new QPushButton(miniPlayer);
    nextButton->setIcon(QIcon::fromTheme("media-skip-forward"));
    nextButton->setFixedSize(32, 32);
    nextButton->setStyleSheet(Theme::BUTTON_STYLE);
    miniControlsLayout->addWidget(nextButton);

    miniLayout->addWidget(miniControls);

    // Mini player album art
    miniAlbumArt = new QLabel(miniPlayer);
    miniAlbumArt->setFixedSize(60, 60);
    miniAlbumArt->setStyleSheet("background-color: palette(mid); border-radius: 5px;");
    miniLayout->addWidget(miniAlbumArt);

    // Mini player info
    QWidget *miniInfo = new QWidget(miniPlayer);
    QVBoxLayout *miniInfoLayout = new QVBoxLayout(miniInfo);
    miniInfoLayout->setContentsMargins(0, 0, 0, 0);
    miniInfoLayout->setSpacing(2);

    miniTitleLabel = new QLabel(miniPlayer);
    miniTitleLabel->setStyleSheet(Theme::LABEL_STYLE + "font-weight: bold;");
    miniInfoLayout->addWidget(miniTitleLabel);

    miniArtistLabel = new QLabel(miniPlayer);
    miniArtistLabel->setStyleSheet(Theme::LABEL_STYLE + "color: palette(mid);");
    miniInfoLayout->addWidget(miniArtistLabel);

    // Initialize nowPlayingLabel which is used but never properly created
    nowPlayingLabel = miniTitleLabel;
    
    miniLayout->addWidget(miniInfo);
    miniLayout->addStretch();

    // Position slider
    positionSlider = new QSlider(Qt::Horizontal, miniPlayer);
    positionSlider->setStyleSheet(Theme::SLIDER_STYLE);
    miniLayout->addWidget(positionSlider);

    // Time label
    timeLabel = new QLabel(miniPlayer);
    timeLabel->setStyleSheet(Theme::LABEL_STYLE + "color: palette(mid);");
    timeLabel->setFixedWidth(50);
    miniLayout->addWidget(timeLabel);

    // Add mini player to main content
    mainContentLayout->addWidget(miniPlayer);

    // Create fullscreen player
    fullscreenPlayer = new QWidget(this);
    fullscreenPlayer->setStyleSheet(Theme::FULLSCREEN_PLAYER_STYLE);

    QVBoxLayout *fullscreenLayout = new QVBoxLayout(fullscreenPlayer);
    fullscreenLayout->setContentsMargins(20, 20, 20, 20);
    fullscreenLayout->setSpacing(20);

    // Album art - make it responsive
    fullscreenAlbumArt = new QLabel(fullscreenPlayer);
    fullscreenAlbumArt->setMinimumSize(200, 200);
    fullscreenAlbumArt->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    fullscreenAlbumArt->setStyleSheet("background-color: palette(mid); border-radius: 10px;");
    fullscreenLayout->addWidget(fullscreenAlbumArt, 1, Qt::AlignHCenter);

    // Title and artist
    QWidget *fullscreenInfo = new QWidget(fullscreenPlayer);
    QVBoxLayout *fullscreenInfoLayout = new QVBoxLayout(fullscreenInfo);
    fullscreenInfoLayout->setContentsMargins(0, 0, 0, 0);
    fullscreenInfoLayout->setSpacing(5);

    fullscreenTitleLabel = new QLabel(fullscreenPlayer);
    fullscreenTitleLabel->setStyleSheet(Theme::LABEL_STYLE + "font-size: 24px; font-weight: bold;");
    fullscreenTitleLabel->setAlignment(Qt::AlignHCenter);
    fullscreenInfoLayout->addWidget(fullscreenTitleLabel);

    fullscreenArtistLabel = new QLabel(fullscreenPlayer);
    fullscreenArtistLabel->setStyleSheet(Theme::LABEL_STYLE + "font-size: 18px; color: palette(mid);");
    fullscreenArtistLabel->setAlignment(Qt::AlignHCenter);
    fullscreenInfoLayout->addWidget(fullscreenArtistLabel);

    fullscreenLayout->addWidget(fullscreenInfo);

    // Progress slider
    fullscreenProgressSlider = new QSlider(Qt::Horizontal, fullscreenPlayer);
    fullscreenProgressSlider->setStyleSheet(Theme::SLIDER_STYLE);
    fullscreenLayout->addWidget(fullscreenProgressSlider);

    // Time labels
    QWidget *timeLabels = new QWidget(fullscreenPlayer);
    QHBoxLayout *timeLabelsLayout = new QHBoxLayout(timeLabels);
    timeLabelsLayout->setContentsMargins(0, 0, 0, 0);

    fullscreenTimeLabel = new QLabel(fullscreenPlayer);
    fullscreenTimeLabel->setStyleSheet(Theme::LABEL_STYLE + "color: palette(mid);");
    timeLabelsLayout->addWidget(fullscreenTimeLabel);

    timeLabelsLayout->addStretch();

    fullscreenDurationLabel = new QLabel(fullscreenPlayer);
    fullscreenDurationLabel->setStyleSheet(Theme::LABEL_STYLE + "color: palette(mid);");
    timeLabelsLayout->addWidget(fullscreenDurationLabel);

    fullscreenLayout->addWidget(timeLabels);

    // Control buttons
    QWidget *fullscreenControls = new QWidget(fullscreenPlayer);
    QHBoxLayout *fullscreenControlsLayout = new QHBoxLayout(fullscreenControls);
    fullscreenControlsLayout->setContentsMargins(0, 0, 0, 0);
    fullscreenControlsLayout->setSpacing(20);
    fullscreenControlsLayout->setAlignment(Qt::AlignHCenter);

    fullscreenPreviousButton = new QPushButton(fullscreenPlayer);
    fullscreenPreviousButton->setIcon(QIcon::fromTheme("media-skip-backward"));
    fullscreenPreviousButton->setFixedSize(48, 48);
    fullscreenPreviousButton->setStyleSheet(Theme::BUTTON_STYLE);
    fullscreenControlsLayout->addWidget(fullscreenPreviousButton);

    fullscreenPlayPauseButton = new QPushButton(fullscreenPlayer);
    fullscreenPlayPauseButton->setIcon(QIcon::fromTheme("media-playback-start"));
    fullscreenPlayPauseButton->setFixedSize(64, 64);
    fullscreenPlayPauseButton->setStyleSheet(Theme::BUTTON_STYLE);
    fullscreenControlsLayout->addWidget(fullscreenPlayPauseButton);

    fullscreenNextButton = new QPushButton(fullscreenPlayer);
    fullscreenNextButton->setIcon(QIcon::fromTheme("media-skip-forward"));
    fullscreenNextButton->setFixedSize(48, 48);
    fullscreenNextButton->setStyleSheet(Theme::BUTTON_STYLE);
    fullscreenControlsLayout->addWidget(fullscreenNextButton);

    fullscreenLayout->addWidget(fullscreenControls);

    // Add fullscreen player to the stacked widget
    pages->addWidget(fullscreenPlayer);
    fullscreenPlayer->hide(); // Initially hidden

    // Set window properties
    setWindowTitle("Muse");
    resize(800, 600);
    setStyleSheet(Theme::MAIN_WINDOW_STYLE);

    // Set tracks page as default
    switchToPage(0);
    
    // Initialize other required labels used in updateMetadata()
    titleLabel = miniTitleLabel;
    artistLabel = miniArtistLabel;
    // Create albumLabel but don't add it to any layout
    albumLabel = new QLabel(this);
    albumLabel->hide(); // Hide the album label
    albumArtLabel = miniAlbumArt;
}

void MainWindow::setupSidebar()
{
    sidebar = new QWidget(this);
    sidebar->setFixedWidth(250);
    sidebar->setStyleSheet(Theme::SIDEBAR_STYLE);

    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(0, 0, 0, 0);
    sidebarLayout->setSpacing(0);

    // Navigation buttons
    QStringList navItems = {"Tracks", "Albums", "Artists", "Playlists"};
    QStringList navIcons = {"audio-x-generic", "media-optical-audio", "view-media-artist", "view-media-playlist"};
    
    for (int i = 0; i < navItems.size(); ++i) {
        QPushButton *button = new QPushButton(navItems[i], sidebar);
        button->setIcon(QIcon::fromTheme(navIcons[i]));
        button->setCheckable(true);
        button->setProperty("pageIndex", i);
        connect(button, &QPushButton::clicked, this, [this, i]() { onNavigationButtonClicked(i); });
        navButtons.append(button);
        sidebarLayout->addWidget(button);
    }

    sidebarLayout->addStretch();
}

void MainWindow::setupPages()
{
    pages = new QStackedWidget(this);
    pages->setStyleSheet(Theme::MAIN_WINDOW_STYLE);
    pages->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Tracks page
    tracksPage = new QWidget;
    QVBoxLayout *tracksLayout = new QVBoxLayout(tracksPage);
    tracksLayout->setContentsMargins(0, 0, 0, 0);
    tracksList = new QListWidget;
    tracksList->setStyleSheet(Theme::LIST_WIDGET_STYLE);
    tracksLayout->addWidget(tracksList);
    pages->addWidget(tracksPage);
    
    // Initialize playlistWidget
    playlistWidget = tracksList;

    // Albums page
    albumsPage = new QWidget;
    QVBoxLayout *albumsLayout = new QVBoxLayout(albumsPage);
    albumsLayout->setContentsMargins(0, 0, 0, 0);
    albumsList = new QListWidget;
    albumsList->setStyleSheet(Theme::LIST_WIDGET_STYLE);
    albumsLayout->addWidget(albumsList);
    pages->addWidget(albumsPage);

    // Artists page
    artistsPage = new QWidget;
    QVBoxLayout *artistsLayout = new QVBoxLayout(artistsPage);
    artistsLayout->setContentsMargins(0, 0, 0, 0);
    artistsList = new QListWidget;
    artistsList->setStyleSheet(Theme::LIST_WIDGET_STYLE);
    artistsLayout->addWidget(artistsList);
    pages->addWidget(artistsPage);

    // Playlists page
    playlistsPage = new QWidget;
    QVBoxLayout *playlistsLayout = new QVBoxLayout(playlistsPage);
    playlistsLayout->setContentsMargins(0, 0, 0, 0);
    playlistsList = new QListWidget;
    playlistsList->setStyleSheet(Theme::LIST_WIDGET_STYLE);
    playlistsLayout->addWidget(playlistsList);
    pages->addWidget(playlistsPage);

    // Set tracks page as default
    switchToPage(0);
}

void MainWindow::switchToPage(int index)
{
    pages->setCurrentIndex(index);
    for (int i = 0; i < navButtons.size(); ++i) {
        navButtons[i]->setChecked(i == index);
    }
}

void MainWindow::onNavigationButtonClicked(int index)
{
    switchToPage(index);
}

void MainWindow::setupConnections()
{
    // Connect media player signals
    connect(mediaPlayer, &QMediaPlayer::positionChanged, this, &MainWindow::onPositionChanged);
    connect(mediaPlayer, &QMediaPlayer::durationChanged, this, &MainWindow::onDurationChanged);
    connect(mediaPlayer, &QMediaPlayer::playbackStateChanged, this, &MainWindow::updatePlayPauseButton);
    connect(mediaPlayer, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error error, const QString &errorString) {
        qDebug() << "Media player error:" << error << errorString;
        // Reset UI to a safe state
        updatePlayPauseButton();
        positionSlider->setValue(0);
        fullscreenProgressSlider->setValue(0);
        nowPlayingLabel->setText("Error playing media");
    });

    // Connect playlist signals
    connect(playlistWidget, &QListWidget::currentRowChanged, this, &MainWindow::onPlaylistPositionChanged);
    connect(playlistWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::onItemDoubleClicked);
    connect(musicLibrary, &MusicLibrary::audioFilesChanged, this, &MainWindow::onAudioFilesChanged);

    // Connect control buttons
    connect(playPauseButton, &QPushButton::clicked, this, &MainWindow::onPlayPauseClicked);
    connect(nextButton, &QPushButton::clicked, this, &MainWindow::onNextClicked);
    connect(previousButton, &QPushButton::clicked, this, &MainWindow::onPreviousClicked);
    
    // Connect position slider signals
    connect(positionSlider, &QSlider::sliderPressed, this, [this]() {
        // Store the current playback state
        wasPlaying = mediaPlayer->playbackState() == QMediaPlayer::PlayingState;
        if (wasPlaying) {
            mediaPlayer->pause();
        }
    });
    
    connect(positionSlider, &QSlider::sliderReleased, this, [this]() {
        // Set the new position
        mediaPlayer->setPosition(positionSlider->value());
        // Resume playback if it was playing before
        if (wasPlaying) {
            mediaPlayer->play();
        }
    });
    
    connect(positionSlider, &QSlider::sliderMoved, this, [this](int value) {
        // Update the position in real-time while dragging
        mediaPlayer->setPosition(value);
    });

    // Install event filter for position slider to handle mouse clicks
    positionSlider->installEventFilter(this);

    // Connect fullscreen player controls
    connect(fullscreenPlayPauseButton, &QPushButton::clicked, this, &MainWindow::onPlayPauseClicked);
    connect(fullscreenNextButton, &QPushButton::clicked, this, &MainWindow::onNextClicked);
    connect(fullscreenPreviousButton, &QPushButton::clicked, this, &MainWindow::onPreviousClicked);
    connect(fullscreenProgressSlider, &QSlider::sliderMoved, mediaPlayer, &QMediaPlayer::setPosition);

    // Install event filter for fullscreen progress slider
    fullscreenProgressSlider->installEventFilter(this);

    // Install event filter for mini player click events
    miniPlayer->installEventFilter(this);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == miniPlayer && event->type() == QEvent::MouseButtonPress) {
        // Toggle the fullscreen player instead of just showing it
        if (fullscreenPlayer->isVisible()) {
            hideFullscreenPlayer();
        } else {
            showFullscreenPlayer();
        }
        return true;
    }
    else if ((obj == positionSlider || obj == fullscreenProgressSlider) && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            QSlider *slider = qobject_cast<QSlider*>(obj);
            // Calculate the position based on the mouse click
            double pos = mouseEvent->position().x() / slider->width();
            int value = pos * slider->maximum();
            slider->setValue(value);
            
            // Store the current playback state
            wasPlaying = mediaPlayer->playbackState() == QMediaPlayer::PlayingState;
            if (wasPlaying) {
                mediaPlayer->pause();
            }
            
            // Set the new position
            mediaPlayer->setPosition(value);
            
            // Resume playback if it was playing before
            if (wasPlaying) {
                mediaPlayer->play();
            }
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::showFullscreenPlayer()
{
    // Hide the mini player
    miniPlayer->hide();
    
    // Replace hamburger menu with back button
    menuButton->setIcon(QIcon::fromTheme("go-previous"));
    menuButton->setToolTip("Back to main screen");
    disconnect(menuButton, &QToolButton::clicked, nullptr, nullptr);
    connect(menuButton, &QToolButton::clicked, this, &MainWindow::hideFullscreenPlayer);
    
    // Find the index of the fullscreen player in the stacked widget
    int fullscreenIndex = -1;
    for (int i = 0; i < pages->count(); ++i) {
        if (pages->widget(i) == fullscreenPlayer) {
            fullscreenIndex = i;
            break;
        }
    }
    
    if (fullscreenIndex >= 0) {
        // Switch to the fullscreen player page
        pages->setCurrentIndex(fullscreenIndex);
        
        // Calculate the album art size based on the window size
        int albumSize = qMin(width(), height()) / 2;
        
        // Set both minimum and maximum size for the album art
        fullscreenAlbumArt->setMinimumSize(albumSize, albumSize);
        fullscreenAlbumArt->setMaximumSize(albumSize, albumSize);
        
        // Get the current file path and extract album art at full resolution
        QString currentFile = mediaPlayer->source().toLocalFile();
        QImage albumArt = extractAlbumArt(currentFile);
        
        if (!albumArt.isNull()) {
            // Create a pixmap with transparency
            QPixmap scaledPixmap(albumSize, albumSize);
            scaledPixmap.fill(Qt::transparent);
            
            // Create a painter to draw the scaled image
            QPainter painter(&scaledPixmap);
            painter.setRenderHint(QPainter::SmoothPixmapTransform);
            
            // Scale the image maintaining aspect ratio
            QImage scaledImage = albumArt.scaled(
                albumSize, 
                albumSize,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            );
            
            // Calculate position to center the image
            int x = (albumSize - scaledImage.width()) / 2;
            int y = (albumSize - scaledImage.height()) / 2;
            
            // Draw the image centered
            painter.drawImage(x, y, scaledImage);
            painter.end();
            
            // Set the pixmap
            fullscreenAlbumArt->setPixmap(scaledPixmap);
        }
        
        // Calculate control sizes based on window size
        int baseSize = qMin(width(), height());
        int controlSize = baseSize / 20;  // Smaller base size for controls
        int controlSpacing = controlSize / 2;  // Spacing between controls
        
        // Update control button sizes with fixed aspect ratio
        fullscreenPreviousButton->setFixedSize(controlSize, controlSize);
        fullscreenPlayPauseButton->setFixedSize(controlSize * 1.5, controlSize * 1.5);
        fullscreenNextButton->setFixedSize(controlSize, controlSize);
        
        // Update progress slider size
        fullscreenProgressSlider->setFixedHeight(controlSize / 2);
        
        // Update time labels font size
        QFont timeFont = fullscreenTimeLabel->font();
        timeFont.setPointSize(controlSize / 3);
        fullscreenTimeLabel->setFont(timeFont);
        fullscreenDurationLabel->setFont(timeFont);
        
        // Calculate margins based on window size
        int margin = baseSize / 25;  // 4% of the smaller window dimension
        
        // Update the layout margins
        QVBoxLayout* fullscreenLayout = qobject_cast<QVBoxLayout*>(fullscreenPlayer->layout());
        if (fullscreenLayout) {
            fullscreenLayout->setContentsMargins(margin, margin, margin, margin);
            fullscreenLayout->setSpacing(controlSpacing);
        }
        
        // Fade in animation
        fullscreenOpacityEffect = new QGraphicsOpacityEffect(fullscreenPlayer);
        fullscreenPlayer->setGraphicsEffect(fullscreenOpacityEffect);
        
        fullscreenAnimation = new QPropertyAnimation(fullscreenOpacityEffect, "opacity");
        fullscreenAnimation->setDuration(300);
        fullscreenAnimation->setStartValue(0.0);
        fullscreenAnimation->setEndValue(1.0);
        fullscreenAnimation->start();
    }
}

void MainWindow::hideFullscreenPlayer()
{
    // Fade out animation
    fullscreenAnimation = new QPropertyAnimation(fullscreenOpacityEffect, "opacity");
    fullscreenAnimation->setDuration(300);
    fullscreenAnimation->setStartValue(1.0);
    fullscreenAnimation->setEndValue(0.0);
    connect(fullscreenAnimation, &QPropertyAnimation::finished, [this]() {
        // Switch back to the tracks page (index 0)
        pages->setCurrentIndex(0);
        
        // Show the mini player again
        miniPlayer->show();
        
        // Restore hamburger menu
        menuButton->setIcon(QIcon::fromTheme("application-menu"));
        menuButton->setToolTip("");
        disconnect(menuButton, &QToolButton::clicked, nullptr, nullptr);
        connect(menuButton, &QToolButton::clicked, this, [this]() {
            sidebarVisible = !sidebarVisible;
            sidebar->setVisible(sidebarVisible);
        });
    });
    fullscreenAnimation->start();
}

void MainWindow::onPlayPauseClicked()
{
    if (mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        mediaPlayer->pause();
    } else {
        mediaPlayer->play();
    }
    updatePlayPauseButton();
}

void MainWindow::onNextClicked()
{
    int nextRow = playlistWidget->currentRow() + 1;
    if (nextRow < playlistWidget->count()) {
        playlistWidget->setCurrentRow(nextRow);
    }
}

void MainWindow::onPreviousClicked()
{
    int prevRow = playlistWidget->currentRow() - 1;
    if (prevRow >= 0) {
        playlistWidget->setCurrentRow(prevRow);
    }
}

void MainWindow::onPositionChanged(qint64 position)
{
    if (!positionSlider->isSliderDown()) {
        positionSlider->setValue(position);
    }
    if (!fullscreenProgressSlider->isSliderDown()) {
        fullscreenProgressSlider->setValue(position);
    }
}

void MainWindow::onDurationChanged(qint64 duration)
{
    positionSlider->setRange(0, duration);
    fullscreenProgressSlider->setRange(0, duration);
}

void MainWindow::onPlaylistPositionChanged(int position)
{
    if (position >= 0 && position < musicLibrary->audioFiles().size()) {
        QString filePath = musicLibrary->audioFiles()[position];
        qDebug() << "Playing file:" << filePath;
        
        // Set the media source with error handling
        QUrl url = QUrl::fromLocalFile(filePath);
        if (url.isValid()) {
            mediaPlayer->setSource(url);
            // Don't auto-play, just update metadata
            updateMetadata();     // Update metadata immediately
        } else {
            qDebug() << "Invalid file URL:" << filePath;
            nowPlayingLabel->setText("Error: Invalid file");
            miniArtistLabel->setText("");
        }
    }
}

void MainWindow::onAudioFilesChanged(const QStringList& files)
{
    tracksList->clear();
    qDebug() << "Updating tracks list with" << files.size() << "files";

    for (const QString &filePath : files) {
        QString fileName = QFileInfo(filePath).fileName();
        tracksList->addItem(fileName);
    }
    
    // If we have files but no selection, select the first one
    if (tracksList->count() > 0 && tracksList->currentRow() < 0) {
        tracksList->setCurrentRow(0);
    }
}

void MainWindow::updatePlayPauseButton()
{
    QIcon playIcon = style()->standardIcon(QStyle::SP_MediaPlay);
    QIcon pauseIcon = style()->standardIcon(QStyle::SP_MediaPause);
    
    if (mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        playPauseButton->setIcon(pauseIcon);
        fullscreenPlayPauseButton->setIcon(pauseIcon);
    } else {
        playPauseButton->setIcon(playIcon);
        fullscreenPlayPauseButton->setIcon(playIcon);
    }
}

QImage MainWindow::extractAlbumArt(const QString &filePath)
{
    QImage albumArt;
    TagLib::FileRef file(filePath.toUtf8().constData());
    
    if (!file.isNull()) {
        TagLib::Tag *tag = file.tag();
        if (!tag) return albumArt;

        // Try to get the cover art based on file type
        if (TagLib::MPEG::File *mpegFile = dynamic_cast<TagLib::MPEG::File*>(file.file())) {
            if (mpegFile->ID3v2Tag()) {
                TagLib::ID3v2::FrameList frames = mpegFile->ID3v2Tag()->frameList("APIC");
                if (!frames.isEmpty()) {
                    TagLib::ID3v2::AttachedPictureFrame *frame = 
                        dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(frames.front());
                    if (frame) {
                        albumArt.loadFromData((const uchar*)frame->picture().data(), 
                                           frame->picture().size());
                    }
                }
            }
        }
        else if (TagLib::FLAC::File *flacFile = dynamic_cast<TagLib::FLAC::File*>(file.file())) {
            const TagLib::List<TagLib::FLAC::Picture*>& pictures = flacFile->pictureList();
            if (!pictures.isEmpty()) {
                TagLib::FLAC::Picture* picture = pictures.front();
                albumArt.loadFromData((const uchar*)picture->data().data(), 
                                   picture->data().size());
            }
        }
        else if (TagLib::MP4::File *mp4File = dynamic_cast<TagLib::MP4::File*>(file.file())) {
            TagLib::MP4::Tag *mp4Tag = mp4File->tag();
            if (mp4Tag) {
                const TagLib::MP4::ItemMap& itemsMap = mp4Tag->itemMap();
                if (itemsMap.contains("covr")) {
                    const TagLib::MP4::CoverArtList& coverArtList = 
                        itemsMap["covr"].toCoverArtList();
                    if (!coverArtList.isEmpty()) {
                        albumArt.loadFromData((const uchar*)coverArtList[0].data().data(), 
                                           coverArtList[0].data().size());
                    }
                }
            }
        }
    }
    
    return albumArt;
}

void MainWindow::updateMetadata()
{
    try {
        QString filePath = mediaPlayer->source().toLocalFile();
        TagLib::FileRef file(filePath.toUtf8().constData());
        
        if (!file.isNull()) {
            TagLib::Tag *tag = file.tag();
            if (tag) {
                // Get basic metadata
                QString title = QString::fromStdString(tag->title().toCString(true));
                QString artist = QString::fromStdString(tag->artist().toCString(true));
                QString album = QString::fromStdString(tag->album().toCString(true));
                
                // Set fallback values if metadata is empty
                if (title.isEmpty()) {
                    QFileInfo fileInfo(filePath);
                    title = fileInfo.baseName();
                }
                if (artist.isEmpty()) {
                    artist = "Unknown Artist";
                }
                if (album.isEmpty()) {
                    album = "Unknown Album";
                }
                
                // Update mini player labels
                titleLabel->setText(title);
                artistLabel->setText(artist);
                // Don't update albumLabel text
                nowPlayingLabel->setText(title);
                miniArtistLabel->setText(artist);
                
                // Update fullscreen player labels
                fullscreenTitleLabel->setText(title);
                fullscreenArtistLabel->setText(artist);
                
                // Get album art
                QImage albumArt = extractAlbumArt(filePath);
                if (!albumArt.isNull()) {
                    QPixmap pixmap = QPixmap::fromImage(albumArt);
                    albumArtLabel->setPixmap(pixmap.scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    miniAlbumArt->setPixmap(pixmap.scaled(50, 50, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                    
                    // Update fullscreen album art
                    int albumSize = qMin(fullscreenPlayer->width(), fullscreenPlayer->height()) / 2;
                    fullscreenAlbumArt->setPixmap(pixmap.scaled(albumSize, albumSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                } else {
                    albumArtLabel->setText("No Album Art");
                    miniAlbumArt->setText("No Art");
                    fullscreenAlbumArt->setText("No Album Art");
                }
            }
        }
    } catch (const std::exception& e) {
        qDebug() << "Error updating metadata:" << e.what();
        // Set safe fallback values
        titleLabel->setText("Unknown Title");
        artistLabel->setText("Unknown Artist");
        // Don't update albumLabel text
        nowPlayingLabel->setText("Unknown Title");
        miniArtistLabel->setText("Unknown Artist");
        albumArtLabel->setText("No Album Art");
        miniAlbumArt->setText("No Art");
        
        // Set safe fallback values for fullscreen player
        fullscreenTitleLabel->setText("Unknown Title");
        fullscreenArtistLabel->setText("Unknown Artist");
        fullscreenAlbumArt->setText("No Album Art");
    }
}

void MainWindow::onItemDoubleClicked(QListWidgetItem *item)
{
    // Start playing the track from the beginning
    mediaPlayer->setPosition(0);
    mediaPlayer->play();
}

void MainWindow::onMiniPlayerClicked()
{
    if (fullscreenPlayer->isVisible()) {
        hideFullscreenPlayer();
    } else {
        showFullscreenPlayer();
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    
    // If the fullscreen player is visible, update its size
    if (fullscreenPlayer->isVisible()) {
        fullscreenPlayer->setGeometry(0, 0, width(), height());
        
        // Calculate the album art size based on the window size
        int albumSize = qMin(width(), height()) / 2;
        
        // Set both minimum and maximum size for the album art
        fullscreenAlbumArt->setMinimumSize(albumSize, albumSize);
        fullscreenAlbumArt->setMaximumSize(albumSize, albumSize);
        
        // Get the current file path and extract album art at full resolution
        QString currentFile = mediaPlayer->source().toLocalFile();
        QImage albumArt = extractAlbumArt(currentFile);
        
        if (!albumArt.isNull()) {
            // Create a pixmap with transparency
            QPixmap scaledPixmap(albumSize, albumSize);
            scaledPixmap.fill(Qt::transparent);
            
            // Create a painter to draw the scaled image
            QPainter painter(&scaledPixmap);
            painter.setRenderHint(QPainter::SmoothPixmapTransform);
            
            // Scale the image maintaining aspect ratio
            QImage scaledImage = albumArt.scaled(
                albumSize, 
                albumSize,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            );
            
            // Calculate position to center the image
            int x = (albumSize - scaledImage.width()) / 2;
            int y = (albumSize - scaledImage.height()) / 2;
            
            // Draw the image centered
            painter.drawImage(x, y, scaledImage);
            painter.end();
            
            // Set the pixmap
            fullscreenAlbumArt->setPixmap(scaledPixmap);
        }
    }
} 