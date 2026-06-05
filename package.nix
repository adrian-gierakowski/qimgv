{
  lib,
  stdenv,
  cmake,
  pkg-config,
  kdePackages,
  exiv2,
  mpv,
  opencv4,
  src ? lib.cleanSource ./.
}:

stdenv.mkDerivation {
  pname = "qimgv";
  version = "unstable";

  inherit src;

  nativeBuildInputs = [
    cmake
    pkg-config
    kdePackages.wrapQtAppsHook
  ];

  cmakeFlags = [
    "-DVIDEO_SUPPORT=ON"
    "-DUSE_QT5=OFF"
    "-DKDE_SUPPORT=ON"
    "-DBUILD_TESTS=ON"
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

  doCheck = true;
  preCheck = "export QT_QPA_PLATFORM=offscreen";

  meta = with lib; {
    description = "Qt6 image viewer with optional video support";
    mainProgram = "qimgv";
    homepage = "https://github.com/easymodo/qimgv";
    license = licenses.gpl3;
    platforms = platforms.linux;
    maintainers = with maintainers; [ cole-h ];
  };
}
