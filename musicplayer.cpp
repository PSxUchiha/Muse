#include "musicplayer.h"
#include <QMediaMetaData>

MusicPlayer::MusicPlayer(QObject *parent)
    : QObject(parent)
    , m_player(new QMediaPlayer(this))
    , m_audioOutput(new QAudioOutput(this))
{
    m_player->setAudioOutput(m_audioOutput);
    
    // Connect player signals
    connect(m_player, &QMediaPlayer::playbackStateChanged,
            this, [this](QMediaPlayer::PlaybackState state) {
                switch (state) {
                    case QMediaPlayer::PlaybackState::PlayingState:
                        emit stateChanged(PlaybackState::Playing);
                        break;
                    case QMediaPlayer::PlaybackState::PausedState:
                        emit stateChanged(PlaybackState::Paused);
                        break;
                    case QMediaPlayer::PlaybackState::StoppedState:
                        emit stateChanged(PlaybackState::Stopped);
                        break;
                }
            });

    connect(m_player, &QMediaPlayer::positionChanged,
            this, &MusicPlayer::positionChanged);
    
    connect(m_player, &QMediaPlayer::durationChanged,
            this, &MusicPlayer::durationChanged);

    connect(m_player, &QMediaPlayer::errorOccurred,
            this, [this](QMediaPlayer::Error error, const QString &errorString) {
                emit this->error(errorString);
            });

    connect(m_player, &QMediaPlayer::metaDataChanged,
            this, [this]() {
                const QMediaMetaData metaData = m_player->metaData();
                
                QString title = metaData.stringValue(QMediaMetaData::Title);
                if (!title.isEmpty() && title != m_currentSong) {
                    m_currentSong = title;
                    emit currentSongChanged();
                }

                QString artist = metaData.stringValue(QMediaMetaData::AlbumArtist);
                if (artist.isEmpty()) {
                    artist = metaData.stringValue(QMediaMetaData::ContributingArtist);
                }
                if (!artist.isEmpty() && artist != m_currentArtist) {
                    m_currentArtist = artist;
                    emit currentArtistChanged();
                }

                QUrl artwork = metaData.value(QMediaMetaData::ThumbnailImage).toUrl();
                if (artwork.isEmpty()) {
                    artwork = metaData.value(QMediaMetaData::CoverArtImage).toUrl();
                }
                if (!artwork.isEmpty() && artwork != m_currentArtwork) {
                    m_currentArtwork = artwork;
                    emit currentArtworkChanged();
                }
            });

    // Set initial volume
    m_audioOutput->setVolume(0.5); // 50%
}

MusicPlayer::~MusicPlayer()
{
    stop();
}

PlaybackState MusicPlayer::state() const
{
    switch (m_player->playbackState()) {
        case QMediaPlayer::PlaybackState::PlayingState:
            return PlaybackState::Playing;
        case QMediaPlayer::PlaybackState::PausedState:
            return PlaybackState::Paused;
        default:
            return PlaybackState::Stopped;
    }
}

qint64 MusicPlayer::position() const
{
    return m_player->position();
}

qint64 MusicPlayer::duration() const
{
    return m_player->duration();
}

int MusicPlayer::volume() const
{
    return static_cast<int>(m_audioOutput->volume() * 100);
}

void MusicPlayer::play()
{
    m_player->play();
}

void MusicPlayer::pause()
{
    m_player->pause();
}

void MusicPlayer::stop()
{
    m_player->stop();
}

void MusicPlayer::seek(qint64 position)
{
    m_player->setPosition(position);
}

void MusicPlayer::setSource(const QUrl &url)
{
    m_player->setSource(url);
}

void MusicPlayer::setVolume(int volume)
{
    m_audioOutput->setVolume(volume / 100.0);
    emit volumeChanged(volume);
} 