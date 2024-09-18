#include <GLFW\glfw3.h>
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>

using namespace std;

const float DEG2RAD = 3.14159 / 180;

void processInput(GLFWwindow* window);

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE, PADDLE};
enum ONOFF { ON, OFF };


class Brick
{
public:
	float red, green, blue;
	float x, y, width;
	int life;
	BRICKTYPE brick_type;
	ONOFF onoff;



	Brick(BRICKTYPE bt, float xx, float yy, float ww, float rr, float gg, float bb)
	{
		brick_type = bt; x = xx; y = yy, width = ww; red = rr, green = gg, blue = bb;
		onoff = ON;
		life = 5;
	};

	void drawBrick()
	{
		if (onoff == ON)
		{
			double halfside = width / 2;

			glColor3d(red, green, blue);
			glBegin(GL_POLYGON);

			glVertex2d(x + halfside, y + halfside);
			glVertex2d(x + halfside, y - halfside);
			glVertex2d(x - halfside, y - halfside);
			glVertex2d(x - halfside, y + halfside);

			glEnd();
		}
	}
};

Brick paddle(PADDLE, -0.0, -1.0, 0.2, 0.58, 0.58, 0.58);


class Circle
{
public:
	float red, green, blue;
	float radius;
	float x;
	float y;
	float speed = 0.03;
	int direction; // 1=up 2=right 3=down 4=left 5 = up right   6 = up left  7 = down right  8= down left
	ONOFF onoff;

	Circle(double xx, double yy, double rr, int dir, float rad, float r, float g, float b)
	{
		x = xx;
		y = yy;
		radius = rr;
		red = r;
		green = g;
		blue = b;
		radius = rad;
		direction = dir;
		onoff = ON;
	}

	void CheckCollision(Brick* brk)
	{
		if (brk->brick_type == REFLECTIVE && onoff == ON)
		{
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width))
			{
				direction = GetRandomDirection();
				x = x + 0.03;
				y = y + 0.04;
			}
		}
		else if (brk->brick_type == DESTRUCTABLE && brk->onoff == ON && brk->life > 0)
		{
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width))
			{
				brk->life--;
				if (brk->life <= 0)	// brick is off when life is 0
				{
					brk->onoff = OFF;


				}


				brk->blue = 1;	// brick changes color to white after being hit


				direction = GetRandomDirection();

				x = x + 0.03;
				y = y + 0.04;
			}
		}
		// Paddle that reflects balls and can move
		else if (brk->brick_type == PADDLE && onoff == ON)
		{
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width))
			{
				direction = GetRandomDirection();

				x = x + 0.06;
				y = y + 0.04;

			}
		}

	}

	int GetRandomDirection()
	{
		return (rand() % 8) + 1;
	}

	void MoveOneStep()
	{
		if (direction == 1 || direction == 5 || direction == 6)  // up
		{
			if (y > -1 + radius)
			{
				y -= speed;
			}
			else
			{
				direction = GetRandomDirection();
			}
		}

		if (direction == 2 || direction == 5 || direction == 7)  // right
		{
			if (x < 1 - radius)
			{
				x += speed;
			}
			else
			{
				direction = GetRandomDirection();
			}
		}

		if (direction == 3 || direction == 7 || direction == 8)  // down
		{
			if (y < 1 - radius) {
				y += speed;
			}
			else
			{
				direction = GetRandomDirection();
			}
		}

		if (direction == 4 || direction == 6 || direction == 8)  // left
		{
			if (x > -1 + radius) {
				x -= speed;
			}
			else
			{
				direction = GetRandomDirection();
			}
		}
	}

	void DrawCircle()
	{
		glColor3f(red, green, blue);
		glBegin(GL_POLYGON);
		for (int i = 0; i < 360; i++) {
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
		}
		glEnd();
	}
};


vector<Circle> world;


int main(void) {
	srand(time(NULL));

	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(480, 480, "Random World of Circles", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// eyes
	Brick brick(REFLECTIVE, 0.4, 0.4, 0.2, 0.0f, 0.68f, 0.93f);
	Brick brick2(REFLECTIVE, -0.4, 0.4, 0.2, 0.0f, 0.68f, 0.93f);

	// mouth
	Brick brick3(DESTRUCTABLE, -0.1, -0.3, 0.1, 1, 1, 0);
	Brick brick4(DESTRUCTABLE, 0, -0.3, 0.1, 1, 1, 0);
	Brick brick5(DESTRUCTABLE, 0.1, -0.3, 0.1, 1, 1, 0);
	Brick brick6(DESTRUCTABLE, -0.2, -0.2, 0.1, 1, 1, 0);
	Brick brick7(DESTRUCTABLE, 0.2, -0.2, 0.1, 1, 1, 0);
	Brick brick8(DESTRUCTABLE, 0.3, -0.1, 0.1, 1, 1, 0);
	Brick brick9(DESTRUCTABLE, -0.3, -0.1, 0.1, 1, 1, 0);


	while (!glfwWindowShouldClose(window)) {
		//Setup View
		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window);

		//Movement
		for (int i = 0; i < world.size(); i++)
		{
			world[i].CheckCollision(&paddle);
			world[i].CheckCollision(&brick);
			world[i].CheckCollision(&brick2);
			world[i].CheckCollision(&brick3);
			world[i].CheckCollision(&brick4);
			world[i].CheckCollision(&brick5);
			world[i].CheckCollision(&brick6);
			world[i].CheckCollision(&brick7);
			world[i].CheckCollision(&brick8);
			world[i].CheckCollision(&brick9);
			world[i].MoveOneStep();
			world[i].DrawCircle();

			// when the circles collide
			for (int j = 0; j < world.size(); j++)
			{

				Circle* cir1 = &world[i];
				Circle* cir2 = &world[j];

				if (cir1 != cir2)
				{

					// circles are within the same radius
					if (((world[i].x > world[j].x - world[j].radius && world[i].x <= world[j].x + world[j].radius)
						&& (world[i].y > world[j].y - world[j].radius && world[i].y <= world[j].y + world[j].radius)))
					{

						// circles dissapear after being hit
						world[i].radius = 0.0f;
						world[j].radius = 0.0f;

					}
				}
			}
		}

		paddle.drawBrick();
		brick.drawBrick();
		brick2.drawBrick();
		brick3.drawBrick();
		brick4.drawBrick();
		brick5.drawBrick();
		brick6.drawBrick();
		brick7.drawBrick();
		brick8.drawBrick();
		brick9.drawBrick();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate;
	exit(EXIT_SUCCESS);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		double r, g, b;
		r = rand() / 10000;
		g = rand() / 10000;
		b = rand() / 10000;
		Circle B(0, 0, 02, 1, 0.05, r, g, b);
		B.direction = B.GetRandomDirection();
		world.push_back(B);

	}

	// use arrow keys to move paddle left and right

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS && paddle.x > -1.0f)
		paddle.x -= 0.05;

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && paddle.x < 1.0f)
		paddle.x += 0.05;
}