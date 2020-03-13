# Chassim

## Run Requirements
- Java 12+ (don't forget to add to PATH!)
- Microsoft Visual C++ Redistributable ([x86](https://aka.ms/vs/16/release/vc_redist.x86.exe), [x64](https://aka.ms/vs/16/release/vc_redist.x64.exe))

## Build Requirements
- Java (Tested with 12)
- Visual Studio (Preferably 2019)
- [SFML 2.5.1](https://www.sfml-dev.org/download.php)
- [TGUI 0.8.6](https://tgui.eu/download)
- [Box2D 5ae818e](https://github.com/erincatto/box2d/tree/5ae818e95ddd09622bad4fd295311ca4706ad2b2)
- [tiny-process-library 2.0.2](https://gitlab.com/eidheim/tiny-process-library)
- [json 3.7.3](https://github.com/nlohmann/json/releases)

## Building
- Execute .\gradlew installDist to build & install the Java components
- Copy the 'include' & 'lib' directories of each dependency (create a include folder for json.hpp) to the respective folders in 'lib'
- Open the solution file in Visual Studio to build the main program

## Usage
- Most things should be self-explanatory
- Normal cartesian coordinate system (+x to the right, -y downwards)
- +angle is counter-clockwise
- Keyboard Shortcuts
	- Ctrl-Shift-C to copy points into clipboard
	- Ctrl-G to regenerate the profile