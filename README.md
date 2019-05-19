# hotkeyz
Jeremy Bell's Hotkeyz to teach people to use ProTools

# How to work with the code

In `app_src` there is an XCode project that you can open and use to build, run and package the binary as a Mac .app file.
Resources are all located in the resources folder (.wav, fonts, .pngs etc.)

The project should work out of the box (except for Code signing):

    $ git clone https://github.com/divyekapoor/hotkeyz
    $ cd hotkeyz/
    $ open app_src/Hotkeyz/Hotkeyz.xcodeproj

To make code signing work, you will need to sign up for an Apple Developer account and use your personal credentials in the settings.

# How to create the .app file

In XCode go to Product -> Archive and that will generate a new .app file with the updated code. Open the file in finder.
The .app format is just a simple folder structure. In order to distribute this
file to your friends, just zip or tar it with any utility and send it to them.


# How to set up the project in case of a corruption

The best way to recover from the corruption is to start again from the delivered XCode project. Trying the steps below takes a *lot* of time and there are many ways to mess up. However, the instructions are located here in case they need to be repeated.

Should you have to make this project from scratch (really not recommended), please keep the following things in mind:
1. Create the project as a Cocoa App (not a command line tool) and name it Hotkeyz.
1. Delete all files other than Assets.xcassets, MainMenu.xlb, Info.plist, Hotkeyz.entitlements (all the code including main).
1. Right click -> create a new file -> main.cpp -> copy in all the code into this file.
1. Add --deep to Other Code Signing flags in Build Settings
1. Add the `SDL_*` frameworks from `framework_files` to Hotkeyz -> Embedded Binaries. Make sure they are also present in the Linked Libraries section. (Make sure you use Add Other... to add these files in).
1. Add Cocoa Framework to the list of linked libraries as well (required to access resources). Do not use Add other.
1. Create a new Group (Resources) and add all the Resources into this.
1. Go to Build Phases and add a Copy Files rule to include Resources into the Resources folder. You should see Copy Bundle Resources (157 items).
1. After all this, In Build Phases you should see 0 Target Dependencies, 1 Compile Sources (main.cpp), 9 Link Binary with Libraries (4x SDL x2 + 1 Cocoa Framework), Copy Bundle Resources (157 items), Embed Frameworks (4 items).
1. Make sure Build Settings `Runpath Search Paths` has a custom entry of `@executable_path/../Frameworks` and Framework Search Paths has /Library/Frameworks present (the second one is optional).
1. To Test that your app is working, run it.

## Common failures

Some common failures that can easily be fixed:
1. Code signing: Please make sure --deep is in the Other code signing flags and that you have linked your App Store dev account correctly.
1. Resources not loading: Check if the Copy Bundle Resources is correctly configured in the Build Phases.
1. Debugging App contents: Finder -> Right click -> Show contents
1. dyld: Image not found: this means that the framework files are not being copied into the .app file. The framework files need to be included into Embedded Binaries and the Link Libraries and Frameworks section. Please also make sure the `Runpath Search Paths` has the entry `@executable_path/../Frameworks`.

# License

This is a work of hire and Jeremy Bell owns all copyright (except for licenses associated with the SDL libraries).
A copy of the SDL licenses is available at the root of the repo (SDL_License.txt) and is also present in the framework files.
