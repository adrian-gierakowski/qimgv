{
  description = "Qt6 image viewer with optional video support";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }: let
    supportedSystems = [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];
    forAllSystems = nixpkgs.lib.genAttrs supportedSystems;
  in {
    packages = forAllSystems (system: let
      pkgs = nixpkgs.legacyPackages.${system};
    in {
      default = pkgs.stdenv.mkDerivation {
        pname = "qimgv";
        version = "unstable";

        src = self;

        nativeBuildInputs = with pkgs; [
          cmake
          pkg-config
          kdePackages.wrapQtAppsHook
        ];

        cmakeFlags = [
          "-DVIDEO_SUPPORT=ON"
          "-DUSE_QT5=OFF"
          "-DKDE_SUPPORT=ON"
        ];

        buildInputs = with pkgs; [
          exiv2
          mpv
          opencv4.cxxdev
          kdePackages.qtbase
          kdePackages.qtimageformats
          kdePackages.qtsvg
          kdePackages.qttools
          kdePackages.kimageformats
          kdePackages.kwindowsystem
        ];

        postPatch = ''
          sed -i "s@/usr/bin/mpv@${pkgs.mpv}/bin/mpv@" \
            qimgv/settings.cpp
        '';

        qtWrapperArgs = [
          "--prefix LD_LIBRARY_PATH : ${placeholder "out"}/lib"
        ];

        meta = with pkgs.lib; {
          description = "Qt6 image viewer with optional video support";
          mainProgram = "qimgv";
          homepage = "https://github.com/easymodo/qimgv";
          license = licenses.gpl3;
          platforms = platforms.linux;
          maintainers = with maintainers; [ cole-h ];
        };
      };
    });

    apps = forAllSystems (system: {
      default = {
        type = "app";
        program = "${self.packages.${system}.default}/bin/qimgv";
      };
    });
  };
}
