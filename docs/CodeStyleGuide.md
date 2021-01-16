# Code Style Guide

## Files

### Naming
* File names must be in "cammelCase"
* Folder names must be kept as "flatcase"
* Folder names must also be kept ideally close to one word, abbreviation or a compound.
* Files and Folders in the "ThirdParty" must be kept faithful to their original as much as possible.
* Names must generally be kept as short as possible without abbreviations.
* Special characters including underlines and intermediate periods must be avoided.
* File extensions must be under six characters.
* Every time you need to expand the dictionary of internally used words, add it to the thesaurus and keep consistent.
* Public facing names should generally describe usage, not implementation unless the usage assumes a certain implementation. (ex: sorting algorithms, containers)

### Project Structure
* NewEngine/
	* assets/
		* core/
			* .../
		* .../
	* docs/
		* .../
	* game/
		* (GAME_NAME)/
		* .../
	* tools/
		* (TOOL_NAME)/
		* .../
	* engine/
		* modules/
			* core/
			* gfx/
				* vulkan/
			* resource/
			* utilities/
			* input/
			* thirdparty/
		* tests/
			* .../
		* thirdparty/
			* (LIBRARY_NAME)/
			* .../
* build/
	* .../
* libs/
	* (PLATFORM_NAME)_(COMPILER_NAME)/
		* static/
		* shared/
	* .../

## C

### Version

**ISO C99**

### File Extensions
* Header: ".h"
* Implementation: ".c"
	
### Indentation
* Indents must use tabs.
* Indent the contents any multi-line "{}" or "()".

### Line Length
* Keep the line length bellow the length of 80 characters without indents.

### Comments
* Always use old-fashioned C style multi-line comments "/**/".
* Do not leave code commented out.

### Namespaces
* C does not have namespaces so follow this:
* "ne" is the engine wide namespace prefix
* "ne" should be followed by the module name: (ex. "neGfx...")

### Standard Libraries
* Don't re-implement what is in the standard library unless there is a reason.
* #define things that might need to be re-implemented such as malloc.
* Don't use "printf()" prefer debug logging functionalities in "neDebugLog/neDebugWarning/neDebugError"

### Typedefs
* Naming: neTypeName
* Avoid unnecessary typedefs
* Only use typedefs for base types and not structs/enums

### Variables
* AVOID GLOBAL VARIABLES!!!
* Only use abbreviations for commonly understood terminology (ex: PBR vs Physically Based Rendering)
* If you don't want the user to modify a variable use "_" to prefix it

### Functions
* Naming: neFunctionName()
* NO HIDDEN STATES!!! Only modify what is passed to a function! (Even if this is just a module context object)
* You can break up function calls and definitions into multiple lines.

### Pointers
* Pointer example format: int* pVariables;
* Pointer-to-pointer format: int** ppVariables;

### Function Pointers
* Format: void (*fpFuncPtrName)(int)

### Const Correctness
* WILL MY FUNCTION INPUT BE MODIFIED? (Y/N) (to const or not to const)

### Returns
* Generally try to only return type neResult for error checking
* Return value optimization should only be applied in performance critical code (

### Arrays
* Fixed sizes must be #defined in the global defines file

### Macros
* Macros should be in SCREAMING_SNAKE_CASE unless used as a function override

### Including
* Todo: Include folder structure

### Include Guards
* #pragma once

### Enums
* enums should follow this style:
enum myEnum {
	MY_VAL_A = 0,
	MY_VAL_B = 1,
	MY_VAL_COUNT,
	MY_VAL_MAX = UINT32_MAX
};

### Loops
* loop indices should go: "i", "j", "k"
* avoid deep loops (offload to functions)

### Structs
* C Style structs should only be assumed to be C compatible inside ".h" files.

### Memory
* Avoid manual mallocs() in modules and try to use the resource manager as much as possible.
* Avoid allocating small blocks of memory.
* Most resources should have a clear lifecycle (ex: an action, a frame, another level, or until shutdown)
* Comment on when the life of a resource is expected to end around creation.
* The resource system implements a reference counting system for shared use, 

### Threading
* Async kickoff functions should be clearly marked and register with the engine's task manager.
* Avoid low level thread management code outside of the task manager.
* A scratch pad of memory for a parallel process can be acquired via the resource system and implicitly locked/checked for mutex lock by opening the resource.
* Blank resources can be used to check for locks as well.
* Non-threadsafe functions should be avoided but should be clearly marked with a comment.

### Errors
* neResult is used to return the state of if a function succeeded or failed and the most relevant description of the failure

### SIMD Intrinsics
* Don't bother with handwritten SIMD, opt for ISPC if absolutely necessary.
* Avoid small functions with internal SIMD implementations, one of SIMD's main benefits is that there are more registers to not get evicted from.

### Additional Features
* Bit packing is allowed only internally in the implementation.
* Variable length arrays are disabled.

## C++

### Version

**C++11**

### File Extensions
* Header: ".hpp"
* Implementation: ".cpp"

### Encapsulation
* The main applications will always work with a C interface, therefore C++ parts of code must only appear publicly behind a C interface.

### Classes

### Members
* Member functions must only be used to perfom certain actions on or give insight into the state of an object, not act as a direct wrapper between the data and the request.

### Exceptions
* Exceptions will be disabled.

### Templates
* Avoid templates as much as possible outside of containers and math.
* Only use a limited set of template functionality if needed and avoid meta-programming.

### STL/BOOST/Other Features
* **DONT**


## CMake

### Version
CMake 3.10

### Globbing
* Avoid globbing and explicitly include files using "set".

### Indentation
* Indent with tab multi-line arguments or branch contents.

### Line Length
* Keep the line length bellow the length of 80 characters without indents.

### Comments
* Single and multi-line comments must be made using "#" and only be used when necessary.
* Do not leave code commented out.
* Avoid single word comments.
* Paragraph long comments should be used for complex subjects. (ex: at the top of a module's file)
* Short comments should be used to highlight stages in a process. (ex: a for loop)
* Do not contradict the code with your comment, if there is an error in the description fix it.

### Variables
* Variables are in SCREAMING_SNAKE_CASE.
* Only use abbreviations for commonly understood terminology (ex: PBR vs Physically Based Rendering).

### Libraries
* Use static linking for engine modules.
* Use dynamic linking for large third party libraries.
* Use relative paths to find the necessary library.
* Break header only libraries into header and implementation as applicable to avoid possible issues.
* TODO: More hurdles

## GLSL

### Version
* GLSL STD 460
https://www.khronos.org/registry/OpenGL/specs/gl/GLSLangSpec.4.60.pdf
* Vulkan Extension (Builtin)
https://github.com/KhronosGroup/GLSL/blob/master/extensions/khr/GL_KHR_vulkan_glsl.txt
* Ray-Tracing
https://github.com/KhronosGroup/GLSL/blob/master/extensions/ext/GLSL_EXT_ray_tracing.txt

### Includes
* Includes are allowed with the use of "#include "...""
* Includes must be relative to the current path.
* Include guards must be in-place

### File Extensions
* Vertex and fragment extensions to seperate stages are not welcome.
* GLSL Shader: ".glsl"

## Python

### Follow PEP-8
https://www.python.org/dev/peps/pep-0008/