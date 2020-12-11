#include "../kinface/kiface.h"
#include "../xorstr.hpp"
#include "../Entities/Baseplayer.hpp"
#include "../Vectors/Vector.hpp"
#include "../Entities/Entities.hpp"
#include "Camera.hpp"
#include "../Renderer/ImGui/imgui.h"

#include <vector>
#include <string>

namespace rust {
	inline uintptr_t gameobjectmanager;
	inline uintptr_t basenetworkable;

	inline baseplayer* local;
	inline baseplayer localclass;
	inline Camera* cam;

	bool init();
	void localplayer_thread();
	void entity_esp_thread(ImFont* font, Vector2 res);
	void gettaggedobjects();

	std::string readchar(uintptr_t addr);
	int length(uintptr_t addr);
	std::string readstring(uintptr_t addr);
}