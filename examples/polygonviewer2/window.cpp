#include "window.hpp"

void Window::onEvent(SDL_Event const &event) {

  // Keyboard events
  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_LEFT) {
      m_simulationData.m_input.set(gsl::narrow<size_t>(Input::Left));
    }
    if (event.key.keysym.sym == SDLK_RIGHT) {
      m_simulationData.m_input.set(gsl::narrow<size_t>(Input::Right));
    }
    if (event.key.keysym.sym == SDLK_UP) {
      m_simulationData.m_input.set(gsl::narrow<size_t>(Input::ZoomIn));
    }
    if (event.key.keysym.sym == SDLK_DOWN) {
      m_simulationData.m_input.set(gsl::narrow<size_t>(Input::ZoomOut));
    }
    if (event.key.keysym.sym == SDLK_r) {
      m_simulationData.m_input.set(gsl::narrow<size_t>(Input::Restart));
      restart();
    }
  }
  if (event.type == SDL_KEYUP) {
    if (event.key.keysym.sym == SDLK_LEFT) {
      m_simulationData.m_input.reset(gsl::narrow<size_t>(Input::Left));
    }
    if (event.key.keysym.sym == SDLK_RIGHT) {
      m_simulationData.m_input.reset(gsl::narrow<size_t>(Input::Right));
    }
    if (event.key.keysym.sym == SDLK_UP) {
      m_simulationData.m_input.reset(gsl::narrow<size_t>(Input::ZoomIn));
    }
    if (event.key.keysym.sym == SDLK_DOWN) {
      m_simulationData.m_input.reset(gsl::narrow<size_t>(Input::ZoomOut));
    }
    if (event.key.keysym.sym == SDLK_r) {
      m_simulationData.m_input.reset(gsl::narrow<size_t>(Input::Restart));
    }
  }
}

void Window::onCreate() {

  // Create shader program
  auto const path{abcg::Application::getAssetsPath()};
  m_program =
      abcg::createOpenGLProgram({{.source = path + "UnlitVertexColor.vert",
                                  .stage = abcg::ShaderStage::Vertex},
                                 {.source = path + "UnlitVertexColor.frag",
                                  .stage = abcg::ShaderStage::Fragment}});

  // Load a new font
  auto const filename{path + "Inconsolata-Medium.ttf"};
  m_font = ImGui::GetIO().Fonts->AddFontFromFileTTF(filename.c_str(), 20.0f);
  if (m_font == nullptr) {
    throw abcg::RuntimeError("Cannot load font file");
  }

  abcg::glClearColor(0, 0, 0, 1);

  // Start pseudo-random number generator
  m_randomEngine.seed(
      std::chrono::steady_clock::now().time_since_epoch().count());

  restart();
}

void Window::restart() {
  m_simulationData.m_state = State::SimulationInProgress;

  auto const scaleLocation{abcg::glGetUniformLocation(m_program, "scale")};
  abcg::glUniform1f(scaleLocation, 0.25);

  // start rendering a triangle
  sides = 3;
  createRegularPolygon(sides);
}

void Window::onUpdate() {
  if (m_simulationData.m_state != State::SimulationInProgress) {
    restart();
    return;
  }
  checkEndOfTheSimulation();
}

void Window::onPaint() {

  // Set the clear color
  abcg::glClearColor(0, 0, 0, 1);
  // Clear the color buffer
  abcg::glClear(GL_COLOR_BUFFER_BIT);

  // Adjust viewport
  abcg::glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);

  // Start using the shader program
  abcg::glUseProgram(m_program);

  if (m_simulationData.m_input[static_cast<size_t>(Input::Left)]) {
    sides = sides - 1;
    createRegularPolygon(sides);
  }
  if (m_simulationData.m_input[static_cast<size_t>(Input::Right)]) {
    sides = sides + 1;
    createRegularPolygon(sides);
  }
  if (m_simulationData.m_input[static_cast<size_t>(Input::ZoomIn)]) {
    auto const scaleLocation{abcg::glGetUniformLocation(m_program, "scale")};
    abcg::glUniform1f(scaleLocation, 0.5);
  }
  if (m_simulationData.m_input[static_cast<size_t>(Input::ZoomIn)]) {
    auto const scaleLocation{abcg::glGetUniformLocation(m_program, "scale")};
    abcg::glUniform1f(scaleLocation, 0.25);
  }

  // Render
  abcg::glBindVertexArray(m_vao);
  abcg::glDrawArrays(GL_TRIANGLE_FAN, 0, sides + 2);
  abcg::glBindVertexArray(0);
  abcg::glUseProgram(0);
}

void Window::onPaintUI() {
  // Parent class will show fullscreen button and FPS meter
  abcg::OpenGLWindow::onPaintUI();

  // Our own ImGui widgets go below
  {
    // If this is the first frame, set initial position of our window
    static auto firstTime{true};
    if (firstTime) {
      ImGui::SetNextWindowPos(ImVec2(5, 75));
      firstTime = false;
    }

    ImGui::PushFont(m_font);

    // Window begin
    ImGui::Begin("Hello! This is a Polygon Viewer");

    // Static text
    ImGui::Text("Here you will see some examples of 2D polygons.");
    ImGui::Text("Regular polygons are convex, equilateral, and equiangular\n"
                "because their sides and angles are congruent. The three\n"
                "conditions must be satisfied.\n");
    ImGui::Text("Use the keyboard arrows to increment and decrement the number "
                "of vertices");

    if (m_simulationData.m_state == State::SimulationInProgress) {
      ImGui::Text("Simulation in Progress");
    } else if (m_simulationData.m_state == State::SimulationEnded) {
      ImGui::Text("End of Simulation");
      ImGui::Text("The Simulation will restart soon");
    }

    // TODO - Edit polygon color

    // End of window
    ImGui::PopFont();
    ImGui::End();
  }
}

void Window::onResize(glm::ivec2 const &size) {
  m_viewportSize = size;
  abcg::glClear(GL_COLOR_BUFFER_BIT);
}

void Window::onDestroy() {
  // Release OpenGL resources
  abcg::glDeleteProgram(m_program);
  abcg::glDeleteBuffers(1, &m_vboVertices);
  abcg::glDeleteBuffers(1, &m_vboColors);
  abcg::glDeleteVertexArrays(1, &m_vao);
}

/*
A polygon is regular when it is convex and has all sides and angles of the
same measure. Therefore, a regular polygon is equilateral, since all sides are
the same length, and equiangular, since all angles are the same measure.

The definition of a polygon is a closed,
flat figure formed by non-aligned and non-intersecting line segments.
These segments are the sides of the polygon that, when regular, are of the
same length.

The meeting of two sides is a vertex, and the area between the sides is called
an interior angle, measured in degrees. In regular polygons the angles are
congruent.

A polygon has the same number of sides, vertices, interior angles and exterior
angles. Regular polygons are convex, equilateral, and equiangular because
their sides and angles are congruent. The three conditions must be satisfied.

*/
void Window::createRegularPolygon(int sides) {
  // Release previous resources, if any
  abcg::glDeleteBuffers(1, &m_vboPositions);
  abcg::glDeleteBuffers(1, &m_vboColors);
  abcg::glDeleteVertexArrays(1, &m_vao);

  // Select random colors
  std::uniform_real_distribution<float> rd(0.0f, 1.0f);
  std::array const colors{glm::vec4(rd(m_randomEngine), rd(m_randomEngine),
                                    rd(m_randomEngine), 1.0f),
                          glm::vec4(rd(m_randomEngine), rd(m_randomEngine),
                                    rd(m_randomEngine), 1.0f),
                          glm::vec4(rd(m_randomEngine), rd(m_randomEngine),
                                    rd(m_randomEngine), 1.0f)};

  std::vector<glm::vec2> positions(0);

  // Polygon center
  positions.emplace_back(0, 0);

  // Border vertices
  const auto step{M_PI * 2 / sides};
  for (const auto angle : iter::range(0.0, M_PI * 2, step)) {
    positions.emplace_back(std::cos(angle), std::sin(angle));
  }
  // it is necessary to duplicate the last vertex so that the polygon image
  // becomes regular.
  positions.emplace_back(positions.at(1));

  // Generate VBO of positions
  abcg::glGenBuffers(1, &m_vboPositions);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_vboPositions);
  abcg::glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec2),
                     positions.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Generate VBO of colors
  abcg::glGenBuffers(1, &m_vboColors);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_vboColors);
  abcg::glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec4),
                     colors.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Get location of attributes in the program
  const auto positionAttribute{
      abcg::glGetAttribLocation(m_program, "inPosition")};
  const auto colorAttribute{abcg::glGetAttribLocation(m_program, "inColor")};

  // Create VAO
  abcg::glGenVertexArrays(1, &m_vao);

  // Bind vertex attributes to current VAO
  abcg::glBindVertexArray(m_vao);

  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_vboPositions);
  abcg::glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0,
                              nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  abcg::glEnableVertexAttribArray(colorAttribute);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_vboColors);
  abcg::glVertexAttribPointer(colorAttribute, 4, GL_FLOAT, GL_FALSE, 0,
                              nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // End of binding to current VAO
  abcg::glBindVertexArray(0);
}

// TODO - implement void createIrregularPolygon(int sides)

void Window::checkEndOfTheSimulation() {
  if (sides == 20) {
    m_simulationData.m_state = State::SimulationEnded;
  }
}