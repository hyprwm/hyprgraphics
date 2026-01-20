{
  lib,
  stdenv,
  stdenvAdapters,
  cmake,
  pkg-config,
  cairo,
  file,
  hyprutils,
  libGL,
  libdrm,
  libheif,
  libjpeg,
  libjxl,
  librsvg,
  libspng,
  libwebp,
  pango,
  pixman,
  version ? "git",
  doCheck ? false,
  debug ? false,
}: let
  inherit (builtins) foldl';
  inherit (lib.lists) flatten;
  inherit (lib.sources) cleanSource cleanSourceWith;
  inherit (lib.strings) hasSuffix;

  adapters = flatten [
    stdenvAdapters.useMoldLinker
    (lib.optional debug stdenvAdapters.keepDebugInfo)
  ];

  customStdenv = foldl' (acc: adapter: adapter acc) stdenv adapters;
in
  customStdenv.mkDerivation {
    pname = "hyprgraphics";
    inherit version doCheck;

    src = cleanSourceWith {
      filter = name: _type: let
        baseName = baseNameOf (toString name);
      in
        ! (hasSuffix ".nix" baseName);
      src = cleanSource ../.;
    };

    nativeBuildInputs = [
      cmake
      pkg-config
    ];

    buildInputs = [
      cairo
      file
      hyprutils
      libGL
      libdrm
      libheif
      libjpeg
      libjxl
      librsvg
      libspng
      libwebp
      pango
      pixman
    ];

    outputs = ["out" "dev"];

    cmakeBuildType =
      if debug
      then "Debug"
      else "RelWithDebInfo";

    meta = with lib; {
      homepage = "https://github.com/hyprwm/hyprgraphics";
      description = "Small C++ library with graphics / resource related utilities used across the hypr* ecosystem";
      license = licenses.bsd3;
      platforms = platforms.linux;
    };
  }
