{
  lib,
  stdenv,
  cmake,
  pkg-config,
  kdePackages,
  exiv2,
  mpv,
  opencv4,
  src ? null
}:

stdenv.mkDerivation {
  pname = "qimgv";
  version = "unstable";

  src = if src != null then src else throw "A source directory must be provided via the 'src' argument";

  nativeBuildInputs = [
    cmake
    pkg-config
    kdePackages.wrapQtAppsHook
  ];

  cmakeFlags = [
    "-DVIDEO_SUPPORT=ON"
    "-DUSE_QT5=OFF"
    "-DKDE_SUPPORT=ON"
  ];

  buildInputs = [
    exiv2
    mpv
    opencv4
    kdePackages.qtbase
    kdePackages.qtimageformats
    kdePackages.qtsvg
    kdePackages.qttools
    kdePackages.kimageformats
    kdePackages.kwindowsystem
  ];

  postPatch = ''
    sed -i "s@/usr/bin/mpv@${mpv}/bin/mpv@" \
      qimgv/settings.cpp
  '';

  qtWrapperArgs = [
    "--prefix LD_LIBRARY_PATH : ${placeholder "out"}/lib"
  ];

  meta = with lib; {
    description = "Qt6 image viewer with optional video support";
    mainProgram = "qimgv";
    homepage = "https://github.com/easymodo/qimgv";
    license = licenses.gpl3;
    platforms = platforms.linux;
    maintainers = with maintainers; [ cole-h ];
  };
}
