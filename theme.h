#ifndef THEME_H
#define THEME_H

#include <QString>

namespace Theme {
    // Color palette
    namespace Colors {
        const QString Primary = "palette(highlight)";
        const QString Background = "palette(window)";
        const QString Text = "palette(text)";
        const QString Mid = "palette(mid)";
        const QString Button = "palette(button)";
        const QString HighlightedText = "palette(highlighted-text)";
    }

    // Common styles
    const QString MAIN_WINDOW_STYLE = QString("QMainWindow { background-color: %1; }").arg(Colors::Background);
    
    const QString BUTTON_STYLE = QString(
        "QPushButton {"
        "    border: none;"
        "    background-color: transparent;"
        "    color: %1;"
        "    padding: 8px;"
        "    border-radius: 4px;"
        "}"
        "QPushButton:hover {"
        "    background-color: %2;"
        "}"
        "QPushButton:pressed {"
        "    background-color: %2;"
        "    color: %3;"
        "}"
    ).arg(Colors::Text, Colors::Primary, Colors::HighlightedText);

    const QString TOOL_BUTTON_STYLE = QString(
        "QToolButton {"
        "    border: none;"
        "    background-color: transparent;"
        "    color: %1;"
        "    padding: 4px;"
        "    border-radius: 4px;"
        "}"
        "QToolButton:hover {"
        "    background-color: %2;"
        "}"
        "QToolButton:pressed {"
        "    background-color: %2;"
        "    color: %3;"
        "}"
    ).arg(Colors::Text, Colors::Primary, Colors::HighlightedText);

    const QString SLIDER_STYLE = QString(
        "QSlider::groove:horizontal {"
        "    border: 1px solid %1;"
        "    height: 4px;"
        "    background: %1;"
        "    margin: 2px 0;"
        "    border-radius: 2px;"
        "}"
        "QSlider::handle:horizontal {"
        "    background: %2;"
        "    border: none;"
        "    width: 12px;"
        "    margin: -4px 0;"
        "    border-radius: 6px;"
        "}"
        "QSlider::sub-page:horizontal {"
        "    background: %3;"
        "    border-radius: 2px;"
        "}"
    ).arg(Colors::Mid, Colors::Button, Colors::Primary);

    const QString LIST_WIDGET_STYLE = QString(
        "QListWidget {"
        "    background-color: %1;"
        "    border: none;"
        "    color: %2;"
        "    font-size: 14px;"
        "}"
        "QListWidget::item {"
        "    padding: 10px;"
        "    border-bottom: 1px solid %3;"
        "}"
        "QListWidget::item:selected {"
        "    background-color: %4;"
        "    color: %5;"
        "}"
        "QListWidget::item:hover {"
        "    background-color: %4;"
        "}"
    ).arg(Colors::Background, Colors::Text, Colors::Mid, Colors::Primary, Colors::HighlightedText);

    const QString LABEL_STYLE = QString(
        "QLabel {"
        "    color: %1;"
        "}"
    ).arg(Colors::Text);

    const QString MINI_PLAYER_STYLE = QString(
        "QWidget {"
        "    background-color: %1;"
        "    border-top: 1px solid %2;"
        "}"
    ).arg(Colors::Background, Colors::Mid);

    const QString SIDEBAR_STYLE = QString(
        "QWidget {"
        "    background-color: %1;"
        "}"
        "QPushButton {"
        "    text-align: left;"
        "    padding: 12px;"
        "    border: none;"
        "    border-radius: 0;"
        "    color: %2;"
        "    background-color: transparent;"
        "    font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "    background-color: %3;"
        "}"
        "QPushButton:checked {"
        "    background-color: %3;"
        "    font-weight: bold;"
        "}"
    ).arg(Colors::Background, Colors::Text, Colors::Primary);

    const QString FULLSCREEN_PLAYER_STYLE = QString(
        "QWidget {"
        "    background-color: %1;"
        "    border-radius: 10px;"
        "}"
    ).arg(Colors::Background);
}

#endif // THEME_H 