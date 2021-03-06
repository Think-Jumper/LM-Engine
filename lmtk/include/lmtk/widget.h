#pragma once

#include "lmgl/resource_sink.h"
#include <lmgl/lmgl.h>
#include <lmlib/geometry.h>

namespace lmtk
{
class widget_interface
{
  public:
    /// Return true if you want to render again immediately.
    virtual bool add_to_frame(lmgl::iframe *frame) = 0;
    virtual lm::size2i get_size() = 0;
    virtual lm::point2i get_position() = 0;

    virtual widget_interface &
      set_rect(lm::point2i position, lm::size2i size) = 0;

    virtual widget_interface &
      move_resources(lmgl::resource_sink &resource_sink) = 0;

    virtual ~widget_interface() = default;
};
} // namespace lmtk
