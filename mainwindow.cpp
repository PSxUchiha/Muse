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
#include <QPainterPath>
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
    mainContent->setStyleSheet("QWidget { background: transparent; border: none; }");
    QVBoxLayout *mainContentLayout = new QVBoxLayout(mainContent);
    mainContentLayout->setContentsMargins(0, 0, 0, 0);
    mainContentLayout->setSpacing(0);

    // Setup pages
    setupPages();
    pages->setStyleSheet(Theme::MAIN_WINDOW_STYLE + "QWidget { border: none; }");
    mainContentLayout->addWidget(pages);

    contentLayout->addWidget(mainContent);
    mainLayout->addLayout(contentLayout);

    // Create mini player
    miniPlayer = new QWidget(this);
    miniPlayer->setFixedHeight(80);
    miniPlayer->setStyleSheet(Theme::MINI_PLAYER_STYLE + "QWidget { border: none; }");

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
    previousButton->setStyleSheet(Theme::BUTTON_STYLE + "QPushButton { border: none; }");
    miniControlsLayout->addWidget(previousButton);

    playPauseButton = new QPushButton(miniPlayer);
    playPauseButton->setIcon(QIcon::fromTheme("media-playback-start"));
    playPauseButton->setFixedSize(32, 32);
    playPauseButton->setStyleSheet(Theme::BUTTON_STYLE + "QPushButton { border: none; }");
    miniControlsLayout->addWidget(playPauseButton);

    nextButton = new QPushButton(miniPlayer);
    nextButton->setIcon(QIcon::fromTheme("media-skip-forward"));
    nextButton->setFixedSize(32, 32);
    nextButton->setStyleSheet(Theme::BUTTON_STYLE + "QPushButton { border: none; }");
    miniControlsLayout->addWidget(nextButton);

    miniLayout->addWidget(miniControls);

    // Mini player album art
    QWidget *miniArtContainer = new QWidget(miniPlayer);
    miniArtContainer->setFixedSize(60, 60);
    miniArtContainer->setStyleSheet("QWidget { background: transparent; border: none; }");
    QVBoxLayout *miniArtLayout = new QVBoxLayout(miniArtContainer);
    miniArtLayout->setContentsMargins(0, 0, 0, 0);
    miniArtLayout->setSpacing(0);

    miniAlbumArt = new QLabel(miniArtContainer);
    miniAlbumArt->setFixedSize(60, 60);
    miniAlbumArt->setStyleSheet("QLabel { background-color: transparent; border: none; border-radius: 5px; }");
    miniAlbumArt->setAlignment(Qt::AlignCenter);
    miniArtLayout->addWidget(miniAlbumArt);

    miniLayout->addWidget(miniArtContainer);

    // Mini player info
    QWidget *miniInfo = new QWidget(miniPlayer);
    QVBoxLayout *miniInfoLayout = new QVBoxLayout(miniInfo);
    miniInfoLayout->setContentsMargins(0, 0, 0, 0);
    miniInfoLayout->setSpacing(2);

    miniTitleLabel = new QLabel(miniPlayer);
    miniTitleLabel->setStyleSheet(Theme::LABEL_STYLE + "font-size: 24pt; font-weight: bold; border: none;");
    miniTitleLabel->setAlignment(Qt::AlignCenter);
    miniInfoLayout->addWidget(miniTitleLabel);

    miniArtistLabel = new QLabel(miniPlayer);
    miniArtistLabel->setStyleSheet(Theme::LABEL_STYLE + "color: palette(mid); border: none;");
    miniInfoLayout->addWidget(miniArtistLabel);
    
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

    // Add back button at the top
    QWidget *topBar = new QWidget(fullscreenPlayer);
    QHBoxLayout *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(0, 0, 0, 0);
    topBarLayout->setSpacing(10);

    QPushButton *backButton = new QPushButton(fullscreenPlayer);
    backButton->setIcon(QIcon::fromTheme("go-previous"));
    backButton->setFixedSize(32, 32);
    backButton->setStyleSheet(Theme::BUTTON_STYLE + "QPushButton { border: none; }");
    backButton->setToolTip("Back to main screen");
    connect(backButton, &QPushButton::clicked, this, &MainWindow::hideFullscreenPlayer);
    topBarLayout->addWidget(backButton);
    topBarLayout->addStretch();

    fullscreenLayout->addWidget(topBar);

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

    // Set albums page as default
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
    pages->setStyleSheet(Theme::MAIN_WINDOW_STYLE + "QWidget { border: none; }");
    pages->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Albums page (now main page)
    albumsPage = new QWidget;
    albumsPage->setStyleSheet("QWidget { background: transparent; border: none; }");
    QVBoxLayout *albumsLayout = new QVBoxLayout(albumsPage);
    albumsLayout->setContentsMargins(0, 0, 0, 0);
    albumsList = new QListWidget;
    albumsList->setStyleSheet(Theme::LIST_WIDGET_STYLE + "QListWidget { border: none; }");
    albumsLayout->addWidget(albumsList);
    pages->addWidget(albumsPage);
    
    // Initialize playlistWidget to albumsList
    playlistWidget = albumsList;

    // Tracks page (now secondary)
    tracksPage = new QWidget;
    tracksPage->setStyleSheet("QWidget { background: transparent; border: none; }");
    QVBoxLayout *tracksLayout = new QVBoxLayout(tracksPage);
    tracksLayout->setContentsMargins(0, 0, 0, 0);
    tracksList = new QListWidget;
    tracksList->setStyleSheet(Theme::LIST_WIDGET_STYLE + "QListWidget { border: none; }");
    tracksLayout->addWidget(tracksList);
    pages->addWidget(tracksPage);

    // Artists page
    artistsPage = new QWidget;
    artistsPage->setStyleSheet("QWidget { background: transparent; border: none; }");
    QVBoxLayout *artistsLayout = new QVBoxLayout(artistsPage);
    artistsLayout->setContentsMargins(0, 0, 0, 0);
    artistsList = new QListWidget;
    artistsList->setStyleSheet(Theme::LIST_WIDGET_STYLE + "QListWidget { border: none; }");
    artistsLayout->addWidget(artistsList);
    pages->addWidget(artistsPage);

    // Playlists page
    playlistsPage = new QWidget;
    playlistsPage->setStyleSheet("QWidget { background: transparent; border: none; }");
    QVBoxLayout *playlistsLayout = new QVBoxLayout(playlistsPage);
    playlistsLayout->setContentsMargins(0, 0, 0, 0);
    playlistsList = new QListWidget;
    playlistsList->setStyleSheet(Theme::LIST_WIDGET_STYLE + "QListWidget { border: none; }");
    playlistsLayout->addWidget(playlistsList);
    pages->addWidget(playlistsPage);

    // Set albums page as default
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
    
    // Store the original window size before going fullscreen
    originalWindowSize = size();
    
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
    fullscreenAnimation->setDuration(150);  // Reduced from 300ms to 150ms
    fullscreenAnimation->setStartValue(1.0);
    fullscreenAnimation->setEndValue(0.0);
    connect(fullscreenAnimation, &QPropertyAnimation::finished, [this]() {
        // Switch back to the tracks page (index 0)
        pages->setCurrentIndex(0);
        
        // Show the mini player again
        miniPlayer->show();
        
        // Restore the original window size
        resize(originalWindowSize);
        
        // Reset size constraints
        setMinimumSize(400, 300);
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
    if (position >= 0) {
        QString filePath;
        if (playlistWidget == albumsList) {
            // If we're in the albums view, get the first track of the selected album
            QListWidgetItem *albumItem = albumsList->item(position);
            if (albumItem) {
                QStringList albumFiles = albumItem->data(Qt::UserRole).toStringList();
                if (!albumFiles.isEmpty()) {
                    filePath = albumFiles.first();
                }
            }
        } else {
            // If we're in the tracks view, get the selected track
            if (position < musicLibrary->audioFiles().size()) {
                filePath = musicLibrary->audioFiles()[position];
            }
        }

        if (!filePath.isEmpty()) {
            QUrl url = QUrl::fromLocalFile(filePath);
            if (url.isValid()) {
                mediaPlayer->setSource(url);
                updateMetadata();
            }
        }
    }
}

void MainWindow::onAudioFilesChanged(const QStringList& files)
{
    // Clear both lists
    albumsList->clear();
    tracksList->clear();
    
    // Group files by album
    QMap<QString, QStringList> albumMap;
    for (const QString &filePath : files) {
        TagLib::FileRef file(filePath.toUtf8().constData());
        if (!file.isNull()) {
            TagLib::Tag *tag = file.tag();
            if (tag) {
                QString album = QString::fromStdString(tag->album().toCString(true));
                if (album.isEmpty()) {
                    album = "Unknown Album";
                }
                albumMap[album].append(filePath);
            }
        }
    }

    // Add albums to the albums list
    for (auto it = albumMap.begin(); it != albumMap.end(); ++it) {
        QListWidgetItem *item = new QListWidgetItem(it.key());
        item->setData(Qt::UserRole, it.value()); // Store the file paths
        albumsList->addItem(item);
    }

    // If we have albums but no selection, select the first one
    if (albumsList->count() > 0 && albumsList->currentRow() < 0) {
        albumsList->setCurrentRow(0);
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
                miniTitleLabel->setText(title);
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
        miniTitleLabel->setText("Unknown Title");
        miniArtistLabel->setText("Unknown Artist");
        // Don't update albumLabel text
        fullscreenTitleLabel->setText("Unknown Title");
        fullscreenArtistLabel->setText("Unknown Artist");
        albumArtLabel->setText("No Album Art");
        miniAlbumArt->setText("No Art");
        
        // Set safe fallback values for fullscreen player
        fullscreenAlbumArt->setText("No Album Art");
    }
}

void MainWindow::onItemDoubleClicked(QListWidgetItem *item)
{
    if (item->listWidget() == albumsList) {
        // Get the list of files for this album
        QStringList albumFiles = item->data(Qt::UserRole).toStringList();
        
        // Clear the tracks list and add all tracks from the album
        tracksList->clear();
        for (const QString &filePath : albumFiles) {
            QFileInfo fileInfo(filePath);
            QString fileName = fileInfo.fileName();
            QListWidgetItem *trackItem = new QListWidgetItem(fileName);
            trackItem->setData(Qt::UserRole, filePath);
            tracksList->addItem(trackItem);
        }
        
        // Start playing the first track
        if (tracksList->count() > 0) {
            tracksList->setCurrentRow(0);
            QString firstTrack = tracksList->item(0)->data(Qt::UserRole).toString();
            mediaPlayer->setSource(QUrl::fromLocalFile(firstTrack));
            mediaPlayer->play();
        }
    } else {
        // Handle track item click (existing behavior)
        mediaPlayer->setPosition(0);
        mediaPlayer->play();
    }
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

void MainWindow::updateNowPlayingInfo()
{
    const QMediaMetaData metaData = mediaPlayer->metaData();
    
    // Get title
    QString title = metaData.stringValue(QMediaMetaData::Title);
    if (title.isEmpty()) {
        QFileInfo fileInfo(mediaPlayer->source().toLocalFile());
        title = fileInfo.baseName();
    }
    
    // Get artist
    QString artist = metaData.stringValue(QMediaMetaData::AlbumArtist);
    if (artist.isEmpty()) {
        artist = metaData.stringValue(QMediaMetaData::ContributingArtist);
    }
    if (artist.isEmpty()) {
        artist = "Unknown Artist";
    }

    // Update labels
    miniTitleLabel->setText(title);
    miniArtistLabel->setText(artist);
    fullscreenTitleLabel->setText(title);
    fullscreenArtistLabel->setText(artist);

    // Handle album art
    QImage albumArt;
    QVariant artData = metaData.value(QMediaMetaData::ThumbnailImage);
    if (!artData.isValid()) {
        artData = metaData.value(QMediaMetaData::CoverArtImage);
    }
    
    if (artData.isValid()) {
        albumArt = artData.value<QImage>();
    }

    if (!albumArt.isNull()) {
        // Mini player album art
        QPixmap miniArtPixmap = QPixmap::fromImage(albumArt)
            .scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        
        // Create a rounded version of the mini art
        QPixmap roundedMiniArt(60, 60);
        roundedMiniArt.fill(Qt::transparent);
        
        QPainter painter(&roundedMiniArt);
        painter.setRenderHint(QPainter::Antialiasing);
        
        QPainterPath path;
        path.addRoundedRect(0, 0, 60, 60, 5, 5);
        painter.setClipPath(path);
        
        // Center the artwork in the rounded frame
        int x = (60 - miniArtPixmap.width()) / 2;
        int y = (60 - miniArtPixmap.height()) / 2;
        painter.drawPixmap(x, y, miniArtPixmap);
        
        miniAlbumArt->setPixmap(roundedMiniArt);

        // Fullscreen album art
        int fullscreenSize = qMin(fullscreenAlbumArt->width(), fullscreenAlbumArt->height());
        QPixmap fullscreenArtPixmap = QPixmap::fromImage(albumArt)
            .scaled(fullscreenSize, fullscreenSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        
        // Create a rounded version of the fullscreen art
        QPixmap roundedFullscreenArt(fullscreenSize, fullscreenSize);
        roundedFullscreenArt.fill(Qt::transparent);
        
        QPainter fullscreenPainter(&roundedFullscreenArt);
        fullscreenPainter.setRenderHint(QPainter::Antialiasing);
        
        QPainterPath fullscreenPath;
        fullscreenPath.addRoundedRect(0, 0, fullscreenSize, fullscreenSize, 10, 10);
        fullscreenPainter.setClipPath(fullscreenPath);
        
        // Center the artwork in the rounded frame
        x = (fullscreenSize - fullscreenArtPixmap.width()) / 2;
        y = (fullscreenSize - fullscreenArtPixmap.height()) / 2;
        fullscreenPainter.drawPixmap(x, y, fullscreenArtPixmap);
        
        fullscreenAlbumArt->setPixmap(roundedFullscreenArt);
    } else {
        // Set placeholder images
        miniAlbumArt->setPixmap(QIcon::fromTheme("media-optical-audio").pixmap(60, 60));
        fullscreenAlbumArt->setPixmap(QIcon::fromTheme("media-optical-audio").pixmap(200, 200));
    }
} 