#pragma once
#include "Drawer.h"
#include "ImGui\imgui.h"
#include "ImGui\imgui_internal.h"
#include "../Vectors/Vector.hpp"

Renderer* Renderer::m_pInstance;

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::Initialize()
{
	ImGuiIO& io = ImGui::GetIO();

	m_pFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 12.f);
}

void Renderer::BeginScene()
{
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::Begin("BUFFERNAME", reinterpret_cast<bool*>(true), ImVec2(0, 0), 0.0f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs);

	ImGui::SetWindowPos(ImVec2(0, 0), ImGuiSetCond_Always);
	ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y), ImGuiSetCond_Always);

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	//window->DrawList->AddRectFilled(ImVec2(0, 0), ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y), ImGui::GetColorU32(ImVec4(.1f, .1f, .1f, 0.1f)));


}

void Renderer::DrawScene()
{
	ImGuiIO& io = ImGui::GetIO();
}

void Renderer::EndScene()
{
	
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	window->DrawList->PushClipRectFullScreen();

	ImGui::End();
	ImGui::PopStyleColor();
}

float Renderer::DrawMyText(ImFont* pFont, PCHAR text, const DirectX::XMFLOAT2& pos, float size, Vector3 RGB, bool center)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	float a = 255;
	float r = RGB.x;
	float g = RGB.y;
	float b = RGB.z;

	float y = 0.0f;
	int i = 0;

	ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, text);

	if (center)
	{
		window->DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) + 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(.1f, .1f, .1f, a / 255)), text);
		window->DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) - 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(.1f, .1f, .1f, a / 255)), text);
		window->DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) + 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(.1f, .1f, .1f, a / 255)), text);
		window->DrawList->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) - 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(.1f, .1f, .1f, a / 255)), text);

		window->DrawList->AddText(pFont, size, ImVec2(pos.x - textSize.x / 2.0f, pos.y + textSize.y * i), ImGui::GetColorU32(ImVec4(r / 255, g / 255, b / 255, a / 255)), text);
	}
	else
	{
		window->DrawList->AddText(pFont, size, ImVec2((pos.x) + 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0.f, 0.f, 0.f, a / 255)), text);
		window->DrawList->AddText(pFont, size, ImVec2((pos.x) - 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0.f, 0.f, 0.f, a / 255)), text);
		window->DrawList->AddText(pFont, size, ImVec2((pos.x) + 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0.f, 0.f, 0.f, a / 255)), text);
		window->DrawList->AddText(pFont, size, ImVec2((pos.x) - 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0.f, 0.f, 0.f, a / 255)), text);

		window->DrawList->AddText(pFont, size, ImVec2(pos.x, pos.y + textSize.y * i), ImGui::GetColorU32(ImVec4(r / 255, g / 255, b / 255, a / 255)), text);
	}

	y = pos.y + textSize.y * (i + 1);
	i++;

	return y;
}

void Renderer::DrawLine(const DirectX::XMFLOAT2& from, const DirectX::XMFLOAT2& to, Vector3 RGB, float thickness)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	window->DrawList->AddLine(ImVec2(from.x, from.y), ImVec2(to.x, to.y), ImGui::GetColorU32(ImVec4(RGB.x / 255, RGB.y / 255, RGB.z / 255, 255 / 255)), thickness);
}

void Renderer::DrawBox(const DirectX::XMFLOAT2& from, const DirectX::XMFLOAT2& size, Vector3 RGB, float rounding, float thickness)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	//ImGuiContext* window = ImGui::GetCurrentContext();
	//window->OverlayDrawList.AddCircleFilled(ImVec2(400, 400), 30, 30, 12);
	window->DrawList->AddRect(ImVec2(from.x, from.y), ImVec2(size.x, size.y), ImGui::GetColorU32(ImVec4(RGB.x / 255, RGB.y / 255, RGB.z / 255, 255 / 255)), rounding, 15, thickness);
}

void Renderer::draw_rect(float x, float y, float w, float h, Vector3 color)
{
	DrawLine(DirectX::XMFLOAT2(x, y), DirectX::XMFLOAT2(w, h), color, 0);
	DrawLine(DirectX::XMFLOAT2(x, y), DirectX::XMFLOAT2(w ,h), color, 0);
	DrawLine(DirectX::XMFLOAT2(x, y), DirectX::XMFLOAT2(w, h) , color, 0);
	DrawLine(DirectX::XMFLOAT2(x, y), DirectX::XMFLOAT2(w, h), color, 0);
}

void Renderer::DrawOutlinedRect(float x, float y, float w, float h, Vector3 color)
{
	draw_rect(x - 1, y - 1, w + 2, h + 2, Vector3(1, 1, 1));
	draw_rect(x + 1, y + 1, w - 2, h - 2, Vector3(1, 1, 1));
	draw_rect(x, y, w, h, color);
}


void Renderer::DrawBoxFilled(const DirectX::XMFLOAT2& from, const DirectX::XMFLOAT2& size, DirectX::XMFLOAT4 color, float rounding)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	window->DrawList->AddRectFilled(ImVec2(from.x, from.y), ImVec2(size.x, size.y), ImGui::GetColorU32(ImVec4(color.x / 255, color.y / 255, color.z / 255, color.w / 255)), rounding, 15);
}

void Renderer::draw_rectangle(float x, float y, float x2, float y2, float thickness, bool filled, ImColor color) {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (filled)
		window->DrawList->AddRectFilled({ x, y }, { x2, y2 }, color, 0.f, 0.1f); // Round All
	else
		window->DrawList->AddRect({ x, y }, { x2, y2 }, color, 5.f, ImDrawCornerFlags_All, 0.1f);
}

void Renderer::draw_line(float x, float y, float x1, float y1, float thickness, ImColor color) {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	window->DrawList->AddLine({ x, y }, { x1, y1 }, color, thickness);
}

void Renderer::DrawBoxOutlined(const DirectX::XMFLOAT2& from, const DirectX::XMFLOAT2& size, Vector3 RGB , Vector3 OutlineRGB, float rounding, float thickness)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    //ImGuiContext* window = ImGui::GetCurrentContext();
    //window->OverlayDrawList.AddCircleFilled(ImVec2(400, 400), 30, 30, 12);
    window->DrawList->AddRect(ImVec2(from.x, from.y), ImVec2(size.x, size.y), ImGui::GetColorU32(ImVec4(RGB.x / 255, RGB.y / 255, RGB.z / 255, 255 / 255)), rounding, 15, thickness);

    window->DrawList->AddRect(ImVec2(from.x, from.y), ImVec2(size.x, size.y), ImGui::GetColorU32(ImVec4(OutlineRGB.x / 255, OutlineRGB.y / 255, OutlineRGB.z / 255, 255 / 255)), rounding, 15, thickness*1.2);
}

void Renderer::DrawCircle(const DirectX::XMFLOAT2& from, float radius, DirectX::XMFLOAT4 color, float thickness)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	window->DrawList->AddCircle(ImVec2(from.x, from.y), radius, ImGui::GetColorU32(ImVec4(color.x / 255, color.y / 255, color.z / 255, color.w / 255)), 180, thickness);
}

void Renderer::DrawCircleFilled(const DirectX::XMFLOAT2& from, float radius, DirectX::XMFLOAT4 color)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	window->DrawList->AddCircleFilled(ImVec2(from.x, from.y), radius, ImGui::GetColorU32(ImVec4(color.x / 255, color.y / 255, color.z / 255, color.w / 255)), 180);
}

void Renderer::DrawBoxFilled(const DirectX::XMFLOAT2& from, const DirectX::XMFLOAT2& size, Vector3 color, float rounding)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	window->DrawList->AddRectFilled(ImVec2(from.x, from.y), ImVec2(size.x, size.y), ImGui::GetColorU32(ImVec4(color.x / 255, color.y / 255, color.z / 255, 255 / 255)), rounding, 15);
}

void Renderer::DrawHealthbars(float PosX, float PosY, float height, float Value1, Vector3 color) {
	int HealthR = 0, HealthG = 0, HealthB = 0; // Lets make some integers so we can use them for the healthbar. What we will be using this for is to change the color of the healthbar according to the damage done to the enemy.

	float Value2 = static_cast<float>(Value1) / 100.f * height;
	int Value = static_cast<int>(Value2);

	if (Value1 > 0 && Value1 < 101) // if Value is greater then 75 and not greater then 90 draw the health has a darker shade of green.
	{
		draw_rect(PosX - 1, PosY - 1, 5, height, color);
		if (Value1 > 80) {
			DrawBoxFilled(DirectX::XMFLOAT2(PosX, PosY), DirectX::XMFLOAT2(PosX + 3, PosY + Value), Vector3(50, 205, 50), 0);
		}
		else if (Value1 > 60) {
			DrawBoxFilled(DirectX::XMFLOAT2(PosX, PosY), DirectX::XMFLOAT2(PosX + 3, PosY + Value), Vector3(255, 165, 0), 0);
		}
		else if (Value1 > 40) {
			DrawBoxFilled(DirectX::XMFLOAT2(PosX, PosY), DirectX::XMFLOAT2(PosX + 3, PosY + Value), Vector3(255, 165, 0), 0);
		}
		else if (Value1 < 40) {
			DrawBoxFilled(DirectX::XMFLOAT2(PosX, PosY), DirectX::XMFLOAT2(PosX + 3, PosY + Value), Vector3(220, 20, 60), 0);
		}
	}
}

void Renderer::draw_text(Vector3 pos, const char* text, ImColor color, bool centered, bool stroked, ImFont* font, ImColor outline_col) {
	const auto old_color = ImGui::GetStyle().Colors[ImGuiCol_Text];
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	if (centered) {
		const auto size = ImGui::CalcTextSize(text);
		pos.x -= size.x / 2.f;
		pos.y -= size.y / 2.f;
	}

	//if (font) ImGui::PushFont(font);

	if (stroked) {
		outline_col.Value.w *= color.Value.w;
		window->DrawList->AddText({ pos.x + 1, pos.y + 1 }, outline_col, text);
	}

	ImGui::GetStyle().Colors[ImGuiCol_Text] = color;
	window->DrawList->AddText({ pos.x, pos.y }, color, text);
	ImGui::GetStyle().Colors[ImGuiCol_Text] = old_color;

	//if (font) ImGui::PopFont();
}

void Renderer::draw_circle(float x, float y, float radius, bool filled, float thickness, ImColor color, int segments) {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	auto segs = static_cast<int>(radius * (ImGui::GetIO().DisplaySize.y / 1920.f)); // Yeh the proper formula is pi * radius^2 but our pixel density isn't high enough to need that.
	if (filled)
		window->DrawList->AddCircleFilled({ x, y }, radius, color, segments != 0 ? segments : segs);
	else
		window->DrawList->AddCircle({ x, y }, radius, color, segments != 0 ? segments : segs, thickness);
}

Renderer* Renderer::GetInstance()
{
	if (!m_pInstance)
		m_pInstance = new Renderer();

	return m_pInstance;
}