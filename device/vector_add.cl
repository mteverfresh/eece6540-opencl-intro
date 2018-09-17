/*
File name: hello-world.cl
Author: Zach Sherer
---------------------------------
Adds two vectors together and stores the result in a second vector.
*/

__kernel void vector_add
(
	__global unsigned restrict* a,
	__global unsigned restrict* b,
	__global unsigned restrict* out
)
{
	int gid = get_global_id(0);
	out[gid] = a[gid] + b[gid];
}
