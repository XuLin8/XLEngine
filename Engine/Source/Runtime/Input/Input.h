#pragma once 

#include <glm/glm.hpp>
#include "Runtime/Input/KeyCodes.h"
#include "Runtime/Input/MouseCodes.h"

namespace XLEngine
{
	//������ṩ����Ľӿڣ����ڴ��������豸�������¼�����Ӧ�ó����л�ȡ����״̬�����λ�õ���Ϣ
	//��������봦���߼���Ҫ����������ʵ��
	class XLENGINE_API Input
	{
	public:
		static bool IsKeyPressed(KeyCode key);

		static bool IsMouseButtonPressed(MouseCode button);
		static glm::vec2 GetMousePosition();
		[[nodiscard]] static float GetMouseX();
		[[nodiscard]] static float GetMouseY();
	};
}