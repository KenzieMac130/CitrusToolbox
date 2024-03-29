# Code Style Guide

## Files

### Naming
* File names must be in "UpperCammelCase"
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
		* .../
		wscript
		DataNicknames.json
	* build/
		* generated/
			* ispc/
			* reflect/
				* .../
			* shaders/
		* output/
			* data/
			* (BINARY_BUILDS)...
		* .../
	* docs/
	* engine/
		* .../
		* utilities/
	* game/
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
		* .../
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

### Language Compatibility
* Always make it C++ compatible, Unless the project is compiled in C or there is a "#ifdef __cplusplus".

### Namespaces
* C does not have namespaces so follow this:
* "ct" is the engine wide namespace prefix
* "ct" can be followed by the module name: (ex. "ctGPU...")

### Standard Libraries
* Don't re-implement what is in the standard library unless there is a reason.
* #define things that might need to be re-implemented such as malloc.
* Don't use "printf()" prefer debug logging functionalities in "ctDebugLog/ctDebugWarning/ctDebugError"
* Avoid functions that have dubious single threading limitations or allocate more memory outside malloc/calloc

### Typedefs
* Naming: ctTypeName
* Avoid unnecessary typedefs
* Only use typedefs for base types and not structs/enums

### Variables
* AVOID GLOBAL VARIABLES!!!
* Only use abbreviations for commonly understood terminology (ex: PBR vs Physically Based Rendering)
* (The following rules have been introduced and will be enforced in a cleanup project)
* Member variables will be prefixed by m_ (ex: m_value or m_pValue...)
* Global variables will be prefixed by g_ (ex: g_value or g_pValue)
* If you don't want the user to modify a variable use "_" to prefix it (ex: _m_value)

### Functions
* Naming: ctFunctionName()
* MINIMIZE HIDDEN STATES!!! Only modify what is passed to a function! (Even if this is just a module context object)
* You can break up function calls and definitions into multiple lines.

### Current Hidden/Global State Exceptions
* Shared Logging
* Shared String Translation
* Tracy Profiling Markup
* ImGUI and Im3D

### Pointers
* Pointer example format: int* pVariables;
* Pointer-to-pointer format: int** ppVariables;

### C Strings
* Use const char* wherever read-only string data needs to be passed.
* Raw string pointers for modifiable strings are discouraged in favor of utilities.
* Always assume data is UTF-8 unless otherwise marked and do not assume 1-byte per character.
* Do not cache C string pointers, copy to another structure to avoid dangling.
* Text that can be presented to the user should be wrapped in a "CT_N*" prefix for translation.

### Function Pointers
* Format: void (*fpFuncPtrName)(int)

### Const Correctness
* WILL MY FUNCTION INPUT BE MODIFIED? (Y/N) (to const or not to const)
* Admittedly this rule is not currently strictly enforced

### Returns
* Generally try to only return type ctResult for error checking
* Return value optimization should only be applied in performance critical code.

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
* Avoid small functions with internal SIMD implementations, one of SIMD's main benefits is that there are more space/registers to not get evicted from.

### Additional Features
* Bit packing is allowed but discouraged.
* Unions should make their use case immediately obvious
* Designated initializers are unfortunately not used for compatibility. :(
* Variable length arrays are disabled.

## C++

### Version

**C++14**

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

### Namespaces
* Namespaces will not be used in the engine in favor of the "ct" prefix.
* Namespaces from 3rd party libraries should not be shortened by "using" unless it has a redundant prefix.
* Namespaces for the scene engine is currently allowed as a compromise

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
* Indent with 3 spaces multi-line arguments or branch contents.

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

## Shaders
* See [ShadingLanguage.md](docs/ShadingLanguage.md)

## ISPC
* Todo

## Python

### Simplicity
* Python should be treated as a small scripting language for offline tools.
* Avoid deeply nested Python scripts with imports.

### Follow PEP-8
https://www.python.org/dev/peps/pep-0008/
