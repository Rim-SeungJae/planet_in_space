<img src="https://img.shields.io/badge/C-A8B9CC?style=flat&logo=C&logoColor=white"/> <img src="https://img.shields.io/badge/C++-00599C?style=flat&logo=C++&logoColor=white"/> <img src="https://img.shields.io/badge/OpenGL-5586A4?style=flat&logo=OpenGL&logoColor=white"/>
# planet_in_space
This repository is about the second assignment of SKKU's 2021 Introduction to Computer Graphics

# Algorithms and data structures
## Sphere generation
When the program initiates 1 sphere is created with radius of 1, and
center of (0,0,0). Vertices of sphere are created by subdividing the sphere
both in the longitude and latitude. There are 72edges in longitude and
36 edges in latitude. Then, those vertices are connected through a for
loop to form a set of triangles.
Figure 1 counter-clockwise connection
## Sphere rotation
Sphere begins rotating right after it is generated. Every time update()
function is called, it calls glfwGetTIme() function and saves its value.
update() function compares current glfwGetTime() and previously saved
glfwGetTime() value to calculate the time interval between the current
and previous frame. According to this time interval, the sphere is rotated
per each frame. Sphere rotation can be stopped/resumed by pressing ‘R’.
## Sphere color change
Pressing ‘D’ key cyclically toggles among (tc.xy,0,1),(tc.xxx,1), and (tc.yyy,1).
This is implemented by switch statement in fragment shader.
# drag-to-rotate
When the automatic rotation of sphere is stopped by pressing ‘R’ key, sphere
can be rotated by dragging the screen of the program. Drag up and down
to rotate the sphere up and down, and drag from side to side to rotate the
sphere side to side. Press the "R" key again to start rotating the sphere
automatically again.
# Discussions
When implementing the function drag-to-rotate, it was implemented
by applying rotation based on the y-axis first and then rotation based
on the z-axis. This resulted in a slightly unnatural rotation through
the drag (if the sphere is rotated 180 degrees to the left and then try
to rotate the sphere upwards, it will be rotated downward). To make
this function more intuitively, finding a new axis that can rotate the
sphere at once is needed.
