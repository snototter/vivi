#include <sstream>
#include <stdexcept>

#include <pybind11/operators.h>
#include <pybind11/numpy.h>

#include <bindings/binding_helpers.h>
#include <viren2d/primitives.h>

namespace py = pybind11;

namespace viren2d {
namespace bindings {
//------------------------------------------------- ImageBuffer from numpy array
// We need a uint8, row-major (C-style) numpy array:
ImageBuffer CreateImageBuffer(
    py::array_t<unsigned char, py::array::c_style | py::array::forcecast> buf,
    bool copy) {
  // Sanity checks
  if (buf.ndim() < 2 || buf.ndim() > 3)
    throw std::runtime_error("Incompatible image dimension!");

  if (!buf.dtype().is(py::dtype::of<uint8_t>()))
    throw std::runtime_error("Incompatible format: expected a uint8 array!");

  ImageBuffer img;
  const int row_stride = static_cast<int>(buf.strides(0));
  const int height = static_cast<int>(buf.shape(0));
  const int width = static_cast<int>(buf.shape(1));
  const int channels = (buf.ndim() == 2) ? 1 : static_cast<int>(buf.shape(2));
  if (copy) {
    img.CreateCopy(buf.data(), width, height, channels, row_stride);
  } else {
    img.CreateSharedBuffer(buf.mutable_data(), width, height, channels, row_stride);
  }
  return img;
}

void RegisterImageBuffer(py::module &m) {
  //TODO class docstr states grayscale, rgb or rgba --> we might also want
  //to visualize flow in the distant future (for now, use vito.flowutils)

  // Info on numpy memory layout: https://stackoverflow.com/a/53099870/400948
  py::class_<ImageBuffer>(m, "ImageBuffer", py::buffer_protocol(),
           "An ImageBuffer holds 8-bit images (Grayscale,\n"
           "RGB or RGBA).")
      .def(py::init(&CreateImageBuffer), R"docstr(
          Creates an :class:`~viren2d.ImageBuffer` from a :class:`numpy.ndarray`.

          Currently, only conversion from/to NumPy arrays with
          :class:`numpy.dtype` = :class:`numpy.uint8` is supported.
          This will change once I get around to implementing the
          pseudocoloring functionality in ``viren2d``.

          Args:
            array: The :class:`numpy.ndarray` holding the image data.
            copy: If ``True``, the :class:`~viren2d.ImageBuffer` will
              make a copy of the given ``array``. The default (``False``)
              is to share the data instead, which avoids memory allocation.
          )docstr", py::arg("array"), py::arg("copy")=false)
     .def_buffer([](ImageBuffer &img) -> py::buffer_info {
          return py::buffer_info(
              img.data, sizeof(unsigned char), // Pointer to data & size of each element
              py::format_descriptor<unsigned char>::format(), // Python struct-style format descriptor
              3, // We'll always return ndim=3 arrays
              { static_cast<size_t>(img.height),
                static_cast<size_t>(img.width),
                static_cast<size_t>(img.channels) }, // Buffer dimensions
              { static_cast<size_t>(img.stride),
                static_cast<size_t>(img.channels),
                sizeof(unsigned char) } // Strides (in bytes) per dimension
          );
      })
      .def("copy", [](const ImageBuffer &buf) {
             ImageBuffer cp;
             cp.CreateCopy(buf.data, buf.width, buf.height, buf.channels, buf.stride);
             return cp;
           }, R"docstr(
        Returns a deep copy.

        The returned copy will **always** allocate and copy the memory,
        even if you call this method on a *shared* buffer.
        )docstr")
      .def("is_valid", &ImageBuffer::IsValid,
           "Returns ``True`` if this buffer points to a valid memory location.")
      .def("flip_channels", &ImageBuffer::RGB2BGR,
           "Swaps the red and blue color channels **in-place**.")
      .def("to_rgb", &ImageBuffer::ToRGB,
           "Returns a copy of this buffer in **RGB** format.\n\n"
           "Note that this call will always return a copy, even if\n"
           "this :class:`~viren2d.ImageBuffer` is already RGB.")
      .def("to_rgba", &ImageBuffer::ToRGBA,
           "Returns a copy of this buffer in **RGBA** format.\n\n"
           "Note that this call will always return a copy, even if\n"
           "this :class:`~viren2d.ImageBuffer` is already RGBA.")
      .def("__repr__",
           [](const ImageBuffer &)
           { return FullyQualifiedType("ImageBuffer", true); })
      .def("__str__", &ImageBuffer::ToString)
      .def_readonly("width", &ImageBuffer::width,
           "int: Image width in pixels (read-only).")
      .def_readonly("height", &ImageBuffer::height,
           "int: Image height in pixels (read-only).")
      .def_readonly("channels", &ImageBuffer::channels,
           "int: Number of channels (read-only).")
      .def_readonly("stride", &ImageBuffer::stride,
           "int: Stride in bytes per row  (read-only).")
      .def_readonly("owns_data", &ImageBuffer::owns_data_,
           "bool: Read-only flag indicating whether this\n"
           ":class:`~viren2d.ImageBuffer` owns the\n"
           "image data (and is responsible for cleaning up).");

  // An ImageBuffer can be initialized from a numpy array
  py::implicitly_convertible<py::array, ImageBuffer>();
}
} // namespace bindings
} // namespace viren2d

