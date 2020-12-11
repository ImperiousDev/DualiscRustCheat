#pragma once
#include "ImGui\imgui.h"
#include "ImGui\imgui_impl_dx11.h"
#include "ImGui\imgui_internal.h"
#include "../Vectors/Vector.hpp"

#include <d3d11.h>
#include <dinput.h>
#include <tchar.h>
#include <Dwmapi.h>
#include <Windows.h>
#include <Tlhelp32.h>
#include <string>
#include <windef.h>
#include <DirectXMath.h>
#include <cmath>
#include <windef.h>
#include <Psapi.h> 
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <mutex>
#include <iostream>
#include <vector>
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "Dwmapi.lib")

class Renderer
{
public:
	void Initialize();
	void BeginScene();
	void DrawScene();
	void EndScene();

	static Renderer* GetInstance();

	ImFont* m_pFont;
	ImFont* m_rowland_26;

	Renderer();
	~Renderer();


	float DrawMyText(ImFont* pFont, PCHAR text, const DirectX::XMFLOAT2& pos, float size, Vector3 RGB, bool center);
	void DrawLine(const DirectX::XMFLOAT2& from, const DirectX::XMFLOAT2& to, Vector3 RGB, float thickness);
	void DrawBox(const DirectX::XMFLOAT2& from, const DirectX::XMFLOAT2& size, Vector3 RGB, float rounding, float thickness);
	void draw_rect(float x, float y, float w, float h, Vector3 color);
	void DrawOutlinedRect(float x, float y, float w, float h, Vector3 color);
	void DrawBoxOutlined(const DirectX::XMFLOAT2& from, const DirectX::XMFLOAT2& size, Vector3 RGB, Vector3 OutlineRGB, float rounding, float thickness);
	void DrawBoxFilled(const DirectX::XMFLOAT2& from, const DirectX::XMFLOAT2& size, DirectX::XMFLOAT4 color, float rounding);
	void draw_rectangle(float x, float y, float x2, float y2, float thickness, bool filled, ImColor color);
	void draw_line(float x, float y, float x1, float y1, float thickness, ImColor color);
	void DrawCircle(const DirectX::XMFLOAT2& from, float radius, DirectX::XMFLOAT4 color, float thickness = 1.f);
	void DrawCircleFilled(const DirectX::XMFLOAT2& from, float radius, DirectX::XMFLOAT4 color);
	void DrawBoxFilled(const DirectX::XMFLOAT2& from, const DirectX::XMFLOAT2& size, Vector3 color, float rounding);
	void DrawHealthbars(float PosX, float PosY, float height, float Value1, Vector3 color);
	void draw_text(Vector3 pos, const char* text, ImColor color, bool centered, bool stroked, ImFont* font, ImColor outline_col = ImColor{ 0.f, 0.f, 0.f, 0.8f });
	void draw_circle(float x, float y, float radius, bool filled, float thickness, ImColor color, int segments);
private:
	static Renderer* m_pInstance;
};