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
	//window context를 현재스레드의 main context로 만들라고 한다.
	glfwMakeContextCurrent(window);
	//유저가 창크기를 바꿀 때마다 이 콜백함수가 불리게 하기 위해 등록
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	//유저가 커서를 움직일 때마다 이 콜백함수가 불리도록 등록
	glfwSetCursorPosCallback(window, mouse_callback);
	//유저가 G를 누를 때마다 이 콜백함수가 불리도록 등록
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

	//Render Loop (루프 한번 = 하나의 프레임)
	while (!glfwWindowShouldClose(window))
	{
		
		//delta time calculation
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//keyboard input check
		processInput(window);

		//Rendering commands..
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);//버퍼 리셋할 때는 이 컬러로 바꾼다.(state-setting function)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);//depth buffer를 쓸거니까 depth버퍼도 리셋해준다.

		//Render Polygons Objs
		for (auto elem : polygons)
		{

			elem->draw(objShader, globalMode);
		}
		
	
		glfwSwapBuffers(window);//백버퍼를 모니터에 출력
		glfwPollEvents();//트리거된 이벤트(키보드 인풋이나 마우스 인풋 등)가 있는지 확인
	}

	//동적할당한 모든 폴리곤 해제
	for (auto elem : polygons)
	{
		delete elem;
	}
	//glfw 리소스 메모리 해제
	glfwTerminate();


	return 0;
}

//유저가 창크기를 바꿀 때마다 불리는 콜백함수
void framebuffer_size_callback(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, w, h);
}

//유저의 마우스 위치를 받아와서 화면 좌표계로 바꿔주는 콜백함수
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	//window position(0~SCR_WIDTH,0~SCR_HEIGHT)
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);
	
	//screen position으로 바꿔주기
	ypos_s = SCR_HEIGHT - ypos -1;

	ypos_s = (ypos_s / (SCR_HEIGHT - 1) * 2.0f - 1.0f) * SCR_HEIGHT / SCR_WIDTH;

	xpos_s = (xpos / (SCR_WIDTH - 1) * 2.0f - 1.0f) * SCR_WIDTH / SCR_HEIGHT;

	//ctrl을 누른상태로 드래그 했을 때
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && scalingMode == true)
	{
		if (firstScale)//첫번째 click 기준 초기화
		{
			oldX = xpos;
			oldY = ypos;
			firstScale = false;
		}

		float xoffset = xpos - oldX;
		float yoffset = oldY - ypos; // 마우스가 가리키는 y 좌표가 아래에서 위로 올라갈 때 yoffset이 증가되어야 하는데 원래 윈도우의 y좌표는 아래에서 위로 갈때 감소하기 때문에 reverse해줌.

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
	
	//드래그 했을 때
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && rotationMode == true)
	{
		if (firstRotate)//첫번째 click 기준 초기화
		{
			oldX = xpos;
			oldY = ypos;
			firstRotate = false;
		}

		float xoffset = xpos - oldX;
		float yoffset = oldY - ypos; // 마우스가 가리키는 y 좌표가 아래에서 위로 올라갈 때 yoffset이 증가되어야 하는데 원래 윈도우의 y좌표는 아래에서 위로 갈때 감소하기 때문에 reverse해줌.

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

	//드래그를 뗐을 때
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		firstScale = true;
		firstRotate = true;
	}
}

//글로벌 모드/로컬 모드 변환 토글 인식하는 콜백함수
void G_toggle(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_G && action == GLFW_PRESS)
	{
		if (globalMode == false)//local 모드였다면
		{
			globalMode = true;//global 모드로 전환
			shaderGlobal->use();
			shaderGlobal->setBool("globalmode", globalMode);
			cout << "Local -> Global" << endl;
			return;
		}
		if (globalMode == true)//global 모드였다면
		{
			globalMode = false;//local 모드로 전환
			shaderGlobal->use();
			shaderGlobal->setBool("globalmode", globalMode);
			cout << "Global -> Local" << endl;
			return;
		}
	}
}

void processInput(GLFWwindow* window)
{
	/*Polygon Drawing 관련 키들*/
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)//esc키가 눌렸다면
	{
		//polygon drawing mode를 취소하기
		renderPolygon = 0;
		drawingMode = false;
		//rotation mode 켜기
		rotationMode = true;
	}

	//drawing 모드가 활성화 된 상태에서 왼쪽 마우스 버튼을 클릭했다면
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

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)//1번 키가 눌렸다면
	{
		drawingMode = true;
		renderPolygon = TRIANGLE;
		rotationMode = false;
	}
	
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)//2번 키가 눌렸다면
	{
		drawingMode = true;
		renderPolygon = RECTANGLE;
		rotationMode = false;
	}

	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)//3번 키가 눌렸다면
	{
		drawingMode = true;
		renderPolygon = ELLIPSE;
		rotationMode = false;
	}

	/* Scaling 관련 키들 */
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		scalingMode = true;
		//polygon drawing mode 끄기
		renderPolygon = 0;
		drawingMode = false;
		//rotation mode 끄기
		rotationMode = false;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE)
	{
		scalingMode = false;
		firstScale = true;
		//rotation mode 켜기
		rotationMode = true;
	}

	/* Translation 관련 키들*/
	float translateSpeed = 0.001f;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)//up키가 눌렸다면
	{
		translation(translateSpeed, GLFW_KEY_UP);
	}

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)//down키가 눌렸다면
	{
		translation(-translateSpeed, GLFW_KEY_DOWN);
	}

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)//오른쪽 방향키가 눌렸다면
	{
		translation(translateSpeed, GLFW_KEY_RIGHT);
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)//왼쪽 방향키가 눌렸다면
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

