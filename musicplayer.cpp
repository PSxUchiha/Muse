#include "musicplayer.h"
#include <QMediaMetaData>

MusicPlayer::MusicPlayer(QObject *parent)
    : QObject(parent)
    , m_player(new QMediaPlayer(this))
{
    connect(m_player, &QMediaPlayer::stateChanged, this, &MusicPlayer::playbackStateChanged);
    connect(m_player, &QMediaPlayer::positionChanged, this, &MusicPlayer::positionChanged);
    connect(m_player, &QMediaPlayer::durationChanged, this, &MusicPlayer::durationChanged);
    connect(m_player, static_cast<void(QMediaPlayer::*)(QMediaPlayer::Error)>(&QMediaPlayer::error),
            this, [this](QMediaPlayer::Error error) {
        emit this->error(m_player->errorString());
    });

    connect(m_player, &QMediaPlayer::metaDataAvailableChanged, this, [this](bool available) {
        if (available) {
            QString title = m_player->metaData(QMediaMetaData::Title).toString();
            if (title != m_currentSong) {
                m_currentSong = title;
                emit currentSongChanged();
            }

            QString artist = m_player->metaData(QMediaMetaData::AlbumArtist).toString();
            if (artist != m_currentArtist) {
                m_currentArtist = artist;
                emit currentArtistChanged();
            }

            QUrl artwork = m_player->metaData(QMediaMetaData::CoverArtUrlLarge).toUrl();
            if (artwork != m_currentArtwork) {
                m_currentArtwork = artwork;
                emit currentArtworkChanged();
            }
        }
    });
}

MusicPlayer::~MusicPlayer()
{
}

QString MusicPlayer::currentSong() const
{
    return m_currentSong.isEmpty() ? "No Song Playing" : m_currentSong;
}

QString MusicPlayer::currentArtist() const
{
    return m_currentArtist.isEmpty() ? "Unknown Artist" : m_currentArtist;
}

QUrl MusicPlayer::currentArtwork() const
{
    return m_currentArtwork;
}

bool MusicPlayer::isPlaying() const
{
    return m_player->state() == QMediaPlayer::PlayingState;
}

qint64 MusicPlayer::position() const
{
    return m_player->position();
}

qint64 MusicPlayer::duration() const
{
    return m_player->duration();
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
    m_player->setMedia(url);
}

void MusicPlayer::setVolume(float volume)
{
    m_player->setVolume(qRound(volume * 100));
} 