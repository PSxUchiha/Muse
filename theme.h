#ifndef THEME_H
#define THEME_H

#include <QString>

namespace Theme {
    // Common styles
    const QString MAIN_WINDOW_STYLE = R"(
        QMainWindow {
            background-color: palette(window);
        }
    )";

    const QString BUTTON_STYLE = R"(
        QPushButton {
            border: none;
            background-color: transparent;
            color: palette(text);
            padding: 8px;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: palette(highlight);
        }
        QPushButton:pressed {
            background-color: palette(highlight);
            color: palette(highlighted-text);
        }
    )";

    const QString TOOL_BUTTON_STYLE = R"(
        QToolButton {
            border: none;
            background-color: transparent;
            color: palette(text);
            padding: 4px;
            border-radius: 4px;
        }
        QToolButton:hover {
            background-color: palette(highlight);
        }
        QToolButton:pressed {
            background-color: palette(highlight);
            color: palette(highlighted-text);
        }
    )";

    const QString SLIDER_STYLE = R"(
        QSlider::groove:horizontal {
            border: 1px solid palette(mid);
            height: 4px;
            background: palette(mid);
            margin: 2px 0;
            border-radius: 2px;
        }
        QSlider::handle:horizontal {
            background: palette(button);
            border: none;
            width: 12px;
            margin: -4px 0;
            border-radius: 6px;
        }
        QSlider::sub-page:horizontal {
            background: palette(highlight);
            border-radius: 2px;
        }
    )";

    const QString LIST_WIDGET_STYLE = R"(
        QListWidget {
            background-color: palette(window);
            border: none;
            color: palette(text);
            font-size: 14px;
        }
        QListWidget::item {
            padding: 10px;
            border-bottom: 1px solid palette(mid);
        }
        QListWidget::item:selected {
            background-color: palette(highlight);
            color: palette(highlighted-text);
        }
        QListWidget::item:hover {
            background-color: palette(highlight);
        }
    )";

    const QString LABEL_STYLE = R"(
        QLabel {
            color: palette(text);
        }
    )";

    const QString MINI_PLAYER_STYLE = R"(
        QWidget {
            background-color: palette(window);
            border-top: 1px solid palette(mid);
        }
    )";

    const QString SIDEBAR_STYLE = R"(
        QWidget {
            background-color: palette(window);
        }
        QPushButton {
            text-align: left;
            padding: 12px;
            border: none;
            border-radius: 0;
            color: palette(text);
            background-color: transparent;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: palette(highlight);
        }
        QPushButton:checked {
            background-color: palette(highlight);
            font-weight: bold;
        }
    )";

    const QString FULLSCREEN_PLAYER_STYLE = R"(
        QWidget {
            background-color: palette(window);
            border-radius: 10px;
        }
    )";
}

#endif // THEME_H 