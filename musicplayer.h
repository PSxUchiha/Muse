#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include <QObject>
#include <QUrl>
#include <QMediaPlayer>
#include <QAudioOutput>

class MusicPlayer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentSong READ currentSong NOTIFY currentSongChanged)
    Q_PROPERTY(QString currentArtist READ currentArtist NOTIFY currentArtistChanged)
    Q_PROPERTY(QUrl currentArtwork READ currentArtwork NOTIFY currentArtworkChanged)
    Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY playbackStateChanged)
    Q_PROPERTY(qint64 position READ position NOTIFY positionChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)

public:
    explicit MusicPlayer(QObject *parent = nullptr);
    ~MusicPlayer();

    QString currentSong() const;
    QString currentArtist() const;
    QUrl currentArtwork() const;
    bool isPlaying() const;
    qint64 position() const;
    qint64 duration() const;

public slots:
    void play();
    void pause();
    void stop();
    void seek(qint64 position);
    void setSource(const QUrl &url);
    void setVolume(float volume);

signals:
    void currentSongChanged();
    void currentArtistChanged();
    void currentArtworkChanged();
    void playbackStateChanged();
    void positionChanged();
    void durationChanged();
    void error(const QString &message);

private:
    QMediaPlayer *m_player;
    QString m_currentSong;
    QString m_currentArtist;
    QUrl m_currentArtwork;
};

#endif // MUSICPLAYER_H 