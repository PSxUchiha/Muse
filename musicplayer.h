#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include <QObject>
#include <QUrl>
#include <QMediaPlayer>
#include <QAudioOutput>
#include "common.h"

class MusicPlayer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentSong READ currentSong NOTIFY currentSongChanged)
    Q_PROPERTY(QString currentArtist READ currentArtist NOTIFY currentArtistChanged)
    Q_PROPERTY(QUrl currentArtwork READ currentArtwork NOTIFY currentArtworkChanged)
    Q_PROPERTY(PlaybackState state READ state NOTIFY stateChanged)
    Q_PROPERTY(qint64 position READ position NOTIFY positionChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)

public:
    explicit MusicPlayer(QObject *parent = nullptr);
    ~MusicPlayer();

    QString currentSong() const { return m_currentSong; }
    QString currentArtist() const { return m_currentArtist; }
    QUrl currentArtwork() const { return m_currentArtwork; }
    PlaybackState state() const;
    qint64 position() const;
    qint64 duration() const;
    int volume() const;

public slots:
    void play();
    void pause();
    void stop();
    void seek(qint64 position);
    void setSource(const QUrl &url);
    void setVolume(int volume);

signals:
    void currentSongChanged();
    void currentArtistChanged();
    void currentArtworkChanged();
    void stateChanged(PlaybackState newState);
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void volumeChanged(int volume);
    void error(const QString &message);

private:
    QMediaPlayer *m_player;
    QAudioOutput *m_audioOutput;
    QString m_currentSong;
    QString m_currentArtist;
    QUrl m_currentArtwork;
};

#endif // MUSICPLAYER_H 