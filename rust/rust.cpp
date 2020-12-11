#include "rust.hpp"
#include "../Entities/config.hpp"
#include "../Renderer/Drawer.h"
#include "../stb_sprintf.hpp"

#include <thread>
#include <array>

using namespace std;

namespace rust {
	namespace module_info {
		uintptr_t gameassembly_base;
		uintptr_t unityplayer_base;
	}
	
	struct monostr
	{
		char buffer[128];
	};

	std::string readchar(uintptr_t addr) {
		std::string str = kiface::read<monostr>(addr).buffer;
		return str;
	}

	int length(uintptr_t addr) { return kiface::read<int>(addr + 0x10); }

	std::string readstring(uintptr_t addr) {
		try {
			static char buff[128] = { 0 };
			buff[length(addr)] = '\0';

			for (int i = 0; i < length(addr); ++i) {
				if (buff[i] < 128) {
					buff[i] = kiface::read<UCHAR>(addr + 0x14 + (i * 2));
				}
				else {
					buff[i] = '?';
					if (buff[i] >= 0xD800 && buff[i] <= 0xD8FF)
						i++;
				}
			}
			return std::string(&buff[0], &buff[length(addr)]);
		}
		catch (const std::exception& exc) {}
		return xorget("Error");
	}


	void erasestr(std::string& mainStr, const std::string& toErase)
	{
		size_t pos = std::string::npos;
		while ((pos = mainStr.find(toErase)) != std::string::npos)
		{
			mainStr.erase(pos, toErase.length());
		}
	}

	struct color_var_t {
		color_var_t() = default;
		color_var_t(ImColor col) { color = col; }

		ImColor color;
	};

	inline float distance_cursor(Vector2 vec)
	{
		POINT p;
		if (GetCursorPos(&p))
		{
			float ydist = (vec.y - p.y);
			float xdist = (vec.x - p.x);
			float ret = sqrt(pow(ydist, 2) + pow(xdist, 2));
			return ret;
		}
	}

	Camera currentcam;

	void gettaggedobjects() {
		uintptr_t m_skydome = NULL;
		uintptr_t m_camera = NULL;
		uintptr_t last_object = NULL;
		{
			auto current_tagged_base = kiface::read<uintptr_t>(rust::gameobjectmanager + 0x08);
			auto current_tagged_obj = kiface::read<uintptr_t>(current_tagged_base + 0x10);

			while (current_tagged_obj &&
				current_tagged_obj != last_object &&
				(!m_skydome || !m_camera)) {
				INT16 tag = kiface::read<uintptr_t>(current_tagged_obj + 0x54);

				if (!m_camera) {
					if (tag == 5) {
						uintptr_t objClass = kiface::read<uintptr_t>(current_tagged_obj + 0x30);

						uintptr_t ent = kiface::read<uintptr_t>(objClass + 0x18);

						m_camera = kiface::read<uintptr_t>(current_tagged_obj + 0x18);

						VMatrix ViewMatrix = kiface::read<VMatrix>(ent + 0xDC);

						
						currentcam.rotation = ViewMatrix;
						rust::cam = &currentcam;
					}
				}

				if (!m_skydome) {
					if (tag == 20011) {
						m_skydome = kiface::read<DWORD64>(current_tagged_obj + 0x18);

						uint64_t unkown = kiface::read<uint64_t>(current_tagged_obj + 0x30);
						uint64_t unkown1 = kiface::read<uint64_t>(unkown + 0x18);
						uint64_t unkown2 = kiface::read<uint64_t>(unkown1 + 0x28);

						uint64_t AtmosphereParameters = kiface::read<uint64_t>(unkown2 + 0x48);
						uint64_t CycleParameters = kiface::read<uint64_t>(unkown2 + 0x38);

						if (config.day_changer) {
							kiface::write<float>(CycleParameters + 0x10, config.day_time);
						}


						if (1) {
							kiface::write<float>(AtmosphereParameters + 0x10, config.sky_color, false); // retro effect //def 1,f
						}

					}
				}
				last_object = current_tagged_obj;
				current_tagged_base = kiface::read<uintptr_t>(current_tagged_base + 0x8);
				current_tagged_obj = kiface::read<uintptr_t>(current_tagged_base + 0x10);
			}
		}
	}

	Vector3 calculate_angle(const Vector3& source, const Vector3& destination)
	{
		constexpr auto r2d = 57.2957795131f; /* 180 / pi, used for conversion from radians to degrees */
		constexpr auto d2r = 0.01745329251f; /* pi / 180, used for conversion from degrees to radians */

		Vector3 direction = source - destination;
		return { std::asin(direction.y / direction.Length()) * r2d, -std::atan2(direction.x, -direction.z) * r2d, 0.f };
	}


	void run_aimbot(uintptr_t player) {
		if (config.aimbot) {
			auto& [width, height] = ImGui::GetIO().DisplaySize;
			Renderer::GetInstance()->draw_circle(width / 2.f,
				height / 2.f,
				config.fov * 10,
				false,
				2.f,
				ImColor(240, 240, 240, 220),
				60);
			if (player == 20)
				return;

			if (GetAsyncKeyState(config.aimbot_key) & 0x8000) {
				Vector3 angle_to = calculate_angle(rust::local->get_entity_bone(rust::local->baseentity, 48), rust::local->get_entity_bone(player, 48));
				angle_to = rust::local->get_viewangles().lerp(angle_to, (1.f - config.aimbot_smooth));

				rust::local->set_viewangles(angle_to);
			}
		}
	}

	char buffer[512];

	char* format(const char* fmt, ...) {
		va_list args;
		if (!fmt)
			return NULL;

		va_start(args, fmt);
		if (!buffer)
			return NULL;
		if (!args)
			return NULL;
		stbsp_vsnprintf(buffer, 512, fmt, args);
		va_end(args);
		return buffer;
	}

	color_var_t color;
	void entity_esp_thread(ImFont* font, Vector2 res) {
		if (config.DrawPlayers) {
			try {
				VMatrix pViewMatrix = cam->rotation;

				struct bounds_t {
					float left, right, top, bottom;
				};

				bounds_t bounds;
				color = color_var_t(ImColor(255, 255, 255));;
				if (config.rainbow_esp) {
					srand(time(NULL));
					int r = rand() % 255 + 0;
					int g = rand() % 255 + 0;
					int b = rand() % 255 + 0;

					color = color_var_t(ImColor(r, g, b));
				}

				uintptr_t closestTarget = 20;

				Vector3 LocalHead = localclass.get_entity_bone(rust::localclass.baseentity, 48);
				DWORD64 LocalvisualState = kiface::read<DWORD64>(rust::localclass.transform + 0x38);
				Vector3 LocalPosition = kiface::read<Vector3>(LocalvisualState + 0x90);
				LocalPosition = pViewMatrix.WorldToScreen(pViewMatrix, LocalPosition, res);

				float closestDist = FLT_MAX;
				float info_x_center;
				bool IsScientist = false;
				for (baseplayer player : otherplayers) {
					bool PassedChecks = true;

					if (!player.is_alive()) {
						PassedChecks = false;
					}

					if ((player.playername.find(xorget("npc")) != std::string::npos) && !config.scientist) {
						PassedChecks = false;
						IsScientist = true;
					}

					if (player.is_sleeping() && !config.sleeper) {
						PassedChecks = false;
					}

					if (!player.networkid)
						PassedChecks = false;


					Vector3 Head = player.get_entity_bone(player.baseentity, 48);
					Vector3 head = pViewMatrix.WorldToScreen(pViewMatrix, Head, res);

					DWORD64 visualState = kiface::read<DWORD64>(player.transform + 0x38);

					Vector3 Position = kiface::read<Vector3>(visualState + 0x90);
					Position = pViewMatrix.WorldToScreen(pViewMatrix, Position, res);

					float distance = LocalHead.Distance(Head);
					float pHealth = kiface::read<float>(player.baseentity + 0x20C);

					float info_pos = 0.f;

					if (distance > config.max_dist)
					{
						PassedChecks = false;
					}

					if (PassedChecks) {
						if (head.z != 0) {
							struct bone_t {
								Vector3 screen;
								int8_t     index;
								bool       on_screen;
							};

							static std::array<bone_t, 20> bones = {
								bone_t{ Vector3{}, 17, false }, bone_t{ Vector3{}, 18, false },
								bone_t{ Vector3{}, 15, false }, bone_t{ Vector3{}, 14, false },
								bone_t{ Vector3{}, 1, false },  bone_t{ Vector3{}, 2, false },
								bone_t{ Vector3{}, 3, false },  bone_t{ Vector3{}, 6, false },
								bone_t{ Vector3{}, 5, false },  bone_t{ Vector3{}, 21, false },
								bone_t{ Vector3{}, 23, false }, bone_t{ Vector3{}, 48, false },
								bone_t{ Vector3{}, 24, false }, bone_t{ Vector3{}, 25, false },
								bone_t{ Vector3{}, 26, false }, bone_t{ Vector3{}, 27, false },
								bone_t{ Vector3{}, 55, false }, bone_t{ Vector3{}, 56, false },
								bone_t{ Vector3{}, 57, false }, bone_t{ Vector3{}, 76, false }
							};

							const auto get_bounds = [&](bounds_t& out, float expand = 0.f) -> bool {
								bounds = { FLT_MAX, FLT_MIN, FLT_MAX, FLT_MIN };

								for (auto& [bone_screen, bone_idx, on_screen] : bones) {
									Vector3 bone = player.get_entity_bone(player.baseentity, bone_idx);
									bone_screen = pViewMatrix.WorldToScreen(pViewMatrix, bone, res);
									if (bone_screen.x < bounds.left)
										bounds.left = bone_screen.x;
									if (bone_screen.x > bounds.right)
										bounds.right = bone_screen.x;
									if (bone_screen.y < bounds.top)
										bounds.top = bone_screen.y;
									if (bone_screen.y > bounds.bottom)
										bounds.bottom = bone_screen.y;
									on_screen = true;
								}

								if (bounds.left == FLT_MAX)
									return false;
								if (bounds.right == FLT_MIN)
									return false;
								if (bounds.top == FLT_MAX)
									return false;
								if (bounds.bottom == FLT_MIN)
									return false;

								bounds.left -= expand;
								bounds.right += expand;
								bounds.top -= expand;
								bounds.bottom += expand;

								out = bounds;

								return true;
							};

							const auto draw_bone_to_bone = [&](uint32_t idx_a, uint32_t idx_b) {
								auto& bone_a = bones[idx_a];
								auto& bone_b = bones[idx_b];

								if (!bone_a.on_screen || !bone_b.on_screen)
									return;

								auto screen_pos_a = bone_a.screen;
								auto screen_pos_b = bone_b.screen;

								Renderer::GetInstance()->draw_line(screen_pos_a.x,
									screen_pos_a.y,
									screen_pos_b.x,
									screen_pos_b.y,
									0.1f,
									color.color);
							};

							bounds_t bounds;
							if (get_bounds(bounds, 4.f)) {
								auto fov = distance_cursor(Vector2(head.x, head.y));
								int MaxAimFOV = config.fov * 10;
								double dist = sqrt(pow((res.x / 2) - head.x, 2) + pow((res.y / 2) - head.y, 2));
								if (fov < MaxAimFOV) {
									if (dist < closestDist)
									{
										closestDist = dist;
										closestTarget = player.baseentity;
									}
								}

								if (config.box_esp && !config.cornered_box_esp) {
									if(config.filled_box_esp)
										Renderer::GetInstance()->draw_rectangle(bounds.left,
											bounds.top,
											bounds.right,
											bounds.bottom,
											4.f,
											config.filled_box_esp,
											ImColor(color.color.Value.x, color.color.Value.y, color.color.Value.z, 0.25f));
									else
										Renderer::GetInstance()->draw_rectangle(bounds.left,
											bounds.top,
											bounds.right,
											bounds.bottom,
											4.f,
											config.filled_box_esp,
											color.color);
								}
								else if (config.box_esp && config.cornered_box_esp) {
									float X = bounds.left, Y = bounds.top, W = bounds.right - bounds.left, H = bounds.bottom - bounds.top;
									float lineW = (W / 5);
									float lineH = (H / 6);
									float lineT = 1;
									Renderer::GetInstance()->draw_line(X, Y, X, Y + lineH, 1, color.color);//bot right
									Renderer::GetInstance()->draw_line(X, Y, X + lineW, Y, 1, color.color);//bot right
									Renderer::GetInstance()->draw_line(X + W - lineW, Y, X + W, Y, 1, color.color);//bot right
									Renderer::GetInstance()->draw_line(X + W, Y, X + W, Y + lineH, 1, color.color);//bot right
									Renderer::GetInstance()->draw_line(X, Y + H - lineH, X, Y + H, 1, color.color);//bot right
									Renderer::GetInstance()->draw_line(X, Y + H, X + lineW, Y + H, 1, color.color);//bot right
									Renderer::GetInstance()->draw_line(X + W - lineW, Y + H, X + W, Y + H, 1, color.color);//bot right
									Renderer::GetInstance()->draw_line(X + W, Y + H - lineH, X + W, Y + H, 1, color.color);//bot right
								}

								if (config.skeleton_esp) {
									draw_bone_to_bone(2, 3);
									draw_bone_to_bone(3, 4);
									draw_bone_to_bone(4, 5);
									draw_bone_to_bone(5, 6);

									draw_bone_to_bone(4, 9);
									draw_bone_to_bone(9, 10);
									draw_bone_to_bone(10, 11);

									draw_bone_to_bone(10, 12);
									draw_bone_to_bone(12, 13);
									draw_bone_to_bone(13, 14);
									draw_bone_to_bone(14, 15);

									draw_bone_to_bone(10, 16);
									draw_bone_to_bone(16, 17);
									draw_bone_to_bone(17, 18);
									draw_bone_to_bone(18, 19);
								}

								if (config.snapline_esp) {
									Renderer::GetInstance()->draw_line(
										LocalPosition.x,
										LocalPosition.y,
										Position.x,
										Position.y,
										0.5f,
										color.color);
								}

								info_x_center = bounds.left + (bounds.right - bounds.left) / 2.f;

								constexpr auto text_y = 10.f;
								auto           info_y = bounds.bottom + 1.f;

								if (config.health_esp) {
									const auto cur_health = pHealth;
									const auto max_health = (IsScientist ? kiface::read<float>(player.baseentity + 0x210) : 100.f);

									const auto health_pc = min(cur_health / max_health, 1.f);
									const auto health_color =
										ImColor::HSV((health_pc * .25f), 1.f, 1);

									constexpr auto width = 2.f;
									const auto     height = (bounds.bottom - bounds.top) * health_pc;

									const auto x_rhs = bounds.left - 2.f;
									const auto x_lhs = x_rhs - width - 2.f;
									Renderer::GetInstance()->draw_rectangle(x_lhs + 1,
										bounds.bottom,
										x_rhs - 1.f,
										bounds.bottom - height,
										1.f,
										true,
										health_color);
								}

								info_y += text_y / 2.f;

								if (config.name_esp)
								{
									string player_name = player.playername;
									if (player_name.c_str()) {
										Renderer::GetInstance()->draw_text(
											{ info_x_center, info_y, NULL },
											player_name.c_str(),
											color.color,
											true,
											false,
											font,
											ImColor(0.f, 0.f, 0.f, 0.8f * color.color.Value.w));

										info_y += text_y;
									}
								}

								if (config.held_weapon) {
									string weapon_name = player.weaponptr->get_active_weapon_name();
									if (weapon_name.length() > 1) {
										Renderer::GetInstance()->draw_text(
											{ info_x_center, info_y , NULL },
											weapon_name.c_str(),
											color.color,
											true,
											false,
											font,
											ImColor(0.f, 0.f, 0.f, 0.8f * color.color.Value.w));

										info_y += text_y;
									}
								}

								if (config.distance_esp) {
									string distance_str = std::to_string((int)distance);
									Renderer::GetInstance()->draw_text(
										{ info_x_center, info_y, NULL },
										distance_str.c_str(),
										color.color,
										true,
										false,
										font,
										ImColor(0.f, 0.f, 0.f, 0.8f * color.color.Value.w));

									info_y += text_y;
								}
							}
						}
					}

				}


				run_aimbot(closestTarget);

				if (config.heli_esp) {
					for (Entity Vehicles : vehicles) {
						DWORD64 visualState = kiface::read<DWORD64>(Vehicles.transform + 0x38);
						Vector3 Position = kiface::read<Vector3>(visualState + 0x90);
						float distance = LocalHead.Distance(Position);
						Position = pViewMatrix.WorldToScreen(pViewMatrix, Position, res);

						bool PassedChecks = true;
						float health_pc = 0.f;

						float _health = kiface::read<float>(Vehicles.baseentity + 0x1F4);
						float health_max = kiface::read<float>(Vehicles.baseentity + 0x210);

						if (Vehicles.baseentity) {
							health_pc = min(_health / health_max, 1.f);
							if (health_pc <= 0.f)
								PassedChecks = false;
						}

						if (Position.z == 0)
							PassedChecks = false;

						if (distance > config.max_dist_vehicle)
							PassedChecks = false;

						if (!Vehicles.networkid)
							PassedChecks = false;

						string heli_name;

						if (Vehicles.name.find(xorget("minicopter.entity")) != std::string::npos)
							heli_name = xorget("Minicopter");
						else if (Vehicles.name.find(xorget("rowboat.prefab")) != std::string::npos)
							heli_name = xorget("Row Boat");
						else if (Vehicles.name.find(xorget("rhib.prefab")) != std::string::npos)
							heli_name = xorget("RHIB");
						else
							heli_name = xorget("ScrapHelicopter");

						if (PassedChecks) {
							float      info_y = Position.y;

							Renderer::GetInstance()->draw_text(Position,
								format(xorget("%s %d"), heli_name.c_str(), (int)distance),
								color.color,
								true,
								true,
								font,
								ImColor(0.f, 0.f, 0.f, 0.8f * color.color.Value.w));
							info_y += 7.f;
						}

					}
				}

				if (config.corpse_esp) {
					for (Entity Corpse : corpse) {
						DWORD64 visualState = kiface::read<DWORD64>(Corpse.transform + 0x38);
						Vector3 Position = kiface::read<Vector3>(visualState + 0x90);
						Position = pViewMatrix.WorldToScreen(pViewMatrix, Position, res);

						float distance = LocalHead.Distance(Corpse.position);


						string corpse_name = xorget("Corpse");
						bool PassedChecks = true;
						if (Position.z == 0)
							PassedChecks = false;

						if (!Corpse.networkid)
							PassedChecks = false;

						if (distance > config.max_dist_corpse)
							PassedChecks = false;

						if (corpse_name.size() > 0 && PassedChecks) {
							Renderer::GetInstance()->draw_text(Position,
								format(xorget("%s %d"), corpse_name.c_str(), (int)distance),
								color.color,
								true,
								true,
								font,
								ImColor(0.f, 0.f, 0.f, 0.8f * color.color.Value.w));
						}

					}
				}

				if (config.dropped_esp) {
					for (Entity droppeditem : DroppedItem) {
						DWORD64 visualState = kiface::read<DWORD64>(droppeditem.transform + 0x38);
						Vector3 Position = kiface::read<Vector3>(visualState + 0x90);
						Position = pViewMatrix.WorldToScreen(pViewMatrix, Position, res);

						float distance = LocalHead.Distance(droppeditem.position);

						string dist = to_string((int)distance);

						bool PassedChecks = true;
						if (Position.z == 0)
							PassedChecks = false;

						if (droppeditem.position.x == 0)
							PassedChecks = false;

						if (distance > config.max_dist_dropped_items)
							PassedChecks = false;

						if (!droppeditem.networkid)
							PassedChecks = false;

						if (droppeditem.name.find(xorget("backpack")) != std::string::npos)
							PassedChecks = false;

						string item_name = droppeditem.name;
						if (PassedChecks) {
							if (item_name.size() > 0) {
								Renderer::GetInstance()->draw_text(Position,
									format(xorget("%s %d"), item_name.c_str(), (int)distance),
									color.color,
									true,
									true,
									font,
									ImColor(0.f, 0.f, 0.f, 0.8f * color.color.Value.w));
							}
						}
					}
				}

				if (config.stash_esp) {
					for (Entity stash : Stash) {
						DWORD64 visualState = kiface::read<DWORD64>(stash.transform + 0x38);
						Vector3 Position = kiface::read<Vector3>(visualState + 0x90);
						Position = pViewMatrix.WorldToScreen(pViewMatrix, Position, res);

						float distance = LocalHead.Distance(stash.position);

						bool PassedChecks = true;
						if (Position.z == 0)
							PassedChecks = false;

						if (distance > config.max_dist_stash)
							PassedChecks = false;

						string stash_name = xorget("Stash");
						if (PassedChecks) {
							Renderer::GetInstance()->draw_text(Position,
								format(xorget("%s %d"), stash_name.c_str(), (int)distance),
								color.color,
								true,
								true,
								font,
								ImColor(0.f, 0.f, 0.f, 0.8f * color.color.Value.w));
						}
					}
				}

				if (config.airdrop) {
					for (Entity airdrop : Airdrop) {
						DWORD64 visualState = kiface::read<DWORD64>(airdrop.transform + 0x38);
						Vector3 Position = kiface::read<Vector3>(visualState + 0x90);
						Position = pViewMatrix.WorldToScreen(pViewMatrix, Position, res);

						float distance = LocalHead.Distance(airdrop.position);

						bool PassedChecks = true;
						if (Position.z == 0)
							PassedChecks = false;

						if (!airdrop.networkid)
							PassedChecks = false;

						if (distance > config.max_dist_airdrop)
							PassedChecks = false;

						if (PassedChecks) {
							Renderer::GetInstance()->draw_text(Position,
								format(xorget("%s %d"), xorget("Airdrop"), (int)distance),
								color.color,
								true,
								true,
								font,
								ImColor(0.f, 0.f, 0.f, 0.8f * color.color.Value.w));
						}
					}
				}

				if (config.tool_cupboard) {
					for (Entity TC : tool_cupboard) {
						DWORD64 visualState = kiface::read<DWORD64>(TC.transform + 0x38);
						Vector3 Position = kiface::read<Vector3>(visualState + 0x90);
						Position = pViewMatrix.WorldToScreen(pViewMatrix, Position, res);

						float distance = LocalHead.Distance(TC.position);

						bool PassedChecks = true;
						if (Position.z == 0)
							PassedChecks = false;

						if (distance > config.max_dist_tool_cupboard)
							PassedChecks = false;

						//if (!TC.networkid)
						//	PassedChecks = false;

						if (PassedChecks) {
							Renderer::GetInstance()->draw_text(Position,
								format(xorget("%s %d"), xorget("Tool Cupboard"), (int)distance),
								color.color,
								true,
								true,
								font,
								ImColor(0.f, 0.f, 0.f, 0.8f * color.color.Value.w));
						}
					}
				}

				if (config.hackable_crate) {
					for (Entity crate : hackable_crate) {
						DWORD64 visualState = kiface::read<DWORD64>(crate.transform + 0x38);
						Vector3 Position = kiface::read<Vector3>(visualState + 0x90);
						Position = pViewMatrix.WorldToScreen(pViewMatrix, Position, res);

						float distance = LocalHead.Distance(crate.position);

						bool PassedChecks = true;
						if (Position.z == 0)
							PassedChecks = false;

						if (distance > config.max_dist_hackable)
							PassedChecks = false;

						//if (!crate.networkid)
						//	PassedChecks = false;

						if (PassedChecks) {
							Renderer::GetInstance()->draw_text(Position,
								format(xorget("%s %d"), xorget("Hackable Crate"), (int)distance),
								color.color,
								true,
								true,
								font,
								ImColor(0.f, 0.f, 0.f, 0.8f * color.color.Value.w));
						}
					}
				}

				if (config.cargo_esp) {
					for (Entity cargo : cargo_ship) {
						DWORD64 visualState = kiface::read<DWORD64>(cargo.transform + 0x38);
						Vector3 Position = kiface::read<Vector3>(visualState + 0x90);
						Position = pViewMatrix.WorldToScreen(pViewMatrix, Position, res);

						float distance = LocalHead.Distance(cargo.position);

						bool PassedChecks = true;
						if (Position.z == 0)
							PassedChecks = false;

						if (distance > config.max_dist_vehicle)
							PassedChecks = false;

						if (!cargo.networkid)
							PassedChecks = false;

						if (PassedChecks) {
							Renderer::GetInstance()->draw_text(Position,
								format(xorget("%s %d"), xorget("Cargo Ship"), (int)distance),
								color.color,
								true,
								true,
								font,
								ImColor(0.f, 0.f, 0.f, 0.8f * color.color.Value.w));
						}
					}
				}

				if (config.food_esp) {
					for (Entity Food : food) {
						DWORD64 visualState = kiface::read<DWORD64>(Food.transform + 0x38);
						Vector3 Position = kiface::read<Vector3>(visualState + 0x90);
						Position = pViewMatrix.WorldToScreen(pViewMatrix, Position, res);

						float distance = LocalHead.Distance(Food.position);

						bool PassedChecks = true;
						if (Position.z == 0)
							PassedChecks = false;

						if (distance > config.max_dist_collectable)
							PassedChecks = false;

						//if (!Food.networkid)
						//	PassedChecks = false;

						if (PassedChecks) {
							Renderer::GetInstance()->draw_text(Position,
								format(xorget("%s %d"), xorget("Food"), (int)distance),
								color.color,
								true,
								true,
								font,
								ImColor(0.f, 0.f, 0.f, 0.8f * color.color.Value.w));
						}
					}
				}

				if (config.hemp) {
					for (Entity Hemp : hemp) {
						DWORD64 visualState = kiface::read<DWORD64>(Hemp.transform + 0x38);
						Vector3 Position = kiface::read<Vector3>(visualState + 0x90);
						Position = pViewMatrix.WorldToScreen(pViewMatrix, Position, res);

						float distance = LocalHead.Distance(Hemp.position);

						bool PassedChecks = true;
						if (Position.z == 0)
							PassedChecks = false;

						if (distance > config.max_dist_collectable)
							PassedChecks = false;

						//if (!Hemp.networkid)
						//	PassedChecks = false;

						if (PassedChecks) {
							Renderer::GetInstance()->draw_text(Position,
								format(xorget("%s %d"), xorget("Hemp"), (int)distance),
								color.color,
								true,
								true,
								font,
								ImColor(0.f, 0.f, 0.f, 0.8f * color.color.Value.w));
						}
					}
				}

				if (config.patrol_heli) {
					for (Entity PatrolHeli : patrol_heli) {
						DWORD64 visualState = kiface::read<DWORD64>(PatrolHeli.transform + 0x38);
						Vector3 Position = kiface::read<Vector3>(visualState + 0x90);
						Position = pViewMatrix.WorldToScreen(pViewMatrix, Position, res);

						float distance = LocalHead.Distance(PatrolHeli.position);

						bool PassedChecks = true;
						if (Position.z == 0)
							PassedChecks = false;

						if (distance > config.max_dist_vehicle)
							PassedChecks = false;

						if (!PatrolHeli.networkid)
							PassedChecks = false;

						if (PassedChecks) {
							Renderer::GetInstance()->draw_text(Position,
								format(xorget("%s %d"), xorget("Patrol Helicopter"), (int)distance),
								color.color,
								true,
								true,
								font,
								ImColor(0.f, 0.f, 0.f, 0.8f * color.color.Value.w));
						}
					}
				}

				if (config.node_esp) {
					if (config.metal_node) {
						for (Entity metalnode : MetalNodes) {
							DWORD64 visualState = kiface::read<DWORD64>(metalnode.transform + 0x38);
							Vector3 Position = kiface::read<Vector3>(visualState + 0x90);
							Position = pViewMatrix.WorldToScreen(pViewMatrix, Position, res);

							float distance = LocalHead.Distance(metalnode.position);

							bool PassedChecks = true;
							if (Position.z == 0)
								PassedChecks = false;

							if (distance > config.max_dist_ore)
								PassedChecks = false;

							string metal_name = xorget("Metal Ore");
							if (PassedChecks) {
								if (metal_name.size() > 0) {
									Renderer::GetInstance()->draw_text(Position,
										format(xorget("%s %d"), metal_name.c_str(), (int)distance),
										color.color,
										true,
										true,
										font,
										ImColor(0.f, 0.f, 0.f, 0.8f * color.color.Value.w));
								}
							}
						}
					}
					if (config.sulfur_node) {
						for (Entity sulfurnode : SulfurNodes) {
							DWORD64 visualState = kiface::read<DWORD64>(sulfurnode.transform + 0x38);
							Vector3 Position = kiface::read<Vector3>(visualState + 0x90);
							Position = pViewMatrix.WorldToScreen(pViewMatrix, Position, res);

							float distance = LocalHead.Distance(sulfurnode.position);
							bool PassedChecks = true;
							if (Position.z == 0)
								PassedChecks = false;

							if (distance > config.max_dist_ore)
								PassedChecks = false;

							string sulfur_name = xorget("Sulfur Ore");
							if (PassedChecks) {
								if (sulfur_name.size() > 0) {
									Renderer::GetInstance()->draw_text(Position,
										format(xorget("%s %d"), sulfur_name.c_str(), (int)distance),
										color.color,
										true,
										true,
										font,
										ImColor(0.f, 0.f, 0.f, 0.8f * color.color.Value.w));
								}
							}
						}
					}
					if (config.stone_node) {
						for (Entity stonenode : StoneNodes) {
							DWORD64 visualState = kiface::read<DWORD64>(stonenode.transform + 0x38);
							Vector3 Position = kiface::read<Vector3>(visualState + 0x90);
							Position = pViewMatrix.WorldToScreen(pViewMatrix, Position, res);

							float distance = LocalHead.Distance(stonenode.position);

							bool PassedChecks = true;
							if (Position.z == 0)
								PassedChecks = false;

							if (distance > config.max_dist_ore)
								PassedChecks = false;

							string stone_name = xorget("Stone Ore");
							if (PassedChecks) {
								if (stone_name.size() > 0) {
									Renderer::GetInstance()->draw_text(Position,
										format(xorget("%s %d"), stone_name.c_str(), (int)distance),
										color.color,
										true,
										true,
										font,
										ImColor(0.f, 0.f, 0.f, 0.8f * color.color.Value.w));
								}
							}
						}
					}
				}

				if (config.high_tier_crate) {
					for (Entity high_crate : high_tier_crates) {
						DWORD64 visualState = kiface::read<DWORD64>(high_crate.transform + 0x38);
						Vector3 Position = kiface::read<Vector3>(visualState + 0x90);
						Position = pViewMatrix.WorldToScreen(pViewMatrix, Position, res);

						float distance = LocalHead.Distance(high_crate.position);

						bool PassedChecks = true;
						if (Position.z == 0)
							PassedChecks = false;

						if (distance > config.max_dist_high_crate)
							PassedChecks = false;

						//if (!Hemp.networkid)
						//	PassedChecks = false;
						string goName = high_crate.name;
						erasestr(goName, xorget("assets/bundled/prefabs/radtown/"));
						erasestr(goName, xorget(".prefab"));

						if (PassedChecks) {
							Renderer::GetInstance()->draw_text(Position,
								format(xorget("%s %d"), goName.c_str(), (int)distance),
								color.color,
								true,
								true,
								font,
								ImColor(0.f, 0.f, 0.f, 0.8f * color.color.Value.w));
						}
					}
				}

				if (config.low_tier_crate) {
					for (Entity low_crates : low_tier_crates) {

						DWORD64 visualState = kiface::read<DWORD64>(low_crates.transform + 0x38);
						Vector3 Position = kiface::read<Vector3>(visualState + 0x90);
						Position = pViewMatrix.WorldToScreen(pViewMatrix, Position, res);

						float distance = LocalHead.Distance(low_crates.position);

						bool PassedChecks = true;
						if (Position.z == 0)
							PassedChecks = false;

						if (distance > config.max_dist_low_crate)
							PassedChecks = false;

						//if (!Hemp.networkid)
						//	PassedChecks = false;
						string goName = low_crates.name;
						erasestr(goName, xorget("assets/bundled/prefabs/radtown/"));
						erasestr(goName, xorget(".prefab"));

						if (PassedChecks) {
							Renderer::GetInstance()->draw_text(Position,
								format(xorget("%s %d"), goName.c_str(), (int)distance),
								color.color,
								true,
								true,
								font,
								ImColor(0.f, 0.f, 0.f, 0.8f * color.color.Value.w));
						}
					}
				}
			}
			catch (exception& e) {
				std::cout << e.what() << endl;
			}
		}
	}

	void localplayer_thread() {
		if (rust::local) {
			if (config.doublejump) {
				rust::local->setnojumpblock();
			}
			if (config.setadmin) {
				rust::local->setadminflag();
			}
			if (config.spiderman) {
				rust::local->setspiderman();
			}

			if (rust::local->weaponptr) {
				if (rust::local->weaponptr->is_weapon()) {
					if (config.recoil)
						rust::local->weaponptr->setnorecoil();

					if (config.spread)
						rust::local->weaponptr->setnospread();

					if (config.sway)
						rust::local->weaponptr->setnosway();
				}
			}
		}
	}


	bool init() {
		kiface::rust_pid = util::get_pid(xorget("RustClient.exe"));
		if (!kiface::rust_pid)
			return false;

		kiface::procHandle = OpenProcess(PROCESS_ALL_ACCESS, NULL, kiface::rust_pid);
		if (!kiface::procHandle) {
			std::cout << "Failed to get handle to process" << std::endl;
			return false;
		}

		module_info::gameassembly_base = util::dwGetModuleBaseAddress(kiface::rust_pid, xorget(L"GameAssembly.dll"));
		if (!module_info::gameassembly_base) {
			std::cout << "Failed to get game assembly base" << std::endl;
			return false;
		}

		module_info::unityplayer_base = util::dwGetModuleBaseAddress(kiface::rust_pid, (xorget(L"UnityPlayer.dll")));
		if (!module_info::unityplayer_base) {
			std::cout << "Failed to get unityplayer base." << std::endl;
			return false;
		}
		Sleep(8000);
		basenetworkable = kiface::read<uintptr_t>(module_info::gameassembly_base + 0x28ffd20);
		if (!basenetworkable) {
			std::cout << "Game updated, cheat outdated" << std::endl;
			return false;
		}

		gameobjectmanager = kiface::read<uintptr_t>(module_info::unityplayer_base + 0x17A6AD8);
		if (!gameobjectmanager) {
			std::cout << "Unity version is outdated" << std::endl;
			return false;
		}
		return true;
	}
}