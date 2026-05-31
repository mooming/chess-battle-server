# Chess Battle Server Agent Guidance

## Source Code
- Main application: `ChessJam/Main.cpp`
- Source files located in `ChessJam/` directory

## Usage
Run the compiled executable with one of:
- `Monitor`
- `Match <Monitor-Address>`
- `Judge <Monitor-Address>`
- `Server <Monitor-Address>`
- `Client <Monitor-Address>`
Invalid arguments print usage.

## Build
- Visual Studio solution: `ChessJam/ChessJam.sln`
- Project file: `ChessJam/ChessJam.vcxproj`
- Build using Visual Studio or MSBuild (Windows) or compatible toolchain.

## Notes
- No test or lint configurations present in repository.
- Protocol details defined in README.md.