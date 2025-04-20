# Muse - Retro Music Player

Muse is a nostalgic music player that brings back the charm of physical records and vinyl playback. It provides a unique, vintage-inspired interface while maintaining modern functionality for your digital music collection.

## Features

- 🎵 Retro-inspired user interface reminiscent of classic record players
- 📀 Album-centric playback experience
- 🎚️ Classic playback controls (play, pause, skip, volume)
- 📚 Music library management
- 🎨 Customizable themes
- 🎧 High-quality audio playback

## Requirements

- Qt 6.x
- C++17 or later
- CMake 3.16 or later
- A C++ compiler with C++17 support

## Building from Source

1. Clone the repository:
```bash
git clone https://github.com/PSxUchiha/Muse.git
cd Muse
```

2. Create a build directory and navigate to it:
```bash
mkdir build
cd build
```

3. Configure the project with CMake:
```bash
cmake ..
```

4. Build the project:
```bash
cmake --build .
```

## Usage

1. Launch the application
2. Add your music library in ~/Music folder
3. Select an album to play

## Project Structure

- `mainwindow.cpp/h` - Main application window and UI components
- `musicplayer.cpp/h` - Core music playback functionality
- `musiclibrary.cpp/h` - Music library management
- `theme.h` - Theme customization
- `main.qml` - Qt Quick interface definitions

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the GNU GPL License - see the LICENSE file for details.

## Acknowledgments

- Inspired by classic record players and vintage audio equipment
- Built with Qt framework
- Thanks to all contributors who help make this project better 