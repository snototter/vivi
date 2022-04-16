#include <sstream>
#include <memory>
#include <stdexcept>

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>

#include <vivi/vivi.hpp>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;


/** @brief Utilities to support pickling.
 *
 *  Naming convention:
 *  * Serialize<X>   --> X.__getstate__
 *  * Deserialize<X> --> X.__setstate__
 *
 *  See corresponding pybind11 doc:
 *  https://pybind11.readthedocs.io/en/stable/advanced/classes.html#pickling-support
 */
namespace pickling
{
//------------------------------------------------- Color
py::tuple SerializeColor(const vivi::Color &c)
{
  return py::make_tuple(c.red, c.green, c.blue, c.alpha);
}


vivi::Color DeserializeColor(py::tuple tpl)
{
  if (tpl.size() != 4)
  {
    std::stringstream s;
    s << "Invalid vivi.Color state - expected 4 values (rgba), got " << tpl.size() << "!";
    throw std::invalid_argument(s.str());
  }
  return vivi::Color(tpl[0].cast<double>(), tpl[1].cast<double>(),
                     tpl[2].cast<double>(), tpl[3].cast<double>());
}

//------------------------------------------------- Templated Vec(tor) class
template<typename _Tp, int dim>
py::list SerializeVec(const vivi::Vec<_Tp, dim> &vec)
{
  py::list lst;
  for (int i = 0; i < dim; ++i)
    lst.append(vec.val[i]);
  return lst;
}

template<typename _Tp, int dim>
vivi::Vec<_Tp, dim> DeserializeVec(py::list lst)
{
  using VC = vivi::Vec<_Tp, dim>;
  if (lst.size() != dim)
  {
    std::stringstream s;
    s << "Invalid vivi." << VC::TypeName()
      << " state - expected " << dim << " values, found "
      << lst.size() << "!";
    throw std::invalid_argument(s.str());
  }

  VC vec;
  for (int i = 0; i < dim; ++i)
    vec.val[i] = lst[i].cast<_Tp>();
  return vec;
}

//------------------------------------------------- Rectangle
py::tuple SerializeRect(const vivi::Rect &r)
{
  return py::make_tuple(r.cx, r.cy, r.width, r.height, r.angle, r.radius);
}


vivi::Rect DeserializeRect(py::tuple tpl)
{
  if (tpl.size() != 6)
  {
    std::stringstream s;
    s << "Invalid vivi.Rect state - expected 6 entries, got " << tpl.size() << "!";
    throw std::invalid_argument(s.str());
  }
  return vivi::Rect(tpl[0].cast<double>(), tpl[1].cast<double>(),
                    tpl[2].cast<double>(), tpl[3].cast<double>(),
                    tpl[4].cast<double>(), tpl[5].cast<double>());
}


//------------------------------------------------- TODO Pickle others if needed




//------------------------------------------------- LineStyle
py::tuple SerializeLineStyle(const vivi::LineStyle &c)
{
  return py::make_tuple(c.line_width, c.color, c.dash_pattern, c.line_cap, c.line_join);
}


vivi::LineStyle DeserializeLineStyle(py::tuple tpl)
{
  if (tpl.size() != 5)
  {
    std::stringstream s;
    s << "Invalid vivi.LineStyle state - expected 5 entries, got " << tpl.size() << "!";
    throw std::invalid_argument(s.str());
  }
  vivi::LineStyle ls(tpl[0].cast<double>(),
                     tpl[1].cast<vivi::Color>(),
                     tpl[2].cast<std::vector<double>>(),
                     tpl[3].cast<vivi::LineStyle::Cap>(),
                     tpl[4].cast<vivi::LineStyle::Join>());
  return ls;
}
} // namespace pickling




/** @brief Utilities to simplify module definition. */
namespace moddef
{
//------------------------------------------------- Wrapper for the Painter
// Necessary, because we're not exposing the ImagePainter, so
// we can't use the pybind11 trampoline mechanism.
class Painter
{
public:
  Painter() : painter_(vivi::CreateImagePainter())
  {}

  void SetCanvasColor(int width, int height, const vivi::Color &color)
  {
    painter_->SetCanvas(width, height, color);
  }

  void SetCanvasFilename(const std::string &image_filename)
  {
    painter_->SetCanvas(image_filename);
  }

//  void SetCanvasImage(TODO)

  vivi::ImageBuffer GetCanvas(bool copy)
  {
    return painter_->GetCanvas(copy);
  }

  void DummyShow() { painter_->DummyShow(); } //TODO remove
  //TODO setcanvas image
  //TODO getcanvas

  void DrawLine(const vivi::Vec2d &from, const vivi::Vec2d &to,
                const vivi::LineStyle &line_style)
  {
    painter_->DrawLine(from, to, line_style);
  }


  void DrawCircle(const vivi::Vec2d &center, double radius,
                  const vivi::LineStyle &line_style,
                  const vivi::Color &fill)
  {
    painter_->DrawCircle(center, radius, line_style, fill);
  }


  void DrawRect(const vivi::Rect &rect, const vivi::LineStyle &line_style,
                const vivi::Color &fill)
  {
    painter_->DrawRect(rect, line_style, fill);
  }

private:
  std::unique_ptr<vivi::Painter> painter_;
};



//-------------------------------------------------  Color from tuple
vivi::Color CreateColor(py::tuple tpl)
{
  if (tpl.size() < 3 || tpl.size() > 4)
  {
    std::stringstream s;
    s << "Cannot create vivi.Color: expected 3 or 4 values, found tuple with"
      << tpl.size() << "!";
    throw std::invalid_argument(s.str());
  }
  vivi::Color col(tpl[0].cast<double>(),
                  tpl[1].cast<double>(),
                  tpl[2].cast<double>(), 1.0);
  if (tpl.size() == 4)
    col.alpha = tpl[3].cast<double>();
  return col;
}


//-------------------------------------------------  LineStyle from tuple
vivi::LineStyle CreateLineStyle(py::tuple tpl)
{
  if (tpl.size() < 2 || tpl.size() > 5)
  {
    std::stringstream s;
    s << "Cannot create vivi.LineStyle from tuple with"
      << tpl.size() << " entries!";
    throw std::invalid_argument(s.str());
  }

  vivi::LineStyle ls(tpl[0].cast<double>(),
                     tpl[1].cast<vivi::Color>());
  if (tpl.size() > 2)
    ls.dash_pattern = tpl[2].cast<std::vector<double>>();

  if (tpl.size() > 3)
    ls.line_cap = tpl[3].cast<vivi::LineStyle::Cap>();

  if (tpl.size() > 4)
    ls.line_join = tpl[4].cast<vivi::LineStyle::Join>();

  return ls;
}


//-------------------------------------------------  Rectangle from tuple
vivi::Rect CreateRect(py::tuple tpl)
{
  if (tpl.size() < 4 || tpl.size() > 6)
  {
    std::stringstream s;
    s << "Cannot create vivi.Rect from tuple with"
      << tpl.size() << " entries!";
    throw std::invalid_argument(s.str());
  }

  vivi::Rect rect(tpl[0].cast<double>(),
                  tpl[1].cast<double>(),
                  tpl[2].cast<double>(),
                  tpl[3].cast<double>());
  if (tpl.size() > 4)
    rect.angle = tpl[4].cast<double>();
  if (tpl.size() > 5)
    rect.radius = tpl[5].cast<double>();
  return rect;
}


//------------------------------------------------- Vec(tor) from tuple
template<typename _Tp, int dim>
vivi::Vec<_Tp, dim> CreateVec(py::tuple tpl)
{
  using VC = vivi::Vec<_Tp, dim>;
  if (tpl.size() != dim)
  {
    std::stringstream s;
    s << "Cannot create vivi." << VC::TypeName()
      << ": expected " << dim << " values, found tuple with"
      << tpl.size() << "!";
    throw std::invalid_argument(s.str());
  }

  VC vec;
  for (int i = 0; i < dim; ++i)
    vec.val[i] = tpl[i].cast<_Tp>();
  return vec;
}


template<typename _Tp, int dim>
void RegisterVec(py::module &m)
{
  using VC = vivi::Vec<_Tp, dim>;
  py::class_<VC> vec_cls(m, VC::TypeName().c_str()); //TODO optional (could be useful if vecs are to be used in python more extensively):, py::dynamic_attr())

  vec_cls.def(py::init<>())
         .def(py::init<>(&CreateVec<_Tp, dim>))
         .def(py::init<_Tp, _Tp>(), py::arg("x"), py::arg("y"))
         .def(py::init<_Tp, _Tp, _Tp>(), py::arg("x"), py::arg("y"), py::arg("z"))
         .def(py::init<_Tp, _Tp, _Tp, _Tp>(), py::arg("x"), py::arg("y"), py::arg("z"), py::arg("w"))
         .def("__repr__",
              [](const VC& v)
              { return "<vivi." + v.ToString() + ">"; })
         .def("__str__", &VC::ToString)
         .def_property("x", static_cast<const _Tp& (VC::*)() const>(&VC::x), &VC::SetX,
                       "Accesses the first dimension.")
         .def_property("y", static_cast<const _Tp& (VC::*)() const>(&VC::y), &VC::SetY,
                       "Accesses the second dimension.")
         .def("__setitem__", [](VC &self, unsigned index, _Tp v)
                             { self[index] = v; })
         .def("__getitem__", [](const VC &self, unsigned index)
                             { return self[index]; })
         .def("dot", &VC::Dot, py::arg("other"))
         .def("length", &VC::Length)
         .def("distance", &VC::Distance, py::arg("other"))
         .def(py::pickle(&pickling::SerializeVec<_Tp, dim>,
                         &pickling::DeserializeVec<_Tp, dim>))
         .def(py::self == py::self)
         .def(py::self != py::self);

  //TODO bind other operators:
  // +
  // +=
  // -
  // -=
  // *
  // *=
  // x / 3; /=

  if (dim == 3)
    vec_cls.def("cross", &VC::Cross, "Cross product", py::arg("other"));

  if (dim > 2)
    vec_cls.def_property("z", static_cast<const _Tp& (VC::*)() const>(&VC::z), &VC::SetZ,
                         "Accesses the third dimension.");
  if (dim > 3)
    vec_cls.def_property("w", static_cast<const _Tp& (VC::*)() const>(&VC::w), &VC::SetW,
                         "Accesses the fourth dimension.");

  py::implicitly_convertible<py::tuple, VC>();
}

} // namespace moddef


//------------------------------------------------- Module definition
// TODO How to bind a new class X:
// * Implement moddef::CreateX (init from py::tuple/list/whatever)
// * Implement pickling::SerializeX
// * Implement pickling::DeserializeX
// * Implement __str__ & __repr__
// * nice-to-have: operator == and !=
// * Declare it py::implicitly_convertible
// * Check (in python) initialization, pickling, comparison, etc.
// * All this info does not hold for ImageBuffer - which exposes a
//   buffer view (and we need to be able to convert to/from numpy
//   arrays)
//
// TODO low priority: DeserializeX could reuse CreateX (not sure if
//      both are using the same inputs, tuples vs list vs ...)

PYBIND11_MODULE(vivi, m)
{ 
  m.doc() = R"pbdoc(
    vivi - A VIsualization tool for computer VIsion

    Python bindings for the C++ vivi toolbox which
    uses the 2D Cairo graphics library.
  )pbdoc";

  //------------------------------------------------- Color
  py::class_<vivi::Color>(m, "Color")
      .def(py::init<>())
      .def(py::init<>(&moddef::CreateColor))// init from tuple
      .def(py::init<double, double, double, double>(),
           py::arg("red"), py::arg("green"), py::arg("blue"),
           py::arg("alpha")=1.0)
      .def("__repr__",
           [](const vivi::Color& c)
           { return "<vivi.Color " + c.ToString() + ">"; })
      .def("__str__", &vivi::Color::ToString)
      .def(py::pickle(&pickling::SerializeColor,
                      &pickling::DeserializeColor))
      .def(py::self == py::self)
      .def(py::self != py::self)
      .def("as_RGBA", &vivi::Color::ToRGBA,
           "Returns the corresponding (R, G, B, a) tuple, where R, G, B in [0, 255] and alpha in [0, 1].")
      .def("as_hex", &vivi::Color::ToHexString,
           "Returns the hex web color code representation, e.g. '#0011FF' (alpha is ignored).")
      .def_readwrite("red", &vivi::Color::red, "Red component within [0, 1].")
      .def_readwrite("green", &vivi::Color::green, "Green component within [0, 1].")
      .def_readwrite("blue", &vivi::Color::blue, "Blue component within [0, 1].")
      .def_readwrite("alpha", &vivi::Color::alpha, "Opacity within [0, 1].")
      .def("inverse", &vivi::Color::Inverse,
           "Returns the inverse color, i.e. (1.0-r, 1.0-g, 1.0-b).\n"
           "Alpha value stays the same.");

  // A Color can be initialized from a given tuple.
  py::implicitly_convertible<py::tuple, vivi::Color>();


  m.def("rgba", vivi::rgba, "Returns a vivi.Color for the given rgba values.\n"
                            "r, g, b and alpha must be within [0, 1]",
        py::arg("red"), py::arg("green"), py::arg("blue"),
        py::arg("alpha")=1.0);


  m.def("RGBA", vivi::RGBA, "Returns a vivi.Color for the given RGBA values.\n"
                            "* R,G,B must be within [0, 255]\n"
                            "* alpha must be within [0, 1].",
        py::arg("red"), py::arg("green"), py::arg("blue"),
        py::arg("alpha")=1.0);


  //------------------------------------------------- Primitives - Vectors
  moddef::RegisterVec<double, 2>(m);
  moddef::RegisterVec<double, 3>(m);
  moddef::RegisterVec<double, 4>(m);
  moddef::RegisterVec<int, 2>(m);
  moddef::RegisterVec<int, 3>(m);

  //------------------------------------------------- Primitives - Rectangle
  py::class_<vivi::Rect>(m, "Rect")
      .def(py::init<>(&moddef::CreateRect))// init from tuple
      .def(py::init<double, double, double, double>(),
           py::arg("cx"), py::arg("cx"), py::arg("w"), py::arg("h"))
      .def(py::init<double, double, double, double, double>(),
           py::arg("cx"), py::arg("cx"), py::arg("w"), py::arg("h"),
           py::arg("angle"))
      .def(py::init<double, double, double, double, double, double>(),
           py::arg("cx"), py::arg("cx"), py::arg("w"), py::arg("h"),
           py::arg("angle"), py::arg("radius"))
      .def("__repr__",
           [](const vivi::Rect& r)
           { return "<vivi." + r.ToString() + ">"; })
      .def("__str__", &vivi::Rect::ToString)
      .def(py::pickle(&pickling::SerializeRect,
                      &pickling::DeserializeRect))
      .def(py::self == py::self)
      .def(py::self != py::self)
      .def_readwrite("cx", &vivi::Rect::cx, "Horizontal center.")
      .def_readwrite("cy", &vivi::Rect::cy, "Vertical center.")
      .def_readwrite("width", &vivi::Rect::width, "Rectangle width.")
      .def_readwrite("height", &vivi::Rect::height, "Rectangle height.")
      .def_readwrite("angle", &vivi::Rect::angle, "Angle in degrees (clockwise rotation).")
      .def_readwrite("radius", &vivi::Rect::radius, "Corner radius (for rounded rectangles).");

  // A Rect can be initialized from a given tuple.
  py::implicitly_convertible<py::tuple, vivi::Rect>();



  //TODO line? (overkill, not needed now)
  //TODO plane? (maybe, when we implement 3d)
  //TODO image? (prefer to use a simple buffer; maybe in combination with stb image)
  //------------------------------------------------- Primitives - ImageBuffer
  // TODO numpy memory layout: https://stackoverflow.com/questions/53097952/how-to-understand-numpy-strides-for-layman
  // pybind11 numpy: https://pybind11.readthedocs.io/en/stable/advanced/pycpp/numpy.html
  py::class_<vivi::ImageBuffer>(m, "ImageBuffer", py::buffer_protocol()) //TODO doc
     .def_buffer([](vivi::ImageBuffer &img) -> py::buffer_info {
          return py::buffer_info(
              img.data, sizeof(unsigned char), // Pointer to data & size of each element
              py::format_descriptor<unsigned char>::format(), // Python struct-style format descriptor
              3, // Number of dimensions (our canvas is always RGBA)
              { static_cast<size_t>(img.height),
                static_cast<size_t>(img.width),
                static_cast<size_t>(img.channels) }, // Buffer dimensions
              { static_cast<size_t>(img.stride),
                static_cast<size_t>(img.channels),
                sizeof(unsigned char) } // Strides (in bytes) per dimension
          );
      });

//  py::class_<Matrix>(m, "Matrix", py::buffer_protocol())
//     .def_buffer([](Matrix &m) -> py::buffer_info {
//          return py::buffer_info(
//              m.data(),                               /* Pointer to buffer */
//              sizeof(float),                          /* Size of one scalar */
//              py::format_descriptor<float>::format(), /* Python struct-style format descriptor */
//              2,                                      /* Number of dimensions */
//              { m.rows(), m.cols() },                 /* Buffer dimensions */
//              { sizeof(float) * m.cols(),             /* Strides (in bytes) for each index */
//                sizeof(float) }
//          );
//      });


  //------------------------------------------------- Drawing - LineStyle
  py::class_<vivi::LineStyle> line_style(m, "LineStyle",
                                         "How a line should be drawn.");

  py::enum_<vivi::LineStyle::Cap>(line_style, "Cap",
                                  "How to render the endpoints of the line (or dash strokes).")
      .value("Butt", vivi::LineStyle::Cap::Butt)
      .value("Round", vivi::LineStyle::Cap::Round)
      .value("Square", vivi::LineStyle::Cap::Square);


  py::enum_<vivi::LineStyle::Join>(line_style, "Join",
                                   "How to render the junction of two lines/segments.")
      .value("Miter", vivi::LineStyle::Join::Miter)
      .value("Bevel", vivi::LineStyle::Join::Bevel)
      .value("Round", vivi::LineStyle::Join::Round);


  line_style.def(py::init<>(&moddef::CreateLineStyle))// init from tuple
      .def(py::init<double, vivi::Color, std::vector<double>,
                    vivi::LineStyle::Cap, vivi::LineStyle::Join>(),
           py::arg("line_width"), py::arg("color"),
           py::arg("dash_pattern")=std::vector<double>(),
           py::arg("line_cap")=vivi::LineStyle::Cap::Butt,
           py::arg("line_join")=vivi::LineStyle::Join::Miter)
      .def("__repr__",
           [](const vivi::LineStyle& st)
           { return "<vivi." + st.ToString() + ">"; })
      .def("__str__", &vivi::LineStyle::ToString)
      .def(py::pickle(&pickling::SerializeLineStyle,
                      &pickling::DeserializeLineStyle))
      .def(py::self == py::self)
      .def(py::self != py::self)
      .def_readwrite("line_width", &vivi::LineStyle::line_width, "Width in pixels (best results for even values).")
      .def_readwrite("color", &vivi::LineStyle::color, "Line color (rgba).")
      .def_readwrite("dash_pattern", &vivi::LineStyle::dash_pattern, "Dash pattern defined as list of on/off strokes (lengths in\n"
                                                                     "pixels), e.g. [20, 10, 40, 10]. If the list is empty, the\n"
                                                                     "line will be drawn solid.")
      .def_readwrite("line_cap", &vivi::LineStyle::line_cap, "How to render the endpoints of the line (or dash strokes).")
      .def_readwrite("line_join", &vivi::LineStyle::line_join, "How to render the junction of two lines/segments.");


  // A LineStyle can be initialized from a given tuple.
  py::implicitly_convertible<py::tuple, vivi::LineStyle>();


  //------------------------------------------------- Drawing - Painter
  py::class_<moddef::Painter>(m, "Painter",
          R"pbdoc(A Painter lets you draw on a canvas.

          Usage:
          1. Create a Painter
          2. set_canvas_rgb() or set_canvas_image()
          3. Draw onto the canvas via draw_xxx()
          4. When all objects have been drawn, retrieve
             the visualization via get_canvas()
          5. For the next visualization, start at (2) to
             reuse the allocated resources.
          )pbdoc")
      .def(py::init<>())
      .def("__repr__", [](const moddef::Painter &) { return "<vivi.Painter>"; })
      .def("__str__", [](const moddef::Painter &) { return "vivi.Painter"; })
      .def("set_canvas_rgb", &moddef::Painter::SetCanvasColor,
           "Initializes the canvas with the given color. This or\n"
           "set_canvas_XXX() must be called before anything can\n"
           "be drawn.",
           py::arg("width"), py::arg("height"),
           py::arg("color")=vivi::Color(0, 0, 0, 1))
      .def("set_canvas_filename", &moddef::Painter::SetCanvasFilename,
           "Initializes the canvas from the given image file.\n"
           "Supported formats are:\n"
           "  JPEG, PNG, TGA, BMP, PSD, GIF, HDR, PIC, PNM\n\n"
           "Relies on the stb library, so check for updates if\n"
           "your format is missing:\n"
           "https://github.com/nothings/stb/blob/master/stb_image.h",
           py::arg("image_filename"))
//      .def("set_canvas_image") //TODO
      .def("get_canvas", &moddef::Painter::GetCanvas,
           "Returns the current state of the visualization.\n\n"
           "If you want a copy, set copy=True. Otherwise, the buffer"
           "will just provide a view on the Painter's canvas.",
           py::arg("copy")=false) //TODO which default should be used???
      .def("draw_line", &moddef::Painter::DrawLine,
           "Draws a line between the two Vec2d coordinates using the\n"
           "LineStyle specification.",
           py::arg("from"), py::arg("to"), py::arg("line_style"))
      .def("draw_circle", &moddef::Painter::DrawCircle,
           "Draws a circle at the given Vec2d position using the\n"
           "LineStyle specification. The circle will be filled if\n"
           "a fill color with alpha > 0 is given.",
           py::arg("center"), py::arg("radius"), py::arg("line_style"),
           py::arg("fill")=vivi::Color(0, 0, 0, 0))
      .def("draw_rect", &moddef::Painter::DrawRect,
           "Draws a rectangle using the LineStyle specification.\n\n"
           "* The rectangle will be filled if fill color has alpha > 0.\n"
           "* Draw a rotated rectangle by defining the rect's angle\n"
           "  in degrees (clockwise rotation).\n"
           "* By defining the rect's corner radius, you can draw\n"
           "  a rounded rectangle.",
           py::arg("rect"), py::arg("line_style"),
           py::arg("fill")=vivi::Color(0, 0, 0, 0))
      //TODO add draw_xxx methods
      .def("show", &moddef::Painter::DummyShow, "TODO - Shows the current canvas; only used for initial development!");


#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
