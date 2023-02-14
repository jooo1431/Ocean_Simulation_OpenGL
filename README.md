# OGL4Core2

OGL4Core2 is a framework to ease the development of prototypical or education-oriented graphical algorithms.
Each algorithm is implemented as its own plugin, which are managed by the framework and can be arbitrarily switched at runtime.
The framework provides functionality on different levels of abstraction that can be chosen according to the education goal.
The functionality includes:
- Plugin instantiation by subclassing and implementing a thin interface
- Window and OpenGL Context creation/handling is done by the framework
- Convenience functions for shader management, texture/data loading, OpenGL objects, mouse interaction
- No overhead for window handling etc.

OGL4Core2 is developed at the [Visualization Research Center of the University of Stuttgart (VISUS)](https://www.visus.uni-stuttgart.de/).
The idea is based on the previous framework OGL4Core [1].
OGL4Core2 happily relies on the functionality of
[GLFW](https://www.glfw.org/),
[glad](https://github.com/Dav1dde/glad),
[GLM](https://glm.g-truc.net/0.9.9/index.html),
[Dear ImGui](https://github.com/ocornut/imgui),
[glowl](https://github.com/invor/glowl),
[LodePNG](https://lodev.org/lodepng/).

## Build

OGL4Core2 runs on Linux and Windows and relies on [CMake](https://cmake.org/) to be build.

### Windows

On Windows use the CMake GUI to configure the project.

### Linux

Some dependencies are required depending on the used display server.

- Debian / Ubuntu
  - X11: `sudo apt install xorg-dev`
  - Wayland: `sudo apt install libwayland-dev libxkbcommon-dev wayland-protocols extra-cmake-modules`
- Fedore / Red Hat
  - X11: `sudo dnf install libXcursor-devel libXi-devel libXinerama-devel libXrandr-devel`
  - Wayland: `sudo dnf install wayland-devel libxkbcommon-devel wayland-protocols-devel extra-cmake-modules`

Source: [GLFW Documentation](https://www.glfw.org/docs/3.3/compile.html#compile_deps).

If all dependencies are installed, you can just run:

```
mkdir build && cd build
cmake ..
make
```

## Documentation

### Concept

Different algorithms/visualizations/prototypes are implement as plugins. Each plugin will be a class with is derived
from the RenderPlugin class. Next to the plugins the most important part of OGL4Core2 is the Core class. It is the
main part of this framework and all plugins as well as the window, OpenGL context and all other setup is done here.
Next to this are different helper functionalities available.

### Plugin creation

To implement a new algorithm a new plugin class is needed. The source file for this class should be crated in the
directory `src/plugins/PluginName` or alternatively with arbitrary grouping `src/plugins/A/B/C/PluginName`. If the
plugin has additional resources such as shaders, textures and other data, they must reside in directory called
`resources` (case-insensitive) within the plugin directory.

A minimal plugin will look like the following example:
```
namespace OGL4Core2::Plugins::PluginName {
    class PluginName : public Core::RenderPlugin {
        REGISTERPLUGIN(PluginName, 500)
    public:
        explicit PluginName(const Core::Core &c) : RenderPlugin(c) {}
        static std::string name() { return "PluginName"; }
        void render() override {};
    };
}
```

The constructor is required to have exactly one parameter which must be a const reference to Core class. This reference
must be passed the base class constructor. The setup of the plugin should be done in the constructor and be cleaned in
the destructor of the plugin.
Further there must be a static `name()` function without any parameter returning a std::string with the name of the
plugin. This name will be later presented in the GUI of the plugin selection.
And finally the abstract `render()` method of the base class must be overwritten and implemented. This method will be
called by the Core in a loop on drawing every frame.

Additionally, the new plugin needs to be registered, that the Core will be able to find it. Therefore, it is required
to call the `REGISTERPLUGIN()` macro within the plugin class. The macro has 2 parameters. The first is the class name
of the plugin, the second parameter is the sort index. The plugins will later be shown in the plugin selection UI
sorted according to this index.

### Mouse/Keyboard/Window input

The RenderPlugin base class has several event callback functions which can be reimplemented in the plugin class to
receive the events. Here is a list of the available methods:

- `void resize(int width, int height) override`
  Window resize event. Is guaranteed to be called after construction and before first call of `render()`.
  Parameters:
  - width: new width of the window
  - height: new height of the window
- `void keyboard(Key key, KeyAction action, Mods mods) override`
  Key event. Parameters:
  - `key`: The key that was pressed or released. Key is an enum class of all keys.
  - `action`: Release, Press or Repeat. KeyAction is an enum class containing this three actions.
  - `mods`: Provides flags, which modifier keys (shift, control, alt, super) are pressed.
- `void charInput(unsigned int codepoint) override`
  Character input event. Parameters:
  - `codepoint`: The Unicode code point of the character.
- `void mouseButton(MouseButton button, MouseButtonAction action, Mods mods) override`
  Mouse button event. Paramters:
  - `button`: The mouse button that was pressed or released. MouseButton is an enum class of all buttons.
  - `action`: One of Release or Press. MouseButtonAction is an enum class containing this two actions.
  - `mods`: Provides flags, which modifier keys (shift, control, alt, super) are pressed.
- `void mouseMove(double xpos, double ypos) override`
  Mouse move event. Paramters:
  - `xpos`: The new cursor x-coordinate, relative to the left edge of the content area.
  - `ypos`: The new cursor y-coordinate, relative to the top edge of the content area.
- `void mouseScroll(double xoffset, double yoffset) override`
  Mouse scroll event. Paramters:
  - `xoffset`: The scroll offset along the x-axis.
  - `yoffset`: The scroll offset along the y-axis.

In addition to the event callbacks, it is possible to check the current state of a keyboard or mouse button. The state
is available from the Core. The following methods can be called on the reference to the Core instance stored in the
`core_` variable of RenderPlugin:
- `bool isKeyPressed(Key key)`
  - `key`: The key to check.
  - Returns true if the key is pressed, false otherwise.
- `bool isMouseButtonPressed(MouseButton button)`
  - `button`: The mouse button to check.
  - Returns true if the button is pressed, false otherwise.
- `void getMousePos(double &xpos, double &ypos)`
  - Current mouse position is set to `xpos` and `ypos`.

### Resource loading

OGL4Core2 offers some convenience functions to locate and load resources from disk. Remember from plugin creation, all
resources must be located within a `resources` directory within the plugin directory. OGL4Core2 is able to automatically
locate this directory during runtime. When running OGL4Core2 from the build directory, the resources from the source
repository will be used. When using CMake to install OGL4Core2, the `resources` directory will be copied to the
installation directory. When running an installed version of OGL4Core2 the installed resources will be used.

To receive the full absolute path of the current resources directory, you can ask the Core instance for it with the
following method:
- `std::filesystem::path getPluginResourcesPath()`
  - Returns the full absolute path to the resources directory of the currently loaded plugin.

In addition, the RenderPlugin base class has implemented a few more helpers to load specific resource types within a
plugin:
- `std::filesystem::path getResourcePath(const std::string& name)`
  Generic resource path generation. Use the name parameter to specify the resource pathname relative to the resources
  directory. Always use Linux style path separator `/`. The full absolute path of the resource will be returned.
  Example: To get the path of a resource located in `src/plugins/A/B/C/PluginName/resources/shaders/vertex.glsl` call
  `getResourceFilePath("shaders/vertex.glsl")`.
- `std::filesystem::path getResourceFilePath(const std::string& name)`
  Same as `getResourcePath()`, but with additional check if resource exists and is regular file.
- `std::filesystem::path getResourceDirPath(const std::string& name)`
  Same as `getResourcePath()`, but with additional check if resource exists and is directory.
- `std::string getStringResource(const std::string& name)`
  Uses `getResourceFilePath()` to locate the file, reads the file and returns the content as string.
- `std::vector<unsigned char> getPngResource(const std::string& name, int& width, int& height)`
  Name parameter as in `getResourcesFilePath()`. The resource must be a valid PNG file. Image will be read and returned
  as an unsigned char buffer in RGBA format. Size will be returned in the width and height parameters.
- `std::shared_ptr<glowl::Texture2D> getTextureResource(const std::string& name)`
  Name parameter as in `getResourcesFilePath()`. The resource must be a valid PNG file. File will be read and a glowl
  texture object will be created form it.
- `std::vector<std::filesystem::path> getResourceDirFilePaths(const std::string& name, const std::string& filter)`
  Get list of files in directory. Name parameter as in `getResourceDirPath()`. Filter param is an optional regex
  pattern to filter the file list.

### Plugin GUI

- To add GUI parameters for the plugin the `Dear ImGui` library can be used within the `render()` method. Direct use of
  any ImGui GUI element is possible. Initialization, per frame handling and final rendering is already done by the Core
  instance. The core will also draw a collapsing header element around all elements created from the plugin.
  For usage of the single GUI elements please refer to the [Dar ImGui documentation](https://github.com/ocornut/imgui).

### Other Helpers

- `glowl`
  The glowl library is included within OGL4Core2 and can be used as modern wrapper around plain OpenGL objects.
- `OrbitCamera`
  is a convenience class to manage a quaternion based trackball rotation of the scene. To use it instance it within the
  plugin. The OrbitCamera has the following interface methods:
  - `drawGUI()`: will draw an GUI element to represent the camera. Should be called in the plugins `render()` method.
  - `rotate()`: if mouse/keyboard interaction is wanted, the following method must be called within the input event callbacks with an appropriate mapping.
  - `moveDolly()`: if mouse/keyboard interaction is wanted, the following method must be called within the input event callbacks with an appropriate mapping.
  - `viewMx()`: will return a 4x4 view matrix for usage during rendering.
- Core Camera handler:
  A camera can be registered within the core instance using the `registerCamera()` method. The core will then
  automatically map all inputs from mouse and keyboard to the camera instance. Within the plugin no additional camera
  handling is needed, except drawing the camera GUI if wanted.

## References

[1]: G. Reina, T. Müller, T. Ertl: Incorporating Modern OpenGL into Computer Graphics Education. IEEE Computer Graphics & Applications 34(4), pp. 16-21, 2014.
