#include "window.hpp"

void Window::onCreate() {

  // Create shader program
  auto const path{abcg::Application::getAssetsPath()};
  m_program =
      abcg::createOpenGLProgram({{.source = path + "UnlitVertexColor.vert",
                                  .stage = abcg::ShaderStage::Vertex},
                                 {.source = path + "UnlitVertexColor.frag",
                                  .stage = abcg::ShaderStage::Fragment}});

  // Start pseudo-random number generator
  m_randomEngine.seed(
      std::chrono::steady_clock::now().time_since_epoch().count());

  // start rendering a triangle
  sides = 3;
  createRegularPolygon(sides);
}

void Window::onPaint() {
  // Set the clear color
  abcg::glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b,
                     m_clearColor.a);
  // Clear the color buffer
  abcg::glClear(GL_COLOR_BUFFER_BIT);

  // Adjust viewport
  abcg::glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);

  // Start using the shader program
  abcg::glUseProgram(m_program);

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

    // Window begin
    ImGui::Begin("Hello! This is a Polygon Viewer");

    // Static text
    ImGui::Text("Here you will see some examples of 2D polygons.");

    // Combo box - at the moment it is only possible to select some regular
    // polygons
    // TODO - increase the list of regular polygons
    // TODO - investigate the possibility of creating an algorithm for creating
    // irregular polygons
    {
      static auto currentIndex{0U};
      std::vector<std::string> const comboItems{
          "Regular Triangle", "Regular Quadrilateral", "Regular Pentagon",
          "Regular Hexagon"};

      if (ImGui::BeginCombo("Select Polygon",
                            comboItems.at(currentIndex).c_str())) {
        for (auto const index : iter::range(comboItems.size())) {
          bool const isSelected{currentIndex == index};
          if (ImGui::Selectable(comboItems.at(index).c_str(), isSelected)) {
            currentIndex = index;

            if (comboItems.at(currentIndex) == "Regular Triangle")
              sides = 3;
            if (comboItems.at(currentIndex) == "Regular Quadrilateral")
              sides = 4;
            if (comboItems.at(currentIndex) == "Regular Pentagon")
              sides = 5;
            if (comboItems.at(currentIndex) == "Regular Hexagon")
              sides = 6;
            createRegularPolygon(sides);
          }

          // Set the initial focus when opening the combo (scrolling + keyboard
          // navigation focus)
          if (isSelected)
            ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
    }
  }

  // Static text
  ImGui::Text("Regular polygons are convex, equilateral, and equiangular\n"
              "because their sides and angles are congruent. The three\n"
              "conditions must be satisfied.\n");

  // Edit background color
  ImGui::ColorEdit3("Background", &m_clearColor.r);

  // TODO - Edit polygon color

  // End of window
  ImGui::End();
}

void Window::onResize(glm::ivec2 const &size) { m_viewportSize = size; }

void Window::onDestroy() {
  // Release OpenGL resources
  abcg::glDeleteProgram(m_program);
  abcg::glDeleteBuffers(1, &m_vboVertices);
  abcg::glDeleteBuffers(1, &m_vboColors);
  abcg::glDeleteVertexArrays(1, &m_vao);
}

/*
A polygon is regular when it is convex and has all sides and angles of the same
measure. Therefore, a regular polygon is equilateral, since all sides are the
same length, and equiangular, since all angles are the same measure.

The definition of a polygon is a closed,
flat figure formed by non-aligned and non-intersecting line segments.
These segments are the sides of the polygon that, when regular, are of the same
length.

The meeting of two sides is a vertex, and the area between the sides is called
an interior angle, measured in degrees. In regular polygons the angles are
congruent.

A polygon has the same number of sides, vertices, interior angles and exterior
angles. Regular polygons are convex, equilateral, and equiangular because their
sides and angles are congruent. The three conditions must be satisfied.

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