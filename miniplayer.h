#ifndef MINIPLAYER_H
#define MINIPLAYER_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include "theme.h"

class MiniPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit MiniPlayer(QWidget *parent = nullptr);
    ~MiniPlayer();

    void updateMetadata(const QString &title, const QString &artist, const QImage &albumArt);
    void updatePlayPauseButton(bool isPlaying);
    void updatePosition(qint64 position);
    void updateDuration(qint64 duration);
    void setPlaybackState(bool isPlaying);

    QPushButton* getPlayPauseButton() const { return playPauseButton; }
    QPushButton* getNextButton() const { return nextButton; }
    QPushButton* getPreviousButton() const { return previousButton; }
    QSlider* getPositionSlider() const { return positionSlider; }
    QLabel* getTimeLabel() const { return timeLabel; }

signals:
    void playPauseClicked();
    void nextClicked();
    void previousClicked();
    void positionSliderPressed();
    void positionSliderReleased();
    void positionSliderMoved(int value);
    void miniPlayerClicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    QLabel *albumArt;
    QLabel *titleLabel;
    QLabel *artistLabel;
    QPushButton *playPauseButton;
    QPushButton *nextButton;
    QPushButton *previousButton;
    QSlider *positionSlider;
    QLabel *timeLabel;
};

#endif // MINIPLAYER_H 