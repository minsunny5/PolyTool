#ifndef POLYGON_H
#define POLYGON_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <iostream>

using namespace std;

class Polygon
{
public:
	Polygon(vector<float> childvertices)
		:vertices(childvertices)
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), &vertices[0], GL_STATIC_DRAW);

		//stride 구하기
		int stride = 0;
		for (auto elem : attributes)
		{
			stride += elem;
		}
		//오프셋 구하면서 attribute 포인터 세팅해주기
		int offset = 0;
		for (int i = 0; i < attributes.size(); i++)
		{
			glVertexAttribPointer(i, attributes[i], GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(offset * sizeof(float)));
			glEnableVertexAttribArray(i);
			offset += attributes[i];
		}
	}

	Polygon(const Polygon&)
	{}

	virtual ~Polygon()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}

	virtual void draw(Shader& shader, bool globalMode) = 0;

	void setPosition(glm::vec3 mousePos, bool globalMode)
	{
		//mousePos로 translate 시키기
		if (globalMode)
		{
			glm::mat4 translation = glm::translate(glm::mat4(1.0f), mousePos);
			trans = translation * trans;
		}
		else
		{
			trans = glm::translate(trans, mousePos);
		}
	}

	void setScale(float xoffset, float yoffset, bool globalMode)
	{
		float scaleSpeed = 0.01f;
		if (globalMode)
		{
			glm::mat4 scalingTrans = glm::scale(glm::mat4(1.0f), glm::vec3(1 + xoffset * scaleSpeed, 1 + yoffset * scaleSpeed, 1.0f));
			trans = scalingTrans * trans;
		}
		else
		{
			trans = glm::scale(trans, glm::vec3(1 + xoffset * scaleSpeed, 1 + yoffset * scaleSpeed, 1.0f));
		}
	}

	void setRotation(float xoffset, float yoffset, bool globalMode)
	{
		float angle = 0.0f;
		if (xoffset < 0 || yoffset > 0)
		{
			angle += 3;
		}
		if (xoffset > 0 || yoffset < 0)
		{
			angle -= 3;
		}
		if (globalMode)
		{
			trans = rotate(angle, trans);
		}
		else
		{
			float oldPosX = trans[3][0];
			float oldPosY = trans[3][1];
			trans[3][0] = 0;
			trans[3][1] = 0;
			trans = rotate(angle, trans);//global rotation at the origin
			trans[3][0] = oldPosX;
			trans[3][1] = oldPosY;
		}
	}

protected:
	unsigned int VBO = 0; 
	unsigned int VAO = 0;
	glm::mat4 trans = glm::mat4(1.0f);
	vector<int> attributes = { 3 }; //추후 vertex attribute가 추가될 수도 있으니까 유동적으로 설정할 수 있게 했다.
	vector<float> vertices;

	glm::mat4 rotate(float angle, glm::mat4 originalT)
	{
		glm::mat4 rotation = glm::mat4(1.0f);
		float rad = glm::radians(angle);
		rotation[0][0] = cos(rad);
		rotation[1][0] = -sin(rad);
		rotation[2][0] = 0;
		rotation[3][0] = 0;
		rotation[0][1] = sin(rad);
		rotation[1][1] = cos(rad);
		rotation[2][1] = 0;
		rotation[3][1] = 0;
		rotation[0][2] = 0;
		rotation[1][2] = 0;
		rotation[2][2] = 1;
		rotation[3][2] = 0;
		rotation[0][3] = 0;
		rotation[1][3] = 0;
		rotation[2][3] = 0;
		rotation[3][3] = 1;

		return rotation * originalT;
	}

	glm::mat4 translate(glm::vec3 pos, glm::mat4 originalT)
	{
		glm::mat4 translation = glm::mat4(1.0f);
		translation[3][0] = pos.x;
		translation[3][1] = pos.y;
		translation[3][2] = 0;

		return translation * originalT;
	}

	glm::mat4 scale(glm::vec3 offset, glm::mat4 originalT)
	{
		glm::mat4 scaling = glm::mat4(1.0f);
		scaling[0][0] = offset.x;
		scaling[1][1] = offset.y;
		scaling[2][2] = 1;

		return scaling * originalT;
	}
};

class Triangle : public Polygon 
{
public:
	Triangle(vector<float> v)
		:Polygon(v)
	{}

	virtual void draw(Shader& shader, bool globalMode)
	{
		shader.use();
		shader.setMat4("transform", trans);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
};

class Rectangle : public Polygon
{
public:
	Rectangle(vector<float> v)
		:Polygon(v)
	{}

	virtual void draw(Shader& shader, bool globalMode)
	{
		shader.use();
		shader.setMat4("transform", trans);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}
};

class Ellipse : public Polygon
{
public:
	Ellipse(vector<float> v)
		:Polygon(v)
	{}

	virtual void draw(Shader& shader, bool globalMode)
	{
		shader.use();
		shader.setMat4("transform", trans);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, numOfpoints+1);
	}

	static vector<float> makeEllipse(float radiusH = 0.1f, float radiusV = 0.05f, int numOfpoints = 360)//horizontal radius, vertical radius
	{
		vector<float> defaultVertices;
		//타원의 중심점을 먼저 찍어준다.
		defaultVertices.push_back(0.0f);
		defaultVertices.push_back(0.0f);
		defaultVertices.push_back(0.0f);

		float t = 0.0f;//angle t

		//0도일 때의 점은 마지막에 한번 더 찍어주기 위해 저장해놓는다.
		vector<float> firstVertices;
		firstVertices.push_back(cos(glm::radians(t)) * radiusH);
		firstVertices.push_back(sin(glm::radians(t)) * radiusV);
		firstVertices.push_back(0.0f);

		for (int i = 3; i < numOfpoints * 3; i += 3)
		{
			defaultVertices.push_back(cos(glm::radians(t)) * radiusH);
			defaultVertices.push_back(sin(glm::radians(t)) * radiusV);
			defaultVertices.push_back(0.0f);
			t += 1.0f;
		}
		//마지막에 0도 일 때의 점을 한번 더 찍어준다. t에 0을 넣든 360를 넣든 같아야 되는데 float 계산이라 미세한 오차가 생겨서 이렇게 한다.
		defaultVertices.push_back(firstVertices[0]);
		defaultVertices.push_back(firstVertices[1]);
		defaultVertices.push_back(firstVertices[2]);
		return defaultVertices;
	}

private:
	enum { numOfpoints = 360 };
	
};



#endif
