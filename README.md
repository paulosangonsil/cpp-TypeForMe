# Unicode Text Injector

A lightweight, zero-dependency Windows utility written in pure C++/Win32 that simulates human-like keyboard typing of arbitrary text into any focused application.

## ✨ Features

- **Full Unicode Support**: Types any character (e.g., `ç`, `ã`, `€`, `!`, emojis) correctly, regardless of your physical keyboard layout, by leveraging the native `KEYEVENTF_UNICODE` flag.
- **Multiline Preservation**: Accurately handles and injects newline characters (`\r\n`) as standard `Enter` key presses, ensuring compatibility with all target applications.
- **Human-like Timing**: Configurable inter-key delay (0–500ms) via a slider to prevent input dropping in sensitive applications.
- **Responsive UI**: Fully resizable Win32 graphical interface with a robust Rich Edit control for seamless copy-pasting of formatted text.
- **Zero Dependencies**: Built entirely with the native Windows API. No .NET, no Electron, no external runtimes required.

## 🚀 How to Use

1. Download or build the `TextInjector.exe` executable.
2. Run the application.
3. Type or paste the desired text into the main text box.
4. Adjust the delay slider if needed (default is 50ms/key, which works well for most apps).
5. Click the **Send** button.
6. **Immediately** click on the target window/application where you want the text to appear (you have a 3-second window to switch focus).
7. The utility will hide itself and begin typing the text into the focused window.

## 🛠️ Building from Source

### Prerequisites
- Windows 10 or later
- Visual Studio 2022/2026 (or Build Tools for Visual Studio) with the "Desktop development with C++" workload.
