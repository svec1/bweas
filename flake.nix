{
  description = "A very basic flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
	flake-utils.url = "github:numtide/flake-utils";  
  };

  outputs = { self, nixpkgs, flake-utils }: flake-utils.lib.eachDefaultSystem (system:
      let
      	pkgs = import nixpkgs { inherit system; };
      	cmakeBuild = pkgs.stdenv.mkDerivation {
        	pname = "bweas";
        	version = "0.1.0";
        	src = ./.;

        	buildInputs = [ pkgs.cmake pkgs.lz4 pkgs.nlohmann_json pkgs.luajit];
		  };
       in {
         packages.default = cmakeBuild;
       });
}
