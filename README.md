# CitrusToolbox
 Small Game Engine
 
 ![CitrusToolboxArchitecture](/docs/CitrusToolboxArchitecture.png)

 Project is a work-in-progress, feel free to take a look around.
 
## Official Supported Platforms
  * Windows 64bit
 
## Building:
  * Prerequisites:
     *  CMake 3.10x
     *  C++ 14 Compiler
     *  Git + LFS (with command line access)
     *  Python 3
  * Clone the repository/submodules to your machine in a directory with sufficient priveleges.
     * `git clone --recurse-submodules https://github.com/KenzieMac130/CitrusToolbox/`
  * Configure and Generate CMake 
      * Make sure the target architecture is x64
  * Compile the Generated Project
 
## Areas of interest:
  * [Code Style](/docs/CodeStyleGuide.md)
  * [Rendering Architecture Presentation](/docs/MovingToGpuDrivenRendering.odp)
  * [Vulkan Backend](/engine/renderer/vulkan/VkBackend.cpp)
  * ["Interact" Design](/docs/InteractNotes.md)
