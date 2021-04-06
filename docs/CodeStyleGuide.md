# Code Style Guide

## Files

### Naming
* File names must be in "cammelCase"
* Folder names must be kept as "flatcase"
* Folder names must also be kept ideally close to one word, abbreviation or a compound.
* Files and Folders in the "thirdparty" must be kept faithful to their original as much as possible.
* Names must generally be kept as short as possible without abbreviations.
* Special characters including underlines and intermediate periods must be avoided.
* File extensions must be under six characters.
* Every time you need to expand the dictionary of internally used words, add it to the thesaurus and keep consistent.
* Public facing names should generally describe usage, not implementation unless the usage assumes a certain implementation. (ex: sorting algorithms, containers)

### Project Structure
* CitrusToolbox/
	* assets/
		* core/
			* (...)/
		* game/
			* (GAME_NAME)/
				* (GAME_ASSET_FILES)...
	* build/
		* generated/
			* ispc/
			* reflect/
				* .../
			* shaders/
		* output/
			* assets/
			* (BINARY_BUILDS)...
		* ...
	* docs/
	* engine/
		* core/
		* ispc/
		* renderer/
			* lowlevel/
				* (GFX_BACKEND_NAME)/
			* shaders/
		* utilities/
	* game/
		* (GAME_NAME)/
			* (GAME_SOURCE_FILES)...
	* libs/
		* (PLATFORM_NAME)/
			* FMOD/
			* Installs/
			* Ispc/
			* PhysX/
			* SDL2/
	* tests/
		* .../
	* thirdparty/
		* .../
		* OpenSourceCredits.txt
	* tools/
		* reflect/
		* tracy/
			* (TRACY_REPO)...
				

## C

### Version

**ISO C99**

### Clang Format
* Clang format has been set up with this project

### File Extensions
* Header: ".h"
* Implementation: ".c"
	
### Indentation
* Indents must use 3 spaces.
* Indent the contents any multi-line "{}" or "()".

### Line Length
* Keep the line length bellow the length of 90 characters without indents.

### Comments
* Always use old-fashioned C style multi-line comments "/**/".
* Do not leave code commented out.

### Namespaces
* C does not have namespaces so follow this:
* "ct" is the engine wide namespace prefix
* "ct" should be followed by the module name: (ex. "ctGfx...")

### Standard Libraries
* Don't re-implement what is in the standard library unless there is a reason.
* #define things that might need to be re-implemented such as malloc.
* Don't use "printf()" prefer debug logging functionalities in "ctDebugLog/ctDebugWarning/ctDebugError"
* Avoid functions that have dubious single threading limitations or allocate more memory outside malloc/calloc

### Typedefs
* Naming: neTypeName
* Avoid unnecessary typedefs
* Only use typedefs for base types and not structs/enums

### Variables
* AVOID GLOBAL VARIABLES!!!
* Only use abbreviations for commonly understood terminology (ex: PBR vs Physically Based Rendering)
* If you don't want the user to modify a variable use "_" to prefix it

### Functions
* Naming: ctFunctionName()
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

### Errors
* ctResult is used to return the state of if a function succeeded or failed and the most relevant description of the failure

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

### Classes
* Keep inheritance shallow.
* Naming is prefixed with "ct".
* Public: Common to access and modify
* Protected: Dangerous to modify outside of children
* Private: Dangerous to modify regardless of inheritance

### Member Functions
* Try to answer booleans with a lowecase "is" prefix.
* Member functions must only be used to perfom certain actions on or give insight into the state of an object, not act as a direct wrapper between the data and the request.
* If the function call doesn't modify the internal state of the object it should also be marked const.
* Only use virtual functions if inheritance is intended to be used and the last should be marked as final.

### Reflection Pragmas
* Todo

### References
* Function inputs
	* Will the result be modified?
		* (Yes) Is it optional?
			* (Yes) var*
			* (No) var&
		* (No) Is it a complex (non-POD) type?
			* (Yes) const var&
			* (No) const var
* Returns
	* Can this return fail non-catastrophically (entry not found)?
		* (Yes) Use pointer (*)
		* (No) Is this an operator override?
			* (Yes) Use reference (&)
			* (No) Use pointer (*)

### Exceptions
* Exceptions will be disabled.

### Templates
* Avoid templates as much as possible outside of containers.
* Use C++03 style formatting (code generator reasons)
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
* Keep the line length bellow the length of 90 characters without indents.

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
* Avoid deeply nested CMakeLists.txt trees, try to keep one per-library

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

## ISPC
* Todo

## Python

### Simplicity
* Python should be treated as a small scripting language for offline tools.
* Avoid deeply nested Python scripts with imports.0

### Follow PEP-8
https://www.python.org/dev/peps/pep-0008/