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

## Structure

* Abstract Backend: Inherits ctModuleBase, Interface into a device API (can specify the max number of instances)
* Abstract Device Instances: Instance of the device which can be asigned to a player. Gets fed player information for the backend to digest.
* Player: A player is a container for diffrent types of device instances and a haptic mixer/action manager. Players are defined by gameplay and are *NOT* destroyed when their devices disconnect.
	* Action: Allows game to define/transition input states, read gamepad buttons, Keys, Sticks, Triggers, Relative Mouse, Etc., abstracted to normalized analogue signals/pulses.
	* Haptic Mixer: Defines a handle indexed 3D scene for each controller where vibrations are "mixed" (or queued/ejected), the vibrations either use wave definitions or optional file names for controllers that support hd rumble. A compatible haptic device will be fed the scene state and will be responsible for conveying the resulting haptic response.
	* Cursor: A 2D pointer-onscreen in absolute normalized coordinates, some devices support hiding the cursor/changing appearance.
	* Text IO: Opens device text input and retrieves the results. The resulting text is a preview of the results until confirmation is raised, on consoles this will be immediate.
	* Spacial: Describes a 3D scene of named point(s), can be used for VR/3D controllers/Mocap/Gyro/Etc
	* Info Queries: Gives info such as active devices type, name, resources for displaying action buttons/controllers/etc if availible.
	* Settings: Allows players to create/store/load profiles for things like input action mappings and game specific per-player payloads like accessibility options.
	* Message Queues: A way of sending and recieving event structures to a device. Anything devices need for communication that are not suitable for other categories should be here. Messages come in the form of structs.
* Device Manager: Devices are not immediately assigned to a player, they need to be assigned, when a device interface deems acceptable a device will be added and await assignment to a player, when a device is disconnected its association with the player is removed. Assigning to the player may fail if the backend deems it unsuitable. Devices in here will list their name/type/icon (if applicable) but do not accept any other meaningful input/output.
* Environment Capabilities: Gives a yes/no about support for diffrent IO features in enum. Filled in by registered backends. Does not necessarily mean that a device is availible/connected, just that it's family of features are supported.

Already the possibilities in player interface are huge, and while interact will likely not have many of these for quite some time to come (possibly ever) it does give a solid direction for expansion while minimally intruding on anything sitting above it.

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

## Interact's Internal Actions are not King

The responsibility of mapping action state and bindings to devices and buttons are per-backend. This is done because some backends already handle bindings at a deeper level, in which case the binding settings will have no affect to avoid creating a mess. If this is the case a capability flag is raised. The primary goal of the action API is to be compatible to be translated to OpenXR/Steam Input. Interact's action system will therfore be data driven by the settings system by loading configs where the mapping is not provided by the backend to avoid hard-coded button mapping issues on APIs that don't support/wouldn't map or want to map buttons progammatically.

## Backends

Backends are code paths can be optionaly compiled per-platform and implement device interfaces and instances. These backends are responsible for implementing the actual funtionality behind the player state and either react or respond. Many of these backends may be short-lived/hacky/proprietary so exclusion from the codebase should not impact anything exept the ability to use a device. When conflicting backends are possible (ex: SDL/Steam Input) the user should be able to choose which one takes dominance.

## Suggested Input Paths

Here we go... All of the inputs that can be suggested to the backend to use for a action binding.

### Keyboards
	
* /keyboard/shift
* /keyboard/ctrl
* /keyboard/alt
* /keyboard/keys/[KEY_CODE] ( follows the SDL keycode convention, see: https://wiki.libsdl.org/SDL_Keycode )
	* /keyboard/keys/a
	* /keyboard/keys/6
	* /keyboard/keys/BACKSLASH
	* /keyboard/keys/F9
	* (etc...)

### Mice

Absolute values are always only availible from cursor, this maps relative to action

* /mouse/relative_move/x
* /mouse/relative_move/y
* /mouse/scroll/x
* /mouse/scroll/y
* /mouse/button/left
* /mouse/button/right
* /mouse/button/middle

### Gamepads

Gyro is handled by spacial

Pads/pointing is handled by cursor

Half a Switch controller drops triggers, select and right stick

* /gamepad/a
* /gamepad/b
* /gamepad/x
* /gamepad/y
* /gamepad/dpad/up
* /gamepad/dpad/down
* /gamepad/dpad/left
* /gamepad/dpad/right
* /gamepad/shoulder_left
* /gamepad/shoulder_right
* /gamepad/trigger_left
* /gamepad/trigger_right
* /gamepad/thumbstick_left/click
* /gamepad/thumbstick_right/click
* /gamepad/thumbstick_left/x
* /gamepad/thumbstick_left/y
* /gamepad/thumbstick_right/x
* /gamepad/thumbstick_right/y
* /gamepad/start
* /gamepad/select
* /gamepad/guide (not reccomended)

### Virtual Reality

In the event that OpenXR is not supported for a vr device these paths will be emulated by the backend for the target device

Action types will all be mapped/split to 1D, pose is not supported here and will be mapped to spacial

* /xr/[path/..] See ( https://www.khronos.org/registry/OpenXR/specs/1.0/html/xrspec.html#semantic-path-interaction-profiles )
	* /xr/user/hand/left/input/trackpad/click
	* /xr/user/gamepad/input/a/click
	* /xr/user/hand/right/input/back/click
	* (etc...)