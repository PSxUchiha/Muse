#include "miniplayer.h"
#include <QStyle>
#include <QApplication>

MiniPlayer::MiniPlayer(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(80);
    setStyleSheet(QString("background-color: %1;").arg(Theme::BackgroundColor));

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 5, 10, 5);
    layout->setSpacing(10);

    // Album art
    albumArt = new QLabel(this);
    albumArt->setFixedSize(60, 60);
    albumArt->setStyleSheet("background-color: transparent;");
    layout->addWidget(albumArt);

    // Track info
    auto *infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(2);
    
    titleLabel = new QLabel(this);
    titleLabel->setStyleSheet(QString("color: %1; font-size: 24pt;").arg(Theme::TextColor));
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    infoLayout->addWidget(titleLabel);

    artistLabel = new QLabel(this);
    artistLabel->setStyleSheet(QString("color: %1; font-size: 12pt;").arg(Theme::SecondaryTextColor));
    artistLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    infoLayout->addWidget(artistLabel);

    layout->addLayout(infoLayout, 1);

    // Playback controls
    auto *controlsLayout = new QHBoxLayout();
    controlsLayout->setSpacing(10);

    previousButton = new QPushButton(this);
    previousButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    previousButton->setStyleSheet(QString("QPushButton { color: %1; border: none; }").arg(Theme::TextColor));
    controlsLayout->addWidget(previousButton);

    playPauseButton = new QPushButton(this);
    playPauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    playPauseButton->setStyleSheet(QString("QPushButton { color: %1; border: none; }").arg(Theme::TextColor));
    controlsLayout->addWidget(playPauseButton);

    nextButton = new QPushButton(this);
    nextButton->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    nextButton->setStyleSheet(QString("QPushButton { color: %1; border: none; }").arg(Theme::TextColor));
    controlsLayout->addWidget(nextButton);

    layout->addLayout(controlsLayout);

    // Position slider and time
    auto *sliderLayout = new QVBoxLayout();
    sliderLayout->setSpacing(2);

    positionSlider = new QSlider(Qt::Horizontal, this);
    positionSlider->setStyleSheet(QString(
        "QSlider::groove:horizontal {"
        "    border: 1px solid %1;"
        "    height: 4px;"
        "    background: %2;"
        "    margin: 0px;"
        "}"
        "QSlider::handle:horizontal {"
        "    background: %1;"
        "    border: none;"
        "    width: 12px;"
        "    margin: -4px 0;"
        "    border-radius: 6px;"
        "}"
    ).arg(Theme::AccentColor).arg(Theme::SecondaryBackgroundColor));
    sliderLayout->addWidget(positionSlider);

    timeLabel = new QLabel("0:00 / 0:00", this);
    timeLabel->setStyleSheet(QString("color: %1; font-size: 10pt;").arg(Theme::SecondaryTextColor));
    timeLabel->setAlignment(Qt::AlignRight);
    sliderLayout->addWidget(timeLabel);

    layout->addLayout(sliderLayout, 1);

    // Connect signals
    connect(playPauseButton, &QPushButton::clicked, this, &MiniPlayer::playPauseClicked);
    connect(nextButton, &QPushButton::clicked, this, &MiniPlayer::nextClicked);
    connect(previousButton, &QPushButton::clicked, this, &MiniPlayer::previousClicked);
    connect(positionSlider, &QSlider::sliderPressed, this, &MiniPlayer::positionSliderPressed);
    connect(positionSlider, &QSlider::sliderReleased, this, &MiniPlayer::positionSliderReleased);
    connect(positionSlider, &QSlider::sliderMoved, this, &MiniPlayer::positionSliderMoved);
}

MiniPlayer::~MiniPlayer()
{
}

void MiniPlayer::updateMetadata(const QString &title, const QString &artist, const QImage &albumArtImage)
{
    titleLabel->setText(title);
    artistLabel->setText(artist);

    if (!albumArtImage.isNull()) {
        QPixmap pixmap = QPixmap::fromImage(albumArtImage);
        pixmap = pixmap.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        
        // Create a rounded rectangle mask
        QPixmap roundedPixmap(pixmap.size());
        roundedPixmap.fill(Qt::transparent);
        
        QPainter painter(&roundedPixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        
        QPainterPath path;
        path.addRoundedRect(0, 0, pixmap.width(), pixmap.height(), 5, 5);
        painter.setClipPath(path);
        painter.drawPixmap(0, 0, pixmap);
        
        albumArt->setPixmap(roundedPixmap);
    } else {
        // Set a default icon or clear the album art
        albumArt->setPixmap(QPixmap());
    }
}

void MiniPlayer::updatePlayPauseButton(bool isPlaying)
{
    playPauseButton->setIcon(style()->standardIcon(isPlaying ? QStyle::SP_MediaPause : QStyle::SP_MediaPlay));
}

void MiniPlayer::updatePosition(qint64 position)
{
    if (!positionSlider->isSliderDown()) {
        positionSlider->setValue(position);
    }
    
    QTime currentTime((position / 3600000) % 60, (position / 60000) % 60, (position / 1000) % 60);
    QTime totalTime((positionSlider->maximum() / 3600000) % 60, (positionSlider->maximum() / 60000) % 60, (positionSlider->maximum() / 1000) % 60);
    
    timeLabel->setText(currentTime.toString("m:ss") + " / " + totalTime.toString("m:ss"));
}

void MiniPlayer::updateDuration(qint64 duration)
{
    positionSlider->setRange(0, duration);
}

void MiniPlayer::setPlaybackState(bool isPlaying)
{
    updatePlayPauseButton(isPlaying);
}

void MiniPlayer::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit miniPlayerClicked();
    }
    QWidget::mousePressEvent(event);
} 