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
* Game Account Services
* Streaming Service Chats
* Etc...

What is **NOT** a "device" (drawing the line in the sand)
* Multiplayer (WAY too specific)
* Console Commands (Too essential)
* Audio Engines (Too varied)
* Rendering (Too complex)
* Windows (This is a softer no, maybe needed sometime)
* *Interact* is only meant to assist gameplay *interactivity*.

Soo many different devices are possible, many with their own requirements. We want to avoid poluting scene/game space with device specific APIs and make it easy to integrate new devices later down the line. Libraries like SDL and OpenXR take care of some of this work but is too fine-grain in terms of device management and doesn't provide enough structure on their own. Managing the lifespan of devices and dealing with proprietary APIs or potential device bugs and workarounds should **NEVER** wind up in gameplay code. So we will design an API that genericizes most uses from the gameplay side into an easy to understand API. 

## Structure

* Abstract Device Interfaces: Inherits ctModuleBase, Interface into a device API (can specify the max number of instances)
* Abstract Device Instances: Instance of the device which can be asigned to a player. Gets fed player information for the backend to digest.
* Player: A player is a container for diffrent types of device instances and a haptic mixer/axis manager. There is always a global player which owns things like the game service account. Players are defined by gameplay and are *NOT* destroyed when their devices disconnect.
	* Axis: Allows game to read gamepad buttons, Keys, Sticks, Triggers, Relative Mouse, Etc., abstracted to normalized analogue signals/pulses.
	* Haptic Mixer: Defines a handle indexed 3D scene for each controller where vibrations are "mixed" (or queued/ejected), the vibrations either use wave definitions or optional file names for controllers that support hd rumble. A compatible haptic device will be fed the scene state and will be responsible for conveying the resulting haptic response.
	* Cursor: A 2D pointer-onscreen in normalized coordinates, some devices support hiding the cursor/changing appearance.
	* Spacial: Describes a 3D scene of named point(s), can be used for VR/3D controllers/Mocap/Gyro/Etc
	* Info Queries: Gives info such as active devices type, name, resources for displaying buttons/controllers/etc if availible.
	* Settings: Allows players to create/store/load profiles for things like input axis mappings and game specific per-player payloads like accessibility options.
	* Message Queues: A way of sending and recieving event structures to a device. Anything devices need for communication that are not suitable for other categories should be here.
* Device Manager: Devices are not immediately assigned to a player, they need to be assigned, when a device interface deems acceptable a device will be added and await assignment to a player, when a device is disconnected its association with the player is removed. Assigning to the player may fail if the backend deems it unsuitable. Devices in here will list their name/type/icon (if applicable) but do not accept any other meaningful input/output.
* Environment Capabilities: Gives a yes/no about support for diffrent IO features in enum. Filled in by registered backends. Does not necessarily mean that a device is availible/connected, just that it's family of features are supported.

Already the possibilities in player interface are huge, and while interact will likely not have many of these for quite some time to come (possibly ever) it does give a solid direction for expansion while minimally intruding on anything sitting above it.

## About Namings and Bindings

Message names/content should be kept generic and not listed to a single device if possible. This allows the message API to be re-used.

Binding button names should also be kept "generic", but in reality different console controllers have diffrent button layouts which may complicate things. To help aleviate this the following remapping between controllers will be used:

XBox's button layout is assumed and will be remapped for other controllers:

* XBox: 
	* A,B,X,Y = A,B,X,Y
	* Start, Select = Start, Guide
* Playstation: 
	* A,B,X,Y = Cross, Circle, Square, Triangle
	* Start, Select = Options, Trackpad Down (Share on PC)
* Switch:
	* A,B,X,Y = B,A,Y,X
	* Start, Select = +,-
	
Keep in mind that on many platforms buttons can be remapped regardless, so this naming convention is already sitting on top of many layers. The intention here is to keep as smooth as an experience when moving accross all platforms.

## Interact Bindings are not King

The responsibility of implementing axis bindings are per-backend. This is done because some backends already handle bindings at a deeper level, in which case the binding settings will have no affect to avoid creating a mess. If this is the case a capability flag is raised.

## Backends

Backends are code paths can be optionaly compiled per-platform and implement device interfaces and instances. These backends are responsible for implementing the actual funtionality behind the player state and either react or respond. Many of these backends may be short-lived/hacky/proprietary so exclusion from the codebase should not impact anything exept the ability to use a device/service. When conflicting backends are possible (ex: SDL/Steam Input) the user should be able to choose which one takes dominance.