# ImageExplorer2

A Visual Studio 2022 C++ (WinUI 3 / C++/WinRT) Windows desktop app that browses image folders in an Explorer-style thumbnail gallery.

## Supported formats
- PNG
- JPG / JPEG
- GIF
- SVG

## Build in Visual Studio
1. Open `ImageExplorer.sln` in Visual Studio 2022 17.10+.
2. Restore NuGet packages.
3. Select `Debug|x64` or `Release|x64`.
4. Build and run `ImageExplorer`.

## Build from Developer Command Prompt
```powershell
nuget restore ImageExplorer.sln
msbuild ImageExplorer.sln /p:Configuration=Release /p:Platform=x64
```

Output executable should be under `bin\Release\ImageExplorer.exe`.
