#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>

#include "shader.h"
#include "polygon.h"
using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int w, int h);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void G_toggle(GLFWwindow* window, int key, int scancode, int action, int mods);
void translation(float translateSpeed, int key);
//void mouse_click_callback(GLFWwindow* window, int button, int action, int mods);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

// ---------------------------------
// Global
// -----------------------------------

//Timing
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

//Polygon drawing
vector<Polygon*> polygons;//render list
int renderPolygon = 0; //which polygon to draw
bool drawingMode = false;
bool scalingMode = false;
bool rotationMode = false;
bool globalMode = false;

//screen mouse position
float xpos_s, ypos_s = 0;
float oldX, oldY = 0;
bool firstScale = true;
bool firstRotate = true;
//click pos
float lastX[3] = {};
float lastY[3] = {};

//enum
enum {
	TRIANGLE = 1,
	RECTANGLE = 2,
	ELLIPSE = 3
};

//shader
Shader* shaderGlobal;

int main()
{
	//Initialize glfw
	glfwInit();

	//configure glfw
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//Create a window object
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return -1;
	}
	//window context�� ���罺������ main context�� ������ �Ѵ�.
	glfwMakeContextCurrent(window);
	//������ âũ�⸦ �ٲ� ������ �� �ݹ��Լ��� �Ҹ��� �ϱ� ���� ���
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	//������ Ŀ���� ������ ������ �� �ݹ��Լ��� �Ҹ����� ���
	glfwSetCursorPosCallback(window, mouse_callback);
	//������ G�� ���� ������ �� �ݹ��Լ��� �Ҹ����� ���
	glfwSetKeyCallback(window, G_toggle);

	//Initialize glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		cout << "Failed to initialize glad" << endl;
		return -1;
	}

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);	
	// ------------------------------------
	//depth testing on
	// ------------------------------------
	//glEnable(GL_DEPTH_TEST);


	// ------------------------------------
	// build and compile our shader program
	// ------------------------------------
	Shader objShader("color.vs", "color.fs");
	shaderGlobal = &objShader;

	//Render Loop (���� �ѹ� = �ϳ��� ������)
	while (!glfwWindowShouldClose(window))
	{
		
		//delta time calculation
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//keyboard input check
		processInput(window);

		//Rendering commands..
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);//���� ������ ���� �� �÷��� �ٲ۴�.(state-setting function)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//depth buffer�� ���Ŵϱ� depth���۵� �������ش�.

		//Render Polygons Objs
		for (auto elem : polygons)
		{

			elem->draw(objShader, globalMode);
		}
		
	
		glfwSwapBuffers(window);//����۸� ����Ϳ� ���
		glfwPollEvents();//Ʈ���ŵ� �̺�Ʈ(Ű���� ��ǲ�̳� ���콺 ��ǲ ��)�� �ִ��� Ȯ��
	}

	//�����Ҵ��� ��� ������ ����
	for (auto elem : polygons)
	{
		delete elem;
	}
	//glfw ���ҽ� �޸� ����
	glfwTerminate();


	return 0;
}

//������ âũ�⸦ �ٲ� ������ �Ҹ��� �ݹ��Լ�
void framebuffer_size_callback(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, w, h);
}

//������ ���콺 ��ġ�� �޾ƿͼ� ȭ�� ��ǥ��� �ٲ��ִ� �ݹ��Լ�
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	//window position(0~SCR_WIDTH,0~SCR_HEIGHT)
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);
	
	//screen position���� �ٲ��ֱ�
	ypos_s = SCR_HEIGHT - ypos -1;

	ypos_s = (ypos_s / (SCR_HEIGHT - 1) * 2.0f - 1.0f) * SCR_HEIGHT / SCR_WIDTH;

	xpos_s = (xpos / (SCR_WIDTH - 1) * 2.0f - 1.0f) * SCR_WIDTH / SCR_HEIGHT;

	//ctrl�� �������·� �巡�� ���� ��
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && scalingMode == true)
	{
		if (firstScale)//ù��° click ���� �ʱ�ȭ
		{
			oldX = xpos;
			oldY = ypos;
			firstScale = false;
		}

		float xoffset = xpos - oldX;
		float yoffset = oldY - ypos; // ���콺�� ����Ű�� y ��ǥ�� �Ʒ����� ���� �ö� �� yoffset�� �����Ǿ�� �ϴµ� ���� �������� y��ǥ�� �Ʒ����� ���� ���� �����ϱ� ������ reverse����.

		oldX = xpos;
		oldY = ypos;

		if (globalMode)
		{
			for (auto elem : polygons)
			{
				elem->setScale(xoffset, yoffset, globalMode);
			}
		}
		else
		{
			if (!polygons.empty())
			{
				polygons.back()->setScale(xoffset, yoffset, globalMode);
			}
		}
	}
	
	//�巡�� ���� ��
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && rotationMode == true)
	{
		if (firstRotate)//ù��° click ���� �ʱ�ȭ
		{
			oldX = xpos;
			oldY = ypos;
			firstRotate = false;
		}

		float xoffset = xpos - oldX;
		float yoffset = oldY - ypos; // ���콺�� ����Ű�� y ��ǥ�� �Ʒ����� ���� �ö� �� yoffset�� �����Ǿ�� �ϴµ� ���� �������� y��ǥ�� �Ʒ����� ���� ���� �����ϱ� ������ reverse����.

		oldX = xpos;
		oldY = ypos;

		if (globalMode)
		{
			for (auto elem : polygons)
			{
				elem->setRotation(xoffset, yoffset, globalMode);
			}
		}
		else
		{
			if (!polygons.empty())
			{
				polygons.back()->setRotation(xoffset, yoffset, globalMode);
			}
		}
	}

	//�巡�׸� ���� ��
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		firstScale = true;
		firstRotate = true;
	}
}

//�۷ι� ���/���� ��� ��ȯ ��� �ν��ϴ� �ݹ��Լ�
void G_toggle(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_G && action == GLFW_PRESS)
	{
		if (globalMode == false)//local ��忴�ٸ�
		{
			globalMode = true;//global ���� ��ȯ
			shaderGlobal->use();
			shaderGlobal->setBool("globalmode", globalMode);
			cout << "Local -> Global" << endl;
			return;
		}
		if (globalMode == true)//global ��忴�ٸ�
		{
			globalMode = false;//local ���� ��ȯ
			shaderGlobal->use();
			shaderGlobal->setBool("globalmode", globalMode);
			cout << "Global -> Local" << endl;
			return;
		}
	}
}

void processInput(GLFWwindow* window)
{
	/*Polygon Drawing ���� Ű��*/
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)//escŰ�� ���ȴٸ�
	{
		//polygon drawing mode�� ����ϱ�
		renderPolygon = 0;
		drawingMode = false;
		//rotation mode �ѱ�
		rotationMode = true;
	}

	//drawing ��尡 Ȱ��ȭ �� ���¿��� ���� ���콺 ��ư�� Ŭ���ߴٸ�
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		if (drawingMode == true && renderPolygon == TRIANGLE)
		{
			if (lastX[TRIANGLE - 1] != xpos_s || lastY[TRIANGLE - 1] != ypos_s)
			{
				vector<float> v = { 0.0f, 0.1f, 0.0f,
								-0.1f, -0.1f, 0.0f,
								0.1f, -0.1f, 0.0f };

				Triangle* tri = new Triangle(v);
				Polygon* poly = tri;
				tri->setPosition(glm::vec3(xpos_s, ypos_s, 0.0f), globalMode);
				polygons.push_back(poly);
				lastX[TRIANGLE - 1] = xpos_s;
				lastY[TRIANGLE - 1] = ypos_s;
			}
		}

		if (drawingMode == true && renderPolygon == RECTANGLE)
		{
			if (lastX[RECTANGLE - 1] != xpos_s || lastY[RECTANGLE - 1] != ypos_s)
			{
				vector<float> v = { -0.1f, 0.1f, 0.0f,
								0.1f, 0.1f, 0.0f,
								0.1f, -0.1f, 0.0f,
								-0.1f, -0.1f, 0.0f };
				Rectangle* rect = new Rectangle(v);
				Polygon* poly = rect;
				rect->setPosition(glm::vec3(xpos_s, ypos_s, 0.0f), globalMode);
				polygons.push_back(poly);
				lastX[RECTANGLE - 1] = xpos_s;
				lastY[RECTANGLE - 1] = ypos_s;
			}
			
		}

		if (drawingMode == true && renderPolygon == ELLIPSE)
		{
			if (lastX[ELLIPSE - 1] != xpos_s || lastY[ELLIPSE - 1] != ypos_s)
			{
				vector<float> v(Ellipse::makeEllipse());
				Ellipse* ellipse = new Ellipse(v);
				Polygon* poly = ellipse;
				ellipse->setPosition(glm::vec3(xpos_s, ypos_s, 0.0f), globalMode);
				polygons.push_back(poly);
				lastX[ELLIPSE - 1] = xpos_s;
				lastY[ELLIPSE - 1] = ypos_s;
			}
		}
	}

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)//1�� Ű�� ���ȴٸ�
	{
		drawingMode = true;
		renderPolygon = TRIANGLE;
		rotationMode = false;
	}
	
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)//2�� Ű�� ���ȴٸ�
	{
		drawingMode = true;
		renderPolygon = RECTANGLE;
		rotationMode = false;
	}

	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)//3�� Ű�� ���ȴٸ�
	{
		drawingMode = true;
		renderPolygon = ELLIPSE;
		rotationMode = false;
	}

	/* Scaling ���� Ű�� */
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		scalingMode = true;
		//polygon drawing mode ����
		renderPolygon = 0;
		drawingMode = false;
		//rotation mode ����
		rotationMode = false;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE)
	{
		scalingMode = false;
		firstScale = true;
		//rotation mode �ѱ�
		rotationMode = true;
	}

	/* Translation ���� Ű��*/
	float translateSpeed = 0.001f;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)//upŰ�� ���ȴٸ�
	{
		translation(translateSpeed, GLFW_KEY_UP);
	}

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)//downŰ�� ���ȴٸ�
	{
		translation(-translateSpeed, GLFW_KEY_DOWN);
	}

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)//������ ����Ű�� ���ȴٸ�
	{
		translation(translateSpeed, GLFW_KEY_RIGHT);
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)//���� ����Ű�� ���ȴٸ�
	{
		translation(-translateSpeed, GLFW_KEY_LEFT);
	}
}

void translation(float translateSpeed, int key)
{
	if (key == GLFW_KEY_UP || key == GLFW_KEY_DOWN)
	{
		if (globalMode)
		{
			for (auto elem : polygons)
			{
				elem->setPosition(glm::vec3(0.0f, translateSpeed, 0.0f), globalMode);
			}
		}
		else
		{
			polygons.back()->setPosition(glm::vec3(0.0f, translateSpeed, 0.0f), globalMode);
		}
	}
	if (key == GLFW_KEY_RIGHT || key == GLFW_KEY_LEFT)
	{
		if (globalMode)
		{
			for (auto elem : polygons)
			{
				elem->setPosition(glm::vec3(translateSpeed, 0.0f, 0.0f), globalMode);
			}
		}
		else
		{
			polygons.back()->setPosition(glm::vec3(translateSpeed, 0.0f, 0.0f), globalMode);
		}
	}
}

