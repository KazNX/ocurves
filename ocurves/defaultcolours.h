//
// author Kazys Stepanas
//
// Copyright (c) CSIRO 2015
//
#ifndef DEFAULTCOLOURS_H_
#define DEFAULTCOLOURS_H_

#include "ocurvesconfig.h"

#include <QRgb>

namespace ocurves
{
  /// An enumeration of the web safe colour set. The colour set is fully defined in @c WebSafeColours
  enum ColourName
  {
    // Blacks.
    Gainsboro,
    LightGrey,
    Silver,
    DarkGrey,
    Grey,
    DimGrey,
    LightSlateGrey,
    SlateGrey,
    DarkSlateGrey,
    Black,

    // Whites
    White,
    Snow,
    Honeydew,
    MintCream,
    Azure,
    AliceBlue,
    GhostWhite,
    WhiteSmoke,
    Seashell,
    Beige,
    OldLace,
    FloralWhite,
    Ivory,
    AntiqueWhite,
    Linen,
    LavenderBlush,
    MistyRose,

    // Pinks
    Pink,
    LightPink,
    HotPink,
    DeepPink,
    PaleVioletRed,
    MediumVioletRed,

    // Reds
    LightSalmon,
    Salmon,
    DarkSalmon,
    LightCoral,
    IndianRed,
    Crimson,
    FireBrick,
    DarkRed,
    Red,

    // Oranges
    OrangeRed,
    Tomato,
    Coral,
    DarkOrange,
    Orange,

    // Yellows
    Yellow,
    LightYellow,
    LemonChiffon,
    LightGoldenrodYellow,
    PapayaWhip,
    Moccasin,
    PeachPuff,
    PaleGoldenrod,
    Khaki,
    DarkKhaki,
    Gold,

    // Browns
    Cornsilk,
    BlanchedAlmond,
    Bisque,
    NavajoWhite,
    Wheat,
    BurlyWood,
    Tan,
    RosyBrown,
    SandyBrown,
    Goldenrod,
    DarkGoldenrod,
    Peru,
    Chocolate,
    SaddleBrown,
    Sienna,
    Brown,
    Maroon,

    // Greens
    DarkOliveGreen,
    Olive,
    OliveDrab,
    YellowGreen,
    LimeGreen,
    Lime,
    LawnGreen,
    Chartreuse,
    GreenYellow,
    SpringGreen,
    MediumSpringGreen,
    LightGreen,
    PaleGreen,
    DarkSeaGreen,
    MediumSeaGreen,
    SeaGreen,
    ForestGreen,
    Green,
    DarkGreen,

    // Cyans
    MediumAquamarine,
    Aqua,
    Cyan,
    LightCyan,
    PaleTurquoise,
    Aquamarine,
    Turquoise,
    MediumTurquoise,
    DarkTurquoise,
    LightSeaGreen,
    CadetBlue,
    DarkCyan,
    Teal,

    // Blues
    LightSteelBlue,
    PowderBlue,
    LightBlue,
    SkyBlue,
    LightSkyBlue,
    DeepSkyBlue,
    DodgerBlue,
    CornflowerBlue,
    SteelBlue,
    RoyalBlue,
    Blue,
    MediumBlue,
    DarkBlue,
    Navy,
    MidnightBlue,

    // Purples
    Lavender,
    Thistle,
    Plum,
    Violet,
    Orchid,
    Fuchsia,
    Magenta,
    MediumOrchid,
    MediumPurple,
    BlueViolet,
    DarkViolet,
    DarkOrchid,
    DarkMagenta,
    Purple,
    Indigo,
    DarkSlateBlue,
    SlateBlue,
    MediumSlateBlue,

    ColourCount
  };

  /// A set of predefined colours implementing the web safe colour set.
  /// Index with values from @c ColourName.
  ///
  /// See <a href="https://en.wikipedia.org/wiki/Web_colors">en.wikipedia.org/wiki/Web_colors</a> or
  /// <a href="http://websafecolors.info/">websafecolors.info</a>
  extern QRgb WebSafeColours[ColourCount];

  /// An enumeration of the various default colour sets used for plotting.
  ///
  /// The primary set is the @c StandardColours set, while other sets attempt
  /// to mitigate colour-blindness issues. The colour blind sets are coarsely
  /// and simplistically defined and cannot cater for all levels of
  /// colour-blindness and may require further modification.
  ///
  /// These are used to index @c DefaultColourSets and @c DefaultColourSets.
  enum DefaultColourSet
  {
    /// Standard colours, cycling through various hues in the web safe colours.
    StandardColours,
    /// Colours addressing deuteranomaly colour blindness.
    DeuteranomalyColours,
    /// Colours addressing protanomaly colour blindness.
    ProtanomalyBlindColours,
    /// Colours addressing tritanomaly colour blindness.
    TritanomalyBlindColours,
    GreyScaleColours,
    ColourSetCount
  };

  /// Colour set names for the default colour sets. English names only, to be used
  /// with Qt's @c tr() macro.
  extern const char *DefaultColourSetNames[ColourSetCount];

  /// Defines various colour combinations to use as the default plotting colours.
  ///
  /// There is one set for each @c DefaultColourSet (excluding @c ColourSetCount).
  /// The number of items in each set is defined in @c DefaultColourCounts using
  /// the same @c DefaultColourSetCounts value as an index.
  ///
  /// Each set contains @c ColourName values which can be used to index into @c
  /// WebSafeColours.
  extern const ColourName *DefaultColourSets[ColourSetCount];

  /// The number of elements in each @c DefaultColourSets set.
  extern const unsigned DefaultColourSetCounts[ColourSetCount];
}

#endif // DEFAULTCOLOURS_H_
