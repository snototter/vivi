#ifndef __VIVI_COLORS_H__
#define __VIVI_COLORS_H__

#include <memory>
#include <algorithm>
#include <string>
#include <tuple>

namespace vivi
{
template<typename _Tp>
_Tp saturation_cast(_Tp val, _Tp low, _Tp high)
{
  return std::min(std::max(val, low), high);
}


/**
 * @brief Represents a rgba color
 *
 * Convention:
 * * RGB (upper case letters) are in range [0, 255]
 * * rgb (lower case letters) are in range [0, 1]
 *
 * For painting with Cairo, we need color values within [0, 1].
 * For convenience, you can use the derived @see ColorRGB
 *
 * TODO nice-to-have: add conversions from other color spaces (such as HSV, CMYK)
 *      --> not now, we're almost exclusively working with RGB/rgb
 */
struct Color
{
  double red;   /**< Red component within [0,1]. */
  double green; /**< Green component within [0,1]. */
  double blue;  /**< Blue component within [0,1]. */
  double alpha; /**< Alpha (opacity) component within [0,1]. */

  Color() : red(0.0), green(0.0), blue(0.0), alpha(1.0)
  {}

  Color(double r, double g, double b, double a)
    : red(saturation_cast<double>(r, 0.0, 1.0)),
      green(saturation_cast<double>(g, 0.0, 1.0)),
      blue(saturation_cast<double>(b, 0.0, 1.0)),
      alpha(saturation_cast<double>(a, 0.0, 1.0))
  {}

  /**
   * @brief Returns the inverse color, i.e. (1.0-r, 1.0-g, 1.0-b). Alpha value stays the same.
   *
   * Caveat: TODO 50% gray inverse = 50% gray in rgb color space --> use opposite color
   */
  Color Inverse() const;

  std::string ToString() const;

  /** @brief Returns the corresponding (R, G, B, a) tuple, where R, G, B in [0, 255] and alpha in [0, 1]. */
  std::tuple<unsigned char, unsigned char, unsigned char, double>
  ToRGBA() const;


  /** @brief Returns the web color code, e.g. "#dcdce4" (alpha value is ignored). */
  std::string ToHexString() const;
};

inline bool operator==(const Color& lhs, const Color& rhs)
{
  // TODO approx equal - https://stackoverflow.com/questions/17333/what-is-the-most-effective-way-for-float-and-double-comparison
  return lhs.red == rhs.red && lhs.green == rhs.green
      && lhs.blue == rhs.blue && lhs.alpha == rhs.alpha;
}

inline bool operator!=(const Color& lhs, const Color& rhs)
{
  return !(lhs == rhs);
}


/** Convenience wrapper to initialize @see Color from rgb values in range [0,1]. */
Color rgba(double r, double g, double b, double alpha=1.0);

/** Convenience wrapper to initialize @see Color from RGB values in range [0,255]. */
Color RGBA(double R, double G, double B, double alpha=1.0);

} // namespace vivi

#endif // __VIVI_COLORS_H__

