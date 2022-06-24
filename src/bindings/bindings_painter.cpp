#include <memory>
#include <sstream>
#include <stdexcept>

#include <viren2d/drawing.h>

#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include <bindings/binding_helpers.h>
//TODO clean up doc painter;
// implement runtime tests examples/demo_runtime (10, 50, 100, ...) subsequent calls

namespace py = pybind11;

namespace viren2d {
namespace bindings {
TextAnchor TextAnchorFromPyObject(py::object &o) {
  if (py::isinstance<py::str>(o)) {
    return TextAnchorFromString(py::cast<std::string>(o));
  } else if (py::isinstance<TextAnchor>(o)) {
    return py::cast<TextAnchor>(o);
  } else {
    const std::string tp = py::cast<std::string>(
        o.attr("__class__").attr("__name__"));
    std::ostringstream str;
    str << "Cannot cast type `" << tp << "` to `"
        << FullyQualifiedType("TextAnchor") << "`!";
    throw std::invalid_argument(str.str());
  }
}


/**
 * @brief A wrapper for the abstract `Painter`
 *
 * This is necessary because I don't want to expose
 * the `ImagePainter` - I like the current factory method
 * layout because the public headers are quite clean.
 * Thus, I cannot (read "don't want to") use pybind11's
 * trampoline mechanism.
 */
class PainterWrapper {
public:
  PainterWrapper() : painter_(CreatePainter())
  {}


  PainterWrapper(const ImageBuffer &image)
    : painter_(CreatePainter()) {
    SetCanvasImage(image);
  }


  void SetCanvasColor(int width, int height, const Color &color) {
    painter_->SetCanvas(width, height, color);
  }


  void SetCanvasFilename(const std::string &image_filename) {
    painter_->SetCanvas(image_filename);
  }


  void SetCanvasImage(const ImageBuffer &image) {
    painter_->SetCanvas(image);
  }


  ImageBuffer GetCanvas(bool copy) {
    return painter_->GetCanvas(copy);
  }


  py::tuple GetCanvasSize() {
    auto sz = painter_->GetCanvasSize();
    return py::make_tuple(sz.x(), sz.y());
  }


  bool IsValid() {
    return (painter_ != nullptr) && (painter_->IsValid());
  }


  void DrawArc(
      const Vec2d &center, double radius, double angle1, double angle2,
      const LineStyle &line_style, bool include_center, const Color &fill_color) {
    painter_->DrawArc(
          center, radius, angle1, angle2, line_style,
          include_center, fill_color);
  }


  void DrawArrow(
      const Vec2d &from, const Vec2d &to, const ArrowStyle &arrow_style) {
    painter_->DrawArrow(from, to, arrow_style);
  }


  void DrawBoundingBox2D(
      const Rect &box, const std::vector<std::string> &label,
      const BoundingBox2DStyle &style) {
    painter_->DrawBoundingBox2D(box, label, style);
  }


  void DrawCircle(
      const Vec2d &center, double radius,
      const LineStyle &line_style, const Color &fill_color) {
    painter_->DrawCircle(center, radius, line_style, fill_color);
  }


  void DrawEllipse(
      const Ellipse &ellipse, const LineStyle &line_style,
      const Color &fill_color) {
    painter_->DrawEllipse(ellipse, line_style, fill_color);
  }


  void DrawGrid(
      double spacing_x, double spacing_y,
      const LineStyle &line_style,
      const Vec2d &top_left,
      const Vec2d &bottom_right) {
    painter_->DrawGrid(
          top_left, bottom_right,
          spacing_x, spacing_y, line_style);
  }


  void DrawLine(
      const Vec2d &from, const Vec2d &to,
      const LineStyle &line_style) {
    painter_->DrawLine(from, to, line_style);
  }


  void DrawMarker(const Vec2d &pos, const MarkerStyle &style) {
    painter_->DrawMarker(pos, style);
  }


  void DrawMarkers(
      const std::vector<std::pair<Vec2d, Color>> &markers,
      const MarkerStyle &style) {
    painter_->DrawMarkers(markers, style);
  }


  void DrawPolygon(
      const std::vector<Vec2d> &polygon, const LineStyle &line_style,
      const Color &fill_color) {
    painter_->DrawPolygon(polygon, line_style, fill_color);
  }


  void DrawRect(
      const Rect &rect, const LineStyle &line_style,
      const Color &fill_color) {
    painter_->DrawRect(rect, line_style, fill_color);
  }


  void DrawText(
      const std::vector<std::string> &text,
      const Vec2d &anchor_position, py::object &pyanchor,
      const TextStyle &text_style, const Vec2d &padding,
      double rotation) {
    TextAnchor anchor = TextAnchorFromPyObject(pyanchor);
    painter_->DrawText(
          text, anchor_position, anchor,
          text_style, padding, rotation);
  }


  void DrawTextBox(
      const std::vector<std::string> &text,
      const Vec2d &anchor_position, py::object &pyanchor,
      const TextStyle &text_style, const Vec2d &padding,
      double rotation, const LineStyle &box_line_style,
      const Color &box_fill_color, double box_corner_radius,
      const Vec2d &fixed_box_size) {
    TextAnchor anchor = TextAnchorFromPyObject(pyanchor);
    painter_->DrawTextBox(
          text, anchor_position, anchor, text_style,
          padding, rotation, box_line_style, box_fill_color,
          box_corner_radius, fixed_box_size);
  }


  void DrawTrajectory(
      const std::vector<Vec2d> &points, const LineStyle &style,
      const Color &color_fade_out, bool oldest_position_first,
      int smoothing_window,
      const std::function<double(double)> &fading_factor) {
    painter_->DrawTrajectory(
          points, style, color_fade_out, oldest_position_first,
          smoothing_window, fading_factor);
  }


  /** String representation used to bind __str__ and __repr__. */
  std::string StringRepresentation(bool tag) const {
    std::ostringstream s;
    if (tag) {
      s << '<';
    }

    s << "viren2d.Painter(";

    if (painter_->IsValid()) {
      auto size = painter_->GetCanvasSize();
      s << size.width() << 'x' << size.height();
    } else {
      s << "canvas not initialized";
    }

    s << ')';

    if (tag) {
      s << '>';
    }
    return s.str();
  }


private:
  std::unique_ptr<Painter> painter_;
};


void RegisterPainter(py::module &m) {
  py::class_<PainterWrapper> painter(m, "Painter", R"docstr(
      A *Painter* lets you draw on its canvas.

      Typical workflow:

      1. Create a painter with an empty canvas:

         >>> import viren2d
         >>> painter = viren2d.Painter()

      2. Initialize its canvas:

         * Paint an empty canvas with a given color
           via :meth:`set_canvas_rgb`
         * Set up the canvas from image data
           via :meth:`set_canvas_image`
         * Set up the canvas by loading an image from disk
           via :meth:`set_canvas_filename`

         Note that the overloaded *Painter* constructor allows
         combining these two steps if you intend on setting up the
         canvas from image data, *e.g.* a :class:`numpy.ndarray`:

         >>> image = np.zeros((600, 800, 3), dtype=np.uint8)
         >>> painter = viren2d.Painter(image)

      3. Draw onto the canvas via the painter's ``draw_xxx(...)``
         methods, for example:

         >>> painter.draw_arrow(...)
         >>> painter.draw_bounding_box(...)

      4. After all objects have been drawn, retrieve the
         visualization via :meth:`get_canvas`. For example,
         to get a deeply copied image as :class:`numpy.ndarray`:

         >>> import numpy as np
         >>> canvas = painter.get_canvas(copy=True)
         >>> img_np = np.array(canvas, copy=False)

      5. Either continue drawing (step 3) or set up
         a new canvas (step 2), *i.e.* it is safe to reuse the
         same painter instance.
      )docstr");

  painter.def(
        py::init<>(), R"docstr(
        Default constructor.

        Initializes an empty canvas, *i.e.* :meth:`~viren2d.Painter.is_valid`
        will return ``False`` until the canvas has been properly set up.
        )docstr")
      .def(
        py::init<const ImageBuffer&>(), R"docstr(
        Creates a painter and initializes its canvas.

        Initializes the painter's canvas with the given image.
        See :meth:`~viren2d.Painter.set_canvas_image` for notes
        on supported image formats.
        )docstr", py::arg("image"))
      .def(
        "__repr__",
        [](const PainterWrapper &p) { return p.StringRepresentation(true); })
      .def(
        "__str__",
        [](const PainterWrapper &p) { return p.StringRepresentation(false); })
      .def(
        "is_valid",
        &PainterWrapper::IsValid,
        "Checks if the canvas has been set up correctly.");

  std::string doc = R"docstr(
      Initializes the canvas with the given color.

      Args:
        width: Canvas width in pixels.
        height: Canvas height in pixels.
        color: Background :class:`~viren2d.Color`.

      Examples:
        >>> painter = viren2d.Painter()
        >>> painter.set_canvas_rgb(800, 600)
        >>> painter.set_canvas_rgb(800, 600, 'crimson')
        >>> painter.set_canvas_rgb(800, 600, (0.5, 0.3, 0.9))
      )docstr";
  painter.def(
        "set_canvas_rgb",
        &PainterWrapper::SetCanvasColor, doc.c_str(),
        py::arg("width"), py::arg("height"),
        py::arg("color") = Color::White);

  painter.def(
        "set_canvas_filename",
        &PainterWrapper::SetCanvasFilename, R"docstr(
      Initializes the canvas from the given image file.

      This functionality uses the
      `stb library <https://github.com/nothings/stb/blob/master/stb_image.h>`__
      to load the image file. Supported formats are:

         JPEG, PNG, TGA, BMP, PSD, GIF, HDR, PIC, PNM

      )docstr", py::arg("image_filename"));


  doc = R"docstr(
      Initializes the canvas from the given image.

      Args:
        img_np: Image as either a :class:`numpy.ndarray` (currently,
          only :class:`numpy.uint8` is supported) or an :class:`~viren2d.ImageBuffer`.
          The image can either be grayscale, RGB or RGBA.

      Example:
        >>> img_np = np.zeros((480, 640, 3), dtype=np.uint8)
        >>> painter.set_canvas_image(img_np)
      )docstr";
  painter.def(
        "set_canvas_image",
        &PainterWrapper::SetCanvasImage, doc.c_str(),
        py::arg("image"));


  //----------------------------------------------------------------------
  painter.def(
        "get_canvas_size",
        &PainterWrapper::GetCanvasSize, R"docstr(
      Returns the size of the canvas in pixels.

      Returns:
        The canvas width & height as the :class:`tuple` ``(W, H)``,
        where ``W`` and ``H`` denote pixels (type :class:`int`).
      )docstr");

  painter.def_property_readonly(
        "width",
        [](PainterWrapper &pw) -> py::int_ { return pw.GetCanvasSize()[0]; },
        "int: Width in pixels of the painter's canvas (read-only).");

  painter.def_property_readonly(
        "height",
        [](PainterWrapper &pw) -> py::int_ { return pw.GetCanvasSize()[1]; },
        "int: Height in pixels of the painter's canvas (read-only).");

  painter.def_property_readonly(
        "canvas_data",
        [](PainterWrapper &pw) -> ImageBuffer { return pw.GetCanvas(false); }, R"docstr(
        :class:`~viren2d.ImageBuffer`: Provides a **shared memory** view
          on the painter's canvas.

          >>> #TODO
          >>> img_np = np.array(painter.canvas)

          TODO update workflow docstring
        )docstr");


  //----------------------------------------------------------------------
  painter.def(
        "get_canvas",
        &PainterWrapper::GetCanvas, R"docstr(
      Returns the current visualization in RGBA format.

      Returns an :class:`~viren2d.ImageBuffer`, which implements
      the Python buffer protocol. This means, it can be easily
      converted to other buffer types, such as :class:`numpy.ndarray`,
      see the examples below.

      Args:
        copy: If you want a **deep copy**, set ``copy = True``.
          Otherwise, the buffer will just provide a **shared
          view** on the painter's canvas.

          Be aware that if you keep on drawing after obtaining
          a shared view, this view will also change. You could
          even externally modify the canvas pixels.

      Returns:
        The current visualization as a 4-channel, ``uint8``
        :class:`~viren2d.ImageBuffer` with pixel format **RGBA**.

      Examples:
        Get canvas as :class:`numpy.ndarray`, where the **memory is
        shared** with the painter:

        >>> img_np = np.array(p.get_canvas(copy=False), copy=False)

        Retrieve a **deep copy** of the canvas as :class:`numpy.ndarray`,
        *i.e.* future ``painter.draw_...`` calls will not affect this
        retrieved copy:

        >>> img_np = np.array(p.get_canvas(copy=True), copy=False)

        If we need a 3-channel image, we can leverage the
        :meth:`~viren2d.ImageBuffer.to_rgb` method of the buffer:

        >>> # We only need a shared view on the canvas...
        >>> img_buf = p.get_canvas(copy=False)
        >>> # ... because the following call copies memory anyways:
        >>> img_np = img_buf.to_rgb()

      .. tip::
          If you can ensure that the painter is not destroyed while
          you display/process the visualization, use the shared view
          (*i.e.* ``copy = False``) on its canvas to avoid unnecessary
          memory allocation.
      )docstr", py::arg("copy") = true);


  //----------------------------------------------------------------------
  doc = R"docstr(
      Draws a circular arc.

      Args:
        center: Center position as :class:`~viren2d.Vec2d`.
        radius: Radius of the arc in pixels as :class:`float`
        angle_from: The arc will be drawn from ``angle_from``
          to ``angle_to`` in clockwise direction. Both angles
          are specified in degrees as :class:`float`, where 0
          degrees points in the direction of increasing *x* coordinates.
        angle_to: See ``angle_from``
        line_style: A :class:`~viren2d.LineStyle` specifying how
          to draw the arc's outline.
          If you pass :attr:`viren2d.LineStyle.Invalid`, the contour
          will not be drawn (then, you must provide a valid
          ``fill_color``).
        include_center:  If ``True`` (default), the center point
          will be included when drawing the outline and filling.
        fill_color: If you provide a valid :class:`~viren2d.Color`,
          the arc will be filled.

      Example:
        >>> #TODO
      )docstr";
  painter.def(
        "draw_arc",
        &PainterWrapper::DrawArc, doc.c_str(),
        py::arg("center"), py::arg("radius"),
        py::arg("angle_from"), py::arg("angle_to"),
        py::arg("line_style") = LineStyle(),
        py::arg("include_center") = true,
        py::arg("fill_color") = Color::Invalid);


  //----------------------------------------------------------------------
  doc = R"docstr(
      Draws an arrow.

      Args:
        pt1: Start of the arrow shaft as :class:`~viren2d.Vec2d`.
        pt2: End of the arrow shaft (*i.e.* the *pointy end*) as
          :class:`~viren2d.Vec2d`.
        arrow_style: An :class:`~viren2d.ArrowStyle` specifying
          how to draw the arrow.

      Example:
        >>> #TODO

      Note:
        Arrows should always be drawn **fully opaque**. Otherwise,
        you'll experience visible blending in the crossing path
        segments (*i.e.* at the tip).
      )docstr";
  painter.def(
        "draw_arrow",
        &PainterWrapper::DrawArrow, doc.c_str(),
        py::arg("pt1"), py::arg("pt2"),
        py::arg("arrow_style") = ArrowStyle());


  //----------------------------------------------------------------------
  doc = R"docstr(
      Draws a single 2D bounding box.

      Args:
        rect: The box geometry as :class:`~viren2d.Rect`.
        label: The label as :class:`list` of :class:`str`, since multi-line
          labels are supported.
        box_style: A :class:`~viren2d.BoundingBox2DStyle` specifying how
          to draw this bounding box.

      Example:
        >>> #TODO
      )docstr";
  painter.def(
        "draw_bounding_box_2d",
        &PainterWrapper::DrawBoundingBox2D, doc.c_str(),
        py::arg("rect"), py::arg("label"),
        py::arg("box_style") = BoundingBox2DStyle());

  //TODO multiple 2d bounding boxes


  //----------------------------------------------------------------------
  doc = R"docstr(
      Draws a circle.

      Args:
        center: Center position as :class:`~viren2d.Vec2d`
        radius: Radius of the circle in pixels as :class:`float`.
        line_style: A :class:`~viren2d.LineStyle` specifying how
          to draw the circle's outline.

          If you pass :attr:`~viren2d.LineStyle.Invalid`, the
          contour will not be drawn - then, you must provide a
          valid ``fill_color``.
        fill_color: If you provide a valid :class:`~viren2d.Color`,
          the circle will be filled.

      Example:
        >>> #TODO
      )docstr";
  painter.def(
        "draw_circle",
        &PainterWrapper::DrawCircle, doc.c_str(),
        py::arg("center"), py::arg("radius"),
        py::arg("line_style") = LineStyle(),
        py::arg("fill_color") = Color::Invalid);


  //----------------------------------------------------------------------
  doc = R"docstr(
      Draws an ellipse.

      Args:
        ellipse: The :class:`~viren2d.Ellipse`, which should
          be drawn.
        line_style: A :class:`~viren2d.LineStyle` specifying
          how to draw the ellipse's outline.

          If you pass :attr:`~viren2d.LineStyle.Invalid`, the
          contour will not be drawn - then, you must provide a
          valid ``fill_color``.
        fill_color: If you provide a valid :class:`~viren2d.Color`,
          the ellipse will be filled."

      Example:
        >>> #TODO
      )docstr";
  painter.def(
        "draw_ellipse",
        &PainterWrapper::DrawEllipse, doc.c_str(),
        py::arg("ellipse"),
        py::arg("line_style") = LineStyle(),
        py::arg("fill_color") = Color::Invalid);


        //TODO coding style//----------------------------------------------------------------------
  doc = "Draws a grid.\n\n:spacing_x:  (float)\n:spacing_y:  (float)\n"
        "    Width & height of each grid cell.\n\n"
        "    The grid will only be drawn within the defined region.\n"
        "    If not provided, the grid will span the whole canvas.\n\n"
        ":line_style:  (" + FullyQualifiedType("LineStyle") + ")\n"
        "    How to draw the grid lines.\n"
        "    If you pass " + FullyQualifiedType("LineStyle.Default") + ", the\n"
        "    painter's default line style will be used.\n\n"
        ":top_left:  (" + FullyQualifiedType(Vec2d::TypeName()) + ")\n"
        ":bottom_right:  (" + FullyQualifiedType(Vec2d::TypeName()) + ")\n";
  painter.def("draw_grid", &PainterWrapper::DrawGrid, doc.c_str(),
        py::arg("spacing_x"), py::arg("spacing_y"),
        py::arg("line_style") = LineStyle(),
        py::arg("top_left") = Vec2d(),
        py::arg("bottom_right") = Vec2d());


  //----------------------------------------------------------------------
  doc = R"docstr(
      Draws a line.

      Args:
        pt1: Start position as :class:`~viren2d.Vec2d`.
        pt2: End position as :class:`~viren2d.Vec2d`.
        line_style: A :class:`~viren2d.LineStyle` specifying
          how to draw the line.

      Example:
        >>> line_style = viren2d.LineStyle(width=3, color='azure')
        >>> painter.draw_line((42, 42), (86, 86), line_style)
        >>> # Same call via named arguments:
        >>> painter.draw_line(
        >>>     pt1=(42, 42), pt2=(86, 86), line_style=line_style)
      )docstr";
  painter.def(
        "draw_line",
        &PainterWrapper::DrawLine, doc.c_str(),
        py::arg("pt1"), py::arg("pt2"),
        py::arg("line_style") = LineStyle());


  //----------------------------------------------------------------------
  doc = R"docstr(
      Draws a single marker/keypoint.

      Args:
        pos: Position as :class:`~viren2d.Vec2d`.
        marker_style: A :class:`~viren2d.MarkerStyle` specifying
          how to draw the marker.

      Example:
        >>> marker_style = viren2d.MarkerStyle()
        >>> marker_style.marker = '7'
        >>> marker_style.size = 30
        >>> painter.draw_marker((42, 70), marker_style)
      )docstr";
  painter.def(
        "draw_marker",
        &PainterWrapper::DrawMarker, doc.c_str(),
        py::arg("pos"), py::arg("marker_style") = MarkerStyle());


  doc = R"docstr(
      Draws multiple (similar) markers/keypoints.

      Args:
        markers: Holds the position and color of each marker.
          Should be provided as a :class:`list` of :class:`tuple`\ s, where
          each :class:`tuple` holds the position and color of a marker
          as (:class:`~viren2d.Vec2d`, :class:`~viren2d.Color`).
          If a marker's color is invalid, it will be drawn using
          ``marker_style``'s color specification instead.
        marker_style: A :class:`~viren2d.MarkerStyle` specifying
          how to draw the markers (except for the color).

      Example:
        >>> marker_style = viren2d.MarkerStyle(color='crimson')
        >>> markers = [
        >>>     ((10, 10), 'blue'),
        >>>     ((20, 10), (0.5, 0, 0.5)),
        >>>     ((30, 10), viren2d.RGBa(200, 0, 180)),
        >>>     ((40, 10), 'invalid'),   # Will use marker_style.color
        >>>     ((50, 34), (-1, -1, -1)) # This one too
        >>> ]
        >>> painter.draw_markers(markers, marker_style)
      )docstr";
  painter.def(
        "draw_markers",
        &PainterWrapper::DrawMarkers, doc.c_str(),
        py::arg("markers"),
        py::arg("marker_style") = MarkerStyle());


  //TODO raw string doc + example //----------------------------------------------------------------------
  doc = "Draws a polygon.\n\n"
        "Args:\n"
        "  polygon: Points of the polygon as :class:`list` of :class:`~"
        + FullyQualifiedType(Vec2d::TypeName()) + "`.\n"
        "  line_style: A :class:`~" + FullyQualifiedType("LineStyle") + "` specifying how\n"
        "    to draw the circle's outline.\n"
        "    If you pass " + FullyQualifiedType("LineStyle.Invalid") + ", the\n"
        "    contour will not be drawn (then you must provide a valid ``fill_color``).\n"
        "  fill_color: If you provide a valid :class:`~" + FullyQualifiedType("Color") + "`,\n"
        "    the circle will be filled.";
  painter.def("draw_polygon", &PainterWrapper::DrawPolygon, doc.c_str(),
              py::arg("polygon"), py::arg("line_style") = LineStyle(),
              py::arg("fill_color") = Color::Invalid);

  //TODO raw string doc + example //----------------------------------------------------------------------
  doc = "Draws a rectangle (axis-aligned/rotated, solid/dashed, etc.)\n\n"
        "Args:\n  rect: The :class:`~" + FullyQualifiedType("Rect") + "`\n"
        "    object which should be drawn.\n"
        "  line_style: A :class:`~" + FullyQualifiedType("LineStyle") + "` specifying how\n"
        "    to draw the rectangle's outline.\n"
        "    If you pass :attr:`" + FullyQualifiedType("LineStyle.Invalid") + "`, the\n"
        "    contour will not be drawn (then you must provide a valid ``fill_color``).\n"
        "  fill_color: If you provide a valid :class:`~" + FullyQualifiedType("Color") + "`,\n"
        "    the rectangle will be filled.";
  painter.def("draw_rect", &PainterWrapper::DrawRect, doc.c_str(),
              py::arg("rect"),
              py::arg("line_style") = LineStyle(),
              py::arg("fill_color") = Color::Invalid);

  //TODO raw string doc + example //----------------------------------------------------------------------
  doc = "Renders text onto the canvas.\n\n"
      "Args:\n"
      "  text: A list of strings to be drawn.\n"
      "  position: Position of the reference point as :class:`~"
      + FullyQualifiedType(Vec2d::TypeName()) + "`.\n"
      "  anchor: How to orient the text w.r.t. the reference point.\n"
      "    Valid inputs are :class:`~" + FullyQualifiedType("TextAnchor")
      + "` enum values and\n"
      "    string representations. A string must correspond to either a\n"
      "    *position specification* (*i.e.* ``center``, ``top``, ``top-right``,\n"
      "    ``right``, ``bottom-right``, ``bottom``, ``bottom-left``,\n"
      "    ``left``, ``top-left``) or one of the 8 *compass directions* (*i.e.*\n"
      "    ``north``, ``north-east``, ``east``, ``south-east``, ``south``,\n"
      "    ``south-west``, ``west``, ``north-west``).\n\n"
      "    Before parsing, the input will be converted to lowercase and any\n"
      "    whitespaces, dashes & underscores will be removed.\n"
      "  text_style: A :class:`~" + FullyQualifiedType("TextStyle") +"`,\n"
      "    specifying how to render the text.\n"
      "  padding: Optional distance between the closest glyph and the\n"
      "    reference point. Specified in pixels as :class:`~"
      + FullyQualifiedType("Vec2d") + "`.\n"
      "  rotation: Rotation angle in degrees as ``float``.";
  painter.def("draw_text", &PainterWrapper::DrawText, doc.c_str(),
      py::arg("text"), py::arg("position"),
      py::arg("anchor") = TextAnchor::BottomLeft,
      py::arg("text_style") = TextStyle(),
      py::arg("padding") = Vec2d(0.0, 0.0),
      py::arg("rotation") = 0.0);



  //TODO raw string doc + example //----------------------------------------------------------------------
  doc = "Draws a text box.\n\n"
      "  anchor: Either enum or string representation";
  painter.def(
        "draw_textbox",
        &PainterWrapper::DrawTextBox, doc.c_str(),
        py::arg("text"),
        py::arg("position"),
        py::arg("anchor") = TextAnchor::BottomLeft,
        py::arg("text_style") = TextStyle(),
        py::arg("padding") = Vec2d::All(6.0),
        py::arg("rotation") = 0.0,
        py::arg("line_style") = LineStyle::Invalid,
        py::arg("fill_color") = Color::White.WithAlpha(0.6),
        py::arg("radius") = 0.2,
        py::arg("fixed_size") = Vec2d::All(-1.0));


  //----------------------------------------------------------------------
  doc = R"docstr(
      Draws a trajectory.

      Can be used to either draw **a single-color path** (if
      ``fade_out_color`` is invalid), or **a path which gradually
      changes its color** from ``line_style.color`` to ``fade_out_color``.
      In the latter case, the color transition can be controlled
      by ``fading_factor``.

      Args:
        points: A :class:`list` of the trajectory's coordinates
          as :class:`~viren2d.Vec2d`.
        line_style: A :class:`~viren2d.LineStyle` specifying how
          to draw the trajectory (except for the color gradient).
        fade_out_color: If this is a valid :class:`~viren2d.Color`,
          the trajectory's tail will be drawn with this color.
        tail_first: Set to ``True`` if the first point, *i.e.*
          ``points[0]``, is the *oldest* point, *i.e.* the trajectory's
          tail. Otherwise, it is assumed to be the *most recent* point.
        smoothing_window: Specifies the window size to optionally
          smooth the trajectory via moving average. Disable smoothing
          by passing a value ``<= 0``.
        fading_factor: A function handle which will be invoked for
          each segment of the trajectory to compute the mixing ratios
          for the color gradient.

          Its **single input** is a :class:`float` :math:`\in [0,1]`, which
          denotes the *drawing progress* along the trajectory, from head
          (*i.e.* :math:`0`) to tail.
          Its **return value** must also be a :class:`float` :math:`\in [0,1]`,
          which specifies the amount of the ``fade_out_color`` to be
          applied for this *drawing progress*.

          For example, to get a *linear* color transition between head
          and tail, we simply use the identity function.
          For convenience, ``viren2d`` already provides :func:`~viren2d.fade_out_linear`,
          :func:`~viren2d.fade_out_quadratic`, and :func:`~viren2d.fade_out_logarithmic`.
          Default is :func:`~viren2d.fade_out_quadratic`.

      Example:
        >>> points = [(0, 0), (10, 20), (42, 30), ...]
        >>> line_style = viren2d.LineStyle(
        >>>     width=5, color='navy-blue',
        >>>     cap=viren2d.LineCap.Round,
        >>>     join=viren2d.LineJoin.Round)
        >>> painter.draw_trajectory(
        >>>     points, line_style,
        >>>     smoothing_window=5,
        >>>     fading_factor=viren2d.fade_out_linear)

      Note:
        For fading colors, the trajectory has to be drawn via
        separate line segments. This means that the
        :attr:`~viren2d.LineStyle.join` setting of ``line_style``
        parameter will be ignored. Additionally, if transparent
        colors are used, the segment endpoints will be visible.

        To avoid this behavior, the trajectory needs to be drawn with
        a single color, *i.e.* pass :attr:`viren2d.Color.Invalid` as
        ``fade_out_color``.
      )docstr";
  painter.def(
      "draw_trajectory", &PainterWrapper::DrawTrajectory, doc.c_str(),
      py::arg("points"),
      py::arg("line_style") = LineStyle(),
      py::arg("fade_out_color") = Color(NamedColor::LightGray, 0.6),
      py::arg("tail_first") = true,
      py::arg("smoothing_window") = 0,
      py::arg("fading_factor") = std::function<double(double)>(ColorFadeOutQuadratic));
  // TODO Adjust the docstring if we change anything. It includes
  // the current default color mix/fading factor function(!)



  //TODO(snototter) add draw_xxx methods

  //TODO(snototter) add convenience functions handling multiple
  //    inputs (plural draw_xxxS), e.g. via the PainterWrapper:
  //    "for (element in list) : painter_->DrawElement();"
}

} // namespace bindings
} // namespace viren2d
