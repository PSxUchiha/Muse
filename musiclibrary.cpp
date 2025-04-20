#include "musiclibrary.h"
#include <QStandardPaths>
#include <QDirIterator>
#include <QDebug>
#include <QMimeDatabase>
#include <QMimeType>
#include <QDir>
#include <QFileInfo>

MusicLibrary::MusicLibrary(QObject *parent)
    : QObject(parent)
    , m_isLoading(false)
    , m_watcher(new QFileSystemWatcher(this))
{
    // Add supported MIME types for audio files
    m_supportedFormats << "audio/mpeg"      // MP3
                      << "audio/mp4"        // M4A
                      << "audio/ogg"        // OGG
                      << "audio/flac"       // FLAC
                      << "audio/wav"        // WAV
                      << "audio/x-wav"      // WAV (alternative MIME type)
                      << "audio/x-flac";    // FLAC (alternative MIME type)
}

MusicLibrary::~MusicLibrary()
{
}

QStringList MusicLibrary::audioFiles() const
{
    return m_audioFiles;
}

bool MusicLibrary::isLoading() const
{
    return m_isLoading;
}

void MusicLibrary::scanMusicDirectory()
{
    qDebug() << "Starting music directory scan...";
    m_audioFiles.clear();

    // Get standard music locations
    QStringList musicDirs = QStandardPaths::standardLocations(QStandardPaths::MusicLocation);
    qDebug() << "Music directories found:" << musicDirs;

    // Scan each directory
    for (const QString &dir : musicDirs) {
        addDirectory(dir);
    }

    // Emit signal with updated files
    emit audioFilesChanged(m_audioFiles);
}

void MusicLibrary::addDirectory(const QString& path)
{
    QDir dir(path);
    QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QFileInfo &entry : entries) {
        if (entry.isDir()) {
            addDirectory(entry.filePath());
        } else if (isAudioFile(entry.filePath())) {
            qDebug() << "Found audio file:" << entry.filePath();
            m_audioFiles.append(entry.filePath());
        }
    }
}

QString MusicLibrary::getFileName(const QString& filePath) const
{
    return QFileInfo(filePath).fileName();
}

QString MusicLibrary::getFileExtension(const QString &filePath) const
{
    QFileInfo fileInfo(filePath);
    return fileInfo.suffix().toLower();
}

void MusicLibrary::setAudioFiles(const QStringList& files)
{
    if (m_audioFiles != files) {
        m_audioFiles = files;
        emit audioFilesChanged(m_audioFiles);
    }
}

void MusicLibrary::setIsLoading(bool loading)
{
    if (m_isLoading != loading) {
        m_isLoading = loading;
        emit isLoadingChanged();
    }
}

bool MusicLibrary::isAudioFile(const QString& filePath) const
{
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(filePath);
    QString extension = QFileInfo(filePath).suffix().toLower();
    
    qDebug() << "Checking file:" << filePath;
    qDebug() << "MIME type:" << mime.name();
    qDebug() << "Extension:" << extension;

    // List of supported audio file extensions
    static const QStringList supportedExtensions = {
        "mp3", "m4a", "aac", "ogg", "wav", "flac", "wma"
    };

    return mime.name().startsWith("audio/") || supportedExtensions.contains(extension);
} 