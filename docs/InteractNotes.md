# Explaining Interact

## Problem

It's simple right? Press key, check if key is down, do something right? Well not really... Game IO is often a very complex subject with many different types of devices that can be used, you can certainly just use said device's API directly in the game logic... But that will be a mess and will discourage supporting more devices that in theory should not be that difficult to add support for while impacting the project's ability to easily port or disclose code.

Types of "devices":
* Keyboards
* Mice
* Gamepads
* Joysticks
* Steering Wheels
* VR Headsets
* VR Controllers
* Insertables
* Gyro
* Zappers/Blasters
* Eye Tracking
* RGB Lighting
* Streaming Service Chats
* Etc...

What is **NOT** a "device" (drawing the line in the sand)
* Multiplayer
* Console Commands
* Audio Engines
* Rendering
* Windows
* *Interact* is only meant to assist human *interactivity*.

Soo many different devices are possible, many with their own requirements. We want to avoid poluting scene/game space with device specific APIs and make it easy to integrate new devices later down the line. Libraries like SDL and OpenXR take care of some of this work but is too fine-grain in terms of device management and doesn't provide enough structure on their own. Managing the lifespan of devices and dealing with proprietary APIs or potential device bugs and workarounds should **NEVER** wind up in gameplay code. So we will design an API that genericizes most uses from the gameplay side into an easy to understand API. 

## About Namings in Bindings

Binding button names should also be kept "generic", but in reality different console controllers have diffrent button layouts which may complicate things. To help aleviate this the following remapping between controllers will be used:

XBox's face button layout is assumed and will be remapped for other controllers:

* XBox: 
	* A,B,X,Y = A,B,X,Y
	* Start, Select = Start, Back
* Playstation: 
	* A,B,X,Y = Cross, Circle, Square, Triangle
	* Start, Select = Options, Trackpad Down (Share on PC)
* Switch:
	* A,B,X,Y = B,A,Y,X
	* Start, Select = +,-
	
Keep in mind that on many platforms buttons can be remapped regardless, so this naming convention is already sitting on top of many layers. The intention here is to keep as smooth as an experience when moving accross all platforms.

## Backends

Backends are code paths can be optionaly compiled per-platform and implement device interfaces and instances. These backends are responsible for implementing the actual funtionality behind the player state and either react or respond. Many of these backends may be short-lived/hacky/proprietary so exclusion from the codebase should not impact anything exept the ability to use a device. When conflicting backends are possible (ex: SDL/Steam Input) the user should be able to choose which one takes dominance.

## Device Paths

### Keyboards
	
* /dev/keyboard/input/scancode/[SCAN_CODE] **( Follows the SDL keycode convention, see: https://wiki.libsdl.org/SDL_Scancode )**

### Mice

Absolute values are always only availible from cursor, this maps relative to action

* /dev/mouse/input/relative_move/x
* /dev/mouse/input/relative_move/y
* /dev/mouse/input/scroll/x
* /dev/mouse/input/scroll/y
* /dev/mouse/input/button/left
* /dev/mouse/input/button/right
* /dev/mouse/input/button/middle

### Gamepads

Gyro is handled by spacial

Pads/pointing is handled by cursor

Half a Switch controller drops triggers, select and right stick

* /dev/gamepad/input/a
* /dev/gamepad/input/b
* /dev/gamepad/input/x
* /dev/gamepad/input/y
* /dev/gamepad/input/dpad/up
* /dev/gamepad/input/dpad/down
* /dev/gamepad/input/dpad/left
* /dev/gamepad/input/dpad/right
* /dev/gamepad/input/shoulder_left
* /dev/gamepad/input/shoulder_right
* /dev/gamepad/input/trigger_left
* /dev/gamepad/input/trigger_right
* /dev/gamepad/input/thumbstick_left/click
* /dev/gamepad/input/thumbstick_right/click
* /dev/gamepad/input/thumbstick_left/x
* /dev/gamepad/input/thumbstick_left/y
* /dev/gamepad/input/thumbstick_right/x
* /dev/gamepad/input/thumbstick_right/y
* /dev/gamepad/input/start
* /dev/gamepad/input/select
* /dev/gamepad/input/guide **( Not reccomended )**

### Virtual Reality

In the event that OpenXR is not supported for a vr device these paths will be emulated by the backend for the target device

Action types will all be mapped/split to 1D, pose is not supported here and will be mapped to spacial

Supported components: click, force, value, x/y, twist

* /dev/xr/[path/..] **( See https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#semantic-path-interaction-profiles )**
	* /dev/xr/user/hand/left/input/trackpad/click
	* /dev/xr/user/gamepad/input/a/click
	* /dev/xr/user/hand/right/input/back/click
	* (etc...)
	
### Joysticks

Joysticks are an all-encompassing term for unknown inputs and are a bit more of a mess in terms of ecosystem. This will likely need manual bindings and community contribution for each device.

* /dev/sdl-joystick/input/[GUID]/axis/[#]
* /dev/sdl-joystick/input/[GUID]/ball/[#]/x
* /dev/sdl-joystick/input/[GUID]/ball/[#]/y
* /dev/sdl-joystick/input/[GUID]/button/[#]
* /dev/sdl-joystick/input/[GUID]/hat/[#]/north
* /dev/sdl-joystick/input/[GUID]/hat/[#]/south
* /dev/sdl-joystick/input/[GUID]/hat/[#]/east
* /dev/sdl-joystick/input/[GUID]/hat/[#]/west
* /dev/sdl-joystick/input/[GUID]/hat/[#]/northeast
* /dev/sdl-joystick/input/[GUID]/hat/[#]/southeast
* /dev/sdl-joystick/input/[GUID]/hat/[#]/southwest
* /dev/sdl-joystick/input/[GUID]/hat/[#]/northwest

## Interact 2.0

Interact 1.0 had a solid foundation of unifying all input types, unfortunately it has some key problems.

1. Lack of multi-player handling (you could technically subpath them)
1. Controllers were tied to the binding (switching slots meant switching bindings)
1. Keymaps were bound to filtering functionality
1. Fixed slot and fixed function filters were limiting and awkward
1. Retrieval of info about inputs was not provided
1. Code structure was sloppy
1. Scalars were heavily assumed and other path types were a second thought

These points will be addressed one by one.

### Multiple Players are Introduced at the API level

On startup the application reserves a certain number of player slots.

Each slot has their own directory that is isolated from other players

Now when you get a signal you have to specify the player index.

### Devices are explicitly assigned to players

All devices must be registered with a player. When a device is registered it will write to that players directory.

Gamepad number has been removed.

### Keymaps are now a seperate config file

There is now a user file and a action file

The user file contains a list of parameters such as action to key bindings for each device and accesibility settings

The action file is now something entirely different and implement control/action logic while referencing the user file as a input

### Actions are now handled by lua script

The action file is now a lua script that implements all the control logic itself. On startup it registers a list of action paths.
The lua script is fed the user file as a dictionary and given an API to interact with paths. This fits great as just another backend, simplifying the core of interact greatly. One Lua context exists per player.

1. Startup: used to register actions
1. Poll: used to update action values

### Metadata paths

* /active-dev: returns the last active device associated with the user
* /action/XXX/inputs: returns a delimiter separated list of paths linked to the action, this can be filtered out by device name (such as active dev)
* /dev/###/INPUT/description: returns metadata about the input (such as the icon and description)

### Refactor in OOP

Controversial decision but following cleaner OOP guidelines, using inheritance, etc will greatly improve this non-performance critical, highly conceptually driven part of the codebase.

### Paths are now properly types

Paths are now typed by base class (casting wont have metadata so we will have to use a type enum/strings), this allows paths to have better type distinction.
