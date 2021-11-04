# Simple Window Switcher
Simple Window Switcher (sws) is an Alt-Tab replacement for Windows.

![Build status](https://github.com/valinet/sws/actions/workflows/build.yml/badge.svg)

## Example usage
An application which implements this is [ExplorerPatcher](https://github.com/valinet/ExplorerPatcher). This project can also be compiled as a library and used standalone in your current workflow.

## Compiling

If you need more information, you can also consult the GitHub Actions automated build file [here](https://github.com/valinet/sws/actions/workflows/msbuild.yml).

The following prerequisites are necessary in order to compile this project:

* Microsoft C/C++ Optimizing Compiler - this can be obtained by installing either of these packages:

  * Visual Studio - this is a fully featured IDE; you'll need to check "C/C++ application development role" when installing. If you do not require the full suite, use the package bellow.
  * Build Tools for Visual Studio - this just installs the compiler, which you'll be able to use from the command line, or from other applications like CMake

  Download either of those [here](http://go.microsoft.com/fwlink/p/?LinkId=840931). The guide assumes you have installed either Visual Studio 2019, either Build Tools for Visual Studio 2019.

* A recent version of the [Windows SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk/) - for development, version 10.0.19041.0 was used, available [here](https://go.microsoft.com/fwlink/p/?linkid=2120843) (this may also be offered as an option when installing Visual Studio)

* Git - you can use [Git for Windows](https://git-scm.com/download/win), or git command via the Windows Subsystem for Linux.

Steps:

1. Clone git repo

   ```
   git clone https://github.com/valinet/sws
   ```

   If "git" is not found as a command, type its full path, or have its folder added to PATH, or open Git command window in the respective folder if using Git for Windows.

2. Compile sws

   * Double click the `SimpleWindowSwitcher.sln` file to open the solution in Visual Studio. Choose Release and your processor architecture in the toolbar. Press `[Ctrl]`+`[Shift]`+`[B]` or choose "Build" - "Build solution" to compile.

   * Open an "x86 Native Tools Command Prompt for VS 2019" (for x86), or "x64 Native Tools Command Prompt for VS 2019" (for x64) (search that in Start), go to folder containing solution file and type:

     * For x86:

       ```
       msbuild ExplorerPatcher.sln /property:Configuration=Release /property:Platform=x86
       ```

     * For x64:

       ```
       msbuild ExplorerPatcher.sln /property:Configuration=Release /property:Platform=x64
       ```

   The resulting libraries will be in the "Release" (for x86) or "x64\Release" (for x64) folder in the directory containing the solution file.

That's it.
