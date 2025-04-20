#ifndef COMMON_H
#define COMMON_H

// Forward declarations
class QMediaPlayer;
class QAudioOutput;
class QLabel;
class QPushButton;
class QSlider;
class QListWidget;
class QWidget;
class QPropertyAnimation;
class QGraphicsOpacityEffect;

// Common constants
namespace Constants {
    constexpr int MINI_PLAYER_HEIGHT = 60;
    constexpr int SIDEBAR_WIDTH = 200;
    constexpr int ANIMATION_DURATION = 300;
    constexpr int DEFAULT_VOLUME = 50;
}

// Common enums
enum class PlaybackState {
    Stopped,
    Playing,
    Paused
};

#endif // COMMON_H 