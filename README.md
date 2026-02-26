# MyFileManager

MyFileManager is a lightweight Qt6-based file manager built with Qt Widgets.

It provides a clean two-pane interface with directory navigation, file operations,
and native desktop integration — without unnecessary overhead.

No Electron. No tracking. Just Qt.

---

## ✨ Features

- Two-pane layout (Tree view + Icon view)
- Directory navigation (Home / Up / Back / Forward*)
- Address/location bar
- Double-click to open files with default system application
- Create folders
- Rename files/folders
- Delete files/folders (recursive delete supported)
- Copy / Cut / Paste (basic implementation)
- Status bar with current path and item count
- Themed toolbar icons
- Native desktop integration via `QDesktopServices`

\* Back/Forward simplified in current version.

---

## 🖥️ Built With

- C++17
- Qt6 (Widgets module)
- QFileSystemModel
- QTreeView + QListView
- QSplitter
- QDesktopServices
- CMake

---

## 📦 Requirements

- Qt6 (Widgets module)
- CMake 3.16+
- GCC / Clang with C++17 support
- Linux (tested on Linux)

---

## 🔧 Build Instructions (CMake Only)

```bash
git clone https://www.github.com/swastikchatt-gh/enapp
cd MyFileManager

mkdir build
cd build

cmake ..
make
./MyFileManager
```

---

## 📁 Project Structure

```
MyFileManager/
 ├── main.cpp
 ├── CMakeLists.txt
 └── resources.qrc   (if using embedded icon)
```

---

## ⌨️ Keyboard Shortcuts

| Action        | Shortcut            |
|--------------|--------------------|
| Home         | Ctrl + H           |
| Up           | Alt + ↑            |
| Back         | Backspace          |
| Forward      | Forward key        |
| Refresh      | F5                 |
| New Folder   | Ctrl + Shift + N   |
| Rename       | F2                 |
| Delete       | Delete             |
| Copy         | Ctrl + C           |
| Cut          | Ctrl + X           |
| Paste        | Ctrl + V           |
| Quit         | Ctrl + Q           |

---

## ⚠️ Limitations

- Recursive folder copy not implemented
- Cross-device move (cut+paste) may fail for directories
- No full navigation history stack
- No drag & drop yet

---

## 🚀 Planned Improvements

- Proper navigation history
- Recursive folder copy
- Drag & Drop support
- Context menu (right-click)
- File properties dialog
- Thumbnail previews
- Search functionality
- Multi-tab support

---

## 📜 License

This project is licensed under **SLIC**.

See the LICENSE file for details.

---

## 👨‍💻 Author

Swastik Chatterjee

---

A simple file manager.
Built because we can.
