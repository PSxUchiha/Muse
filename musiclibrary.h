#ifndef MUSICLIBRARY_H
#define MUSICLIBRARY_H

#include <QObject>
#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QMimeDatabase>
#include <QMimeType>
#include <QDebug>
#include <QFileSystemWatcher>

class MusicLibrary : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList audioFiles READ audioFiles NOTIFY audioFilesChanged)
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY isLoadingChanged)

public:
    explicit MusicLibrary(QObject *parent = nullptr);
    ~MusicLibrary();

    QStringList audioFiles() const;
    bool isLoading() const;

    Q_INVOKABLE void scanMusicDirectory();
    Q_INVOKABLE QString getFileName(const QString &filePath) const;
    Q_INVOKABLE QString getFileExtension(const QString &filePath) const;

    void addDirectory(const QString& path);

    void setAudioFiles(const QStringList& files);

signals:
    void audioFilesChanged(const QStringList& files);
    void isLoadingChanged();

private:
    QStringList m_audioFiles;
    bool m_isLoading;
    QStringList m_supportedFormats;
    
    void setIsLoading(bool loading);
    void scanDirectory(const QString &path);
    bool isAudioFile(const QString &filePath) const;

    QFileSystemWatcher* m_watcher;
};

#endif // MUSICLIBRARY_H 