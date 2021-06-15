## Scene Coordinate Space

* X+: Right
* Y+: Up
* Z+: Forward
* 1 unit = 1 meter
* Positive rotation = counterclockwise

https://github.com/KhronosGroup/glTF/blob/master/specification/2.0/README.md#coordinate-system-and-units

## NDC Space

* Y+: Down
* X+: Right
* Z+: Towards viewer (see reverse depth)

https://matthewwellings.com/blog/the-new-vulkan-coordinate-system/

## Quaternions

* Preferred way of storing rotations
* Specified in (x,y,z,w)

## Radians vs Degrees

* Radians are *ALWAYS* used internally to represent angles.
* Degrees are only used to provide intuitive user-input and converted directly from/to radians.

## Transform Storage

* Translation (vector)
* Rotation (quaternion)
* Scale (vector)

## Matrices

* Matrices should **NOT** be used for storage of transforms, decomposing a matrix into components is obscenely lossy. Use matrices to describe operations that can be applied on top of vectors.
* Do **NOT** invert a matrix! This is a lossy operation. Compute the inverse of a transform matrix directly by reversing transform inputs and reversing the order
* Do **NOT** chain transformations of vectors by matrices. Take the most direct approach from the input vector to the output through a matrix. Ex: instead of multiplying a position by a world matrix, a view matrix, and a projection matrix: Multiply those matrices together and apply it to the position without intermediate vectors.

## Reverse Depth Buffering

* Depth goes from 1.0 (closest) to 0.0 (farthest)
https://dev.theomader.com/depth-precision/ 

## Camera Convention

* Camera: Specifies a device that is viewer into the world
* View: Specifies a lens into the world (multiple may exist for shadow maps/vr)

A camera matrix may encompass all the views' frustrums to be used for culling/froxels in vr, while a view matrix specifies for one eye/point of view

## Perspective vs Ortho

* Orthographic perspective will never be used outside of sun shadow maps. Any further updates will be posted here.

## Variable Naming

* Matrices:
	* Bone to Local Matrix: boneMatrix
	* Local to World Matrix: wolrdMatrix
	* Camera Matrix: cameraMatrix
	* View Matrix: viewMatrix
	* Projection Matrix: projMatrix
* Vectors:
	* Direction: xxxDir (or just) direction
	* Position/Implied Origin: xxxPos (or just) position
	* Scale: xxxScale (or just) scale
* Quaternions:
	* Rotation: xxxRot (or just) rotation

## More References

https://www.humus.name/Articles/Persson_CreatingVastGameWorlds.pdf 