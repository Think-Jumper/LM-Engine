#pragma once
#include "input_event.h"
#include "lmgl/resource_sink.h"
#include <lmgl/lmgl.h>

#include "resource_cache.h"
#include "widget.h"

namespace lmtk
{
class rect_border : public widget_interface
{
  public:
    lm::point2i get_position() override;
    static lmgl::material create_material(lmgl::irenderer *renderer);
    lm::size2i get_size() override;
    rect_border(
      lmgl::irenderer *renderer,
      resource_cache const &resource_cache,
      lm::point2i position,
      lm::size2i size,
      std::array<float, 4> colour,
      float border_thickness);

    rect_border(rect_border const &) = delete;
    rect_border(rect_border &&) = default;

    bool add_to_frame(lmgl::iframe *frame) override;

    void set_thickness(float thickness);

    rect_border &set_rect(lm::point2i position, lm::size2i size) override
    {
        this->position = position;
        this->size = size;
        return *this;
    }

    rect_border &move_resources(lmgl::resource_sink &sink) override
    {
        sink.add(ubuffer);
        return *this;
    }

  private:
    lm::point2i position;
    lm::size2i size;
    float thickness;
    std::array<float, 4> colour;
    lmgl::buffer ubuffer;
    lmgl::geometry geometry;
};
} // namespace lmtk
