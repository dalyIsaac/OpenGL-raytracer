/*========================================================================
* COSC 363  Computer Graphics (2018)
* Ray tracer 
* See Lab08.pdf for details.
*=========================================================================
*/
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "SceneObject.h"
#include "Ray.h"
#include "Plane.h"
#include <GL/glut.h>
using namespace std;

// width of the image plane in world units
const float WIDTH = 20.0;  

// height of the image place in world unites
const float HEIGHT = 20.0;

// the distance of the image plane from the camera/origin
const float EDIST = 40.0;

// the number of cells along x and y directions
const int NUMDIV = 500;

// the number of levels of recursion
const int MAX_STEPS = 5;

// boundary values of the image plane
const float XMIN = -WIDTH * 0.5;
const float XMAX =  WIDTH * 0.5;
const float YMIN = -HEIGHT * 0.5;
const float YMAX =  HEIGHT * 0.5;

vector<SceneObject*> sceneObjects;  //A global list containing pointers to objects in the scene


//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
// If `xindex` is `-1`, then the background color is returned. Otherwise, it returns the
// object's color
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{
	glm::vec3 backgroundCol(0);
	glm::vec3 light(10, 40, -3);
	glm::vec3 ambientCol(0.2);   //Ambient color of light

    ray.closestPt(sceneObjects);		//Compute the closest point of intersetion of objects with the ray

	// If there is no intersection return background colour
    if(ray.xindex == -1) { 
		return backgroundCol;
	}      

    glm::vec3 materialCol = sceneObjects[ray.xindex]->getColor(); //else return object's colour

	// normal vector on the sphere at the point of intersection
	glm::vec3 normalVector = sceneObjects[ray.xindex]->normal(ray.xpt);

	// vector from the point of intersection towards the light source
	glm::vec3 lightVector = light - ray.xpt;
	lightVector = glm::normalize(lightVector);

	float lDotn = glm::dot(lightVector, normalVector);

	// Specular reflections
	
	glm::vec3 viewVector = -ray.dir;
	
	// first param: incident light's direction (unit vector from light source to the point of intersection)
	glm::vec3 reflVector = glm::reflect(-lightVector, normalVector);
	
	float rDotV = glm::dot(reflVector, normalVector);
	float specularTerm = rDotV < 0.0 ? 0.0 : pow(rDotV, 20.0);

	// Shadows
	Ray shadow(ray.xpt, lightVector);
	shadow.closestPt(sceneObjects);
	float lightDist = glm::length(light);

	glm::vec3 colorSum(0);

	if (lDotn <= 0 || (shadow.xindex > -1 && shadow.xdist < lightDist)) {
		colorSum = ambientCol * materialCol;
	} else {
		colorSum = ambientCol * materialCol + lDotn * materialCol + specularTerm;
	}

	if (ray.xindex == 0 && step < MAX_STEPS) {
		// the following does not need to be normalized as it will have a unit
		// length, since both the incident rays direction and the normal vector
		// are unit vectors
		glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVector);

		// Defines the reflected ray using its source (the point of
		// intersection  on the object), and the direction
		Ray reflectedRay(ray.xpt, reflectedDir);
		
		// Recursive
		glm::vec3 reflectedCol = trace(reflectedRay, step + 1);

		// Coefficient of reflection is specified as 0.8
		colorSum = colorSum + (0.8f * reflectedCol);
	}

	return colorSum;
}

//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
void display()
{
	float xp, yp;  //grid point
	float cellX = (XMAX-XMIN)/NUMDIV;  //cell width
	float cellY = (YMAX-YMIN)/NUMDIV;  //cell height

	// A ray is generated from the origin through the center of each cell
	glm::vec3 eye(0., 0., 0.);  //The eye position (source of primary rays) is the origin

	glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	// draws each cell as a quad
	glBegin(GL_QUADS);  //Each cell is a quad.

	for(int i = 0; i < NUMDIV; i++)  	//For each grid point xp, yp
	{
		xp = XMIN + i*cellX;
		for(int j = 0; j < NUMDIV; j++)
		{
			yp = YMIN + j*cellY;

		    glm::vec3 dir(xp+0.5*cellX, yp+0.5*cellY, -EDIST);	//direction of the primary ray

		    Ray ray = Ray(eye, dir);		//Create a ray originating from the camera in the direction 'dir'
			ray.normalize();				//Normalize the direction of the ray to a unit vector
		    glm::vec3 col = trace (ray, 1); //Trace the primary ray and get the colour value

			glColor3f(col.r, col.g, col.b);
			glVertex2f(xp, yp);				//Draw each cell with its color value
			glVertex2f(xp+cellX, yp);
			glVertex2f(xp+cellX, yp+cellY);
			glVertex2f(xp, yp+cellY);
        }
    }

    glEnd();
    glFlush();
}


//---This function initializes the scene ------------------------------------------- 
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL orthographc projection matrix for drawing the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize()
{
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(XMIN, XMAX, YMIN, YMAX);
    glClearColor(0, 0, 0, 1);

	//-- Create a pointer to a sphere object
	Sphere *sphere1 = new Sphere(glm::vec3(-5.0, -5.0, -90.0), 15.0, glm::vec3(0, 0, 1));

	//--Add the above to the list of scene objects.
	sceneObjects.push_back(sphere1); 

	Sphere *sphere2 = new Sphere(glm::vec3(10.0, 5.0, -70.0), 4.0, glm::vec3(1, 1, 0));
	sceneObjects.push_back(sphere2);

	Sphere *sphere3 = new Sphere(glm::vec3(7.0, -17.0, -78.0), 2.0, glm::vec3(0, 1, 0));
	sceneObjects.push_back(sphere3);

	Plane *plane = new Plane(glm::vec3(-20.0, -20, -40),     // Point A
								glm::vec3(20.0, -20, -40),   // Point B
								glm::vec3(20.0, -20, -200),  // Point C
								glm::vec3(-20.0, -20, -200), // Point D
								glm::vec3(0.5, 0.5, 0)); 	 // Color
	sceneObjects.push_back(plane);
}



int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("Raytracer");

    glutDisplayFunc(display);
    initialize();

    glutMainLoop();
    return 0;
}