cs2_cheat/
├── driver/
│   ├── driver.cpp          ← WDM kernel read/write
│   └── driver.h
├── src/
│   ├── main.cpp            ← init + game loop
│   ├── memory.h            ← IOCTL comms wrapper
│   ├── offsets.h           ← CS2 offsets (re-dump per patch)
│   ├── math.h              ← Vec2/Vec3, WorldToScreen, angles
│   ├── entity.h            ← CS2 entity structures
│   ├── aimbot.cpp/.h       ← bone aimbot + smooth + triggerbot
│   ├── esp.cpp/.h          ← boxes, skeleton, health, distance
│   └── overlay.cpp/.h      ← D3D11 transparent ImGui window
└── CMakeLists.txt
