#include "window.hpp"

int main(int argc, char **argv) {
  try {
    abcg::Application app(argc, argv);

    Window window;
    window.setOpenGLSettings({.samples = 4, .doubleBuffering = false});
    window.setWindowSettings({.width = 800,
                              .height = 800,
                              .showFPS = false,
                              .showFullscreenButton = false,
                              .title = "Polygon Viewer 2.0"});

    app.run(window);
  } catch (std::exception const &exception) {
    fmt::print(stderr, "{}\n", exception.what());
    return -1;
  }
  return 0;
}