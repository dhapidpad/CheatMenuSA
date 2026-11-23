# 🎮 HUONG DAN SU DUNG CHI TIET - CHEATMENUSA v2.5 PRO AAA
> **Dự án:** CheatMenuSA - Mod Menu / Trainer cao cấp dành cho GTA San Andreas & GSOCity Offline  
> **Kiến trúc:** C++20 / Direct3D 9 ImGui-like Native Renderer / GTA Plugin-SDK / MCP TCP Bridge  

---

## 📌 1. BẢNG PHÍM TẮT (HOTKEYS) NHANH

| Phím Tắt | Chức Năng | Ghi Chú |
| :--- | :--- | :--- |
| **`INSERT`** hoặc **`F11`** | **Mở / Đóng Menu Cheat** | Hiển thị con trỏ chuột mượt mà, tạm khóa input game |
| **`DELETE`** | **Unload Mod an toàn** | Giải phóng hook, đóng socket MCP và dọn dẹp bộ nhớ |
| **`RSHIFT`** (Right Shift) | **Bật / Tắt Airbrake (Bay xuyên tường)** | Bay tự do theo hướng camera 6-DOF |
| **`W / A / S / D`** | **Di chuyển khi Bay (Airbrake)** | Tiến, lùi, sang trái, sang phải |
| **`SPACE` / `LSHIFT`** | **Bay Lên / Bay Xuống (Airbrake)** | Nâng độ cao hoặc hạ độ cao mượt mà |
| **`NUMPAD 0`** | **Chế độ Bay Chậm (Slow-mo Precision)** | Giảm tốc độ để căn góc chụp ảnh hoặc luồn lách |
| **`ALT`** hoặc **`LSHIFT`** | **Car Rocket Nitro (Phản lực xe)** | Nhấn giữ khi đang lái xe để phóng cực mạnh |
| **`X`** | **Lật xe tức thì (Flip Vehicle)** | Khôi phục xe bị lật ngửa về trạng thái 4 bánh chạm đất |
| **`Chuột trái`** | **Tương tác Menu** | Click chọn Toggle, kéo Slider, chọn Combobox, bấm Button |

---

## 🖥️ 2. GIAO DIỆN HIỆN ĐẠI (AAA UI/UX) & CÁCH SỬ DỤNG

Menu được thiết kế lại hoàn toàn theo phong cách **Cyberpunk / Dark Slate Glassmorphism**:
- **Sidebar Tabs với hiệu ứng Lerp mượt:** Khi chuyển tab, vệt sáng thanh chỉ mục di chuyển mượt mà (smooth lerp interpolation) với âm thanh phản hồi native GTA SA.
- **Toggle Switches (iOS / Cyberpunk style):** Nút gạt bo tròn với viên bi sáng trượt êm ái, hiển thị rõ ràng nhãn `[ON]` / `[OFF]`.
- **Neon Sliders với Value Badge Pill:** Thanh kéo hiển thị gradient chuyển màu Neon Cyan sang Electric Blue, viên bi kéo phát sáng và thẻ huy hiệu (Pill Badge) hiển thị con số chính xác.
- **Combobox Cards:** Thẻ chọn dạng card bo góc kèm mũi tên điều hướng trái/phải `◀` / `▶` có hiệu ứng hover và âm thanh click.
- **Tactile Buttons:** Nút bấm có hiệu ứng viền phát sáng khi rê chuột và lún nhẹ (press depth) khi bấm xuống.
- **Hệ thống Tooltip thông minh ở Footer:** Rê chuột vào bất kỳ tính năng nào, thanh Footer bên dưới sẽ lập tức hiển thị giải thích chi tiết chức năng đó.

---

## 📂 3. HƯỚNG DẪN CHI TIẾT TỪNG TAB CHỨC NĂNG

### 🎯 TAB 0: COMBAT (Tự Động Chiến Đấu & Bắn Súng)
- **Silent Aimbot:** Tự động nắn đường đạn bay thẳng vào đối thủ mà không làm rung lắc camera góc nhìn thứ nhất, giúp ngắm bắn tự nhiên.
- **Target Visible Only:** Chỉ khóa mục tiêu khi đối thủ không bị che khuất bởi tường, vật cản.
- **Draw FOV Circle:** Vẽ vòng tròn bán kính nhận diện mục tiêu xung quanh tâm súng.
- **Aimbot FOV Radius:** Thanh trượt điều chỉnh độ rộng vòng ngắm (từ 20px đến 400px).
- **Triggerbot (Auto Attack):** Tự động bóp cò ngay khoảnh khắc tâm súng lia trúng kẻ thù.
- **Rapid Fire:** Chuyển tất cả vũ khí bán tự động (Pistol, Shotgun, Sniper) thành chế độ bắn liên thanh cực nhanh.
- **Smart Bone Selection:** Tự động chọn khớp xương gần tâm súng nhất (Đầu, Cổ, Ngực).
- **Ưu tiên Băng đảng (Gangs) / Cảnh sát (Cops):** Lựa chọn mục tiêu ưu tiên khi giao tranh.
- **Perfect Accuracy & Range:** Xóa bỏ hoàn toàn độ giật/độ nở tâm (No Spread) và tăng tầm bắn tối đa.

---

### 👁️ TAB 1: VISUALS & LIVE MANNEQUIN PREVIEW (ESP Nhìn Xuyên Tường)
- **Master ESP Switch:** Công tắc tổng bật/tắt toàn bộ tính năng nhìn xuyên vật cản.
- **Bounding Box Style:** Chọn kiểu khung bao quanh đối phương:
  - `Off`: Tắt khung.
  - `Full 2D Box`: Khung chữ nhật đầy đủ viền Neon Cyan.
  - `Corner Box`: Khung 4 góc hiện đại chuẩn game FPS thi đấu.
- **Skeleton ESP (3D Bones):** Vẽ khung xương 3D thời gian thực theo từng khớp cử động của nhân vật.
- **Health Bar (Gradient):** Thanh máu thanh mảnh chuyển màu động từ Xanh lá (đầy máu) sang Đỏ (yếu máu).
- **Armor Bar:** Thanh giáp màu xanh dương bạc hiển thị ngay cạnh thanh máu.
- **Distance & Weapon Display:** Hiển thị khoảng cách theo mét và tên loại súng đối phương đang cầm.
- **Draw Snaplines:** Kẻ đường chỉ dẫn từ mép dưới màn hình nối thẳng tới chân mục tiêu.
- **Overlays Phụ:** Radar Blips mini, ESP xe xung quanh, ESP hình vẽ Graffiti chưa hoàn thành, ESP vật phẩm sưu tầm (Ngọc trai, Móng ngựa, Điểm chụp ảnh).
- ⭐ **WIDGET MÔ PHỎNG ESP TRỰC QUAN (LIVE PREVIEW):** Một hình nộm Mannequin 3D/2D trực tiếp phản hồi ngay trong menu. Khi bạn bật/tắt Box, Skeleton, Health, Armor, Weapon hay Snapline, hình nộm bên phải sẽ cập nhật tức thì để bạn xem trước góc nhìn mà không cần ra ngoài game!

---

### 🏃 TAB 2: MOVEMENT (Bay Xuyên Tường & Di Chuyển Thần Tốc)
- **Airbrake (Free Flight):** Nhấn phím **`RSHIFT`** để kích hoạt chế độ bay. Người chơi có thể bay lượn tự do qua các tòa nhà, lòng đất hay leo lên đỉnh tháp cao nhất.
  - `Flight Speed`: Điều chỉnh tốc độ bay từ 40 đến 500.
  - `NUMPAD 0`: Nhấn giữ để bay cực chậm giúp tinh chỉnh vị trí hạ cánh chính xác từng milimet.
- **Speedhack:** Tăng tốc độ chạy bộ và di chuyển gấp 1.5x đến 6.0x lần bình thường.
- **Super Mega Jump:** Nhấn giữ phím Nhảy (Space) để bật nhảy cao gấp 10 lần, vượt qua các bức tường cao.
- **Infinite Stamina & Oxygen:** Chạy nước rút không bao giờ mệt, lặn dưới nước không bao giờ cạn dưỡng khí.
- **Car Rocket Nitro:** Giữ phím `ALT` hoặc `LSHIFT` khi lái xe để kích hoạt luồng phản lực tên lửa đẩy xe lao vút về phía trước.

---

### 🛡️ TAB 3: PLAYER & VEHICLE CHEATS (Bất Tử & Quản Lý Xe)
- **Godmode (Bất Tử):** Miễn nhiễm hoàn toàn với sát thương súng, nổ xe, ngã từ trên trời rơi xuống, chết đuối hoặc bị đánh cận chiến.
- **Infinite Ammo & No Reload:** Đạn vô hạn, bắn liên tục không bao giờ phải thay băng đạn.
- **Never Wanted:** Khóa vĩnh viễn cấp độ truy nã ở 0 sao, cảnh sát sẽ không bao giờ truy đuổi bạn.
- **Vehicle Godmode:** Xe của bạn không thể bị trầy xước, nát đầu hoặc phát nổ khi va chạm mạnh.
- **Instant Fix Vehicle:** Bấm 1 chạm để phục hồi 100% độ bền và vẻ ngoài bóng bẩy của xe.
- **Super Grip / Heavy Handling:** Tăng trọng tải và độ bám đường, xe không bị trượt bánh khi bo cua tốc độ cao.
- **Auto Flip:** Tự động phát hiện khi xe bị lật úp và dựng đứng lại ngay lập tức (hoặc bấm phím `X`).

---

### ⚡ TAB 4: ACTIONS & VEHICLE SPAWNER (Hành Động Nhanh & Triệu Hồi Xe)
- **Hồi phục tức thì:** Bấm nút hồi 100% Máu + 100% Giáp, Xóa sạch sao truy nã chỉ trong 1 click.
- **Trang bị Bộ Vũ Khí (Weapon Loadouts):**
  - *Heavy Artillery:* Minigun 6 nòng, Súng phóng lựu Rocket, Shotgun Spas12.
  - *Special Ops:* M4 Carbine, MP5 SMG, Súng tỉa Sniper Rifle, Lựu đạn nổ.
  - *Street Thug:* Súng lục Desert Eagle, Shotgun cưa nòng Sawn-off, Dao găm.
- **Thời tiết & Thời gian:** Thay đổi tức thời thời tiết (Trời nắng gắt, Mây mù, Mưa bão) và khóa giờ chơi (12:00 Trưa, 00:00 Đêm).
- **Vehicle Spawner (Triệu Hồi Siêu Xe):** Nhấp chọn để triệu hồi ngay trước mặt các mẫu xe huyền thoại:
  - *Infernus, Bullet, Cheetah, Sultan, Elegy, NRG-500 (Mô tô đua), Sanchez, Hydra (Phản lực chiến đấu), Hunter (Trực thăng tên lửa), Tank Rhino...*

---

### 🖥️ TAB 5: DISPLAY & GFX TWEAKS (Tối Ưu Đồ Họa & Toàn Màn Hình)
Dành riêng cho màn hình rộng và máy tính hiện đại chơi bản GSOCity:
- **Borderless Fullscreen (Toàn Màn Hình Không Viền):** Chuyển game về chế độ Borderless. Giúp chuyển đổi Alt+Tab qua lại giữa Game, Discord, Trình duyệt web mượt mà 100%, không bao giờ bị đen màn hình hay đơ crash game!
- **Custom Camera FOV:** Kéo thanh trượt để mở rộng góc nhìn của nhân vật từ 30° lên đến 130° (chuẩn điện ảnh / góc nhìn rộng hiện đại).
- **Far Clip (Draw Distance Multiplier):** Tăng tầm nhìn xa của bản đồ gấp 1.0x - 5.0x lần, loại bỏ hoàn toàn hiện tượng sương mù che khuất tầm mắt khi ngắm từ xa.
- **FPS Limiter Unlock:** Mở khóa giới hạn 30 FPS mặc định của GTA SA, hỗ trợ tần số quét 60Hz, 120Hz, 144Hz, 240Hz siêu mượt.
- **Widescreen Fix:** Khắc phục tình trạng méo hình trên các tỷ lệ màn hình 16:9, 16:10, 21:9 UltraWide.

---

### 🔍 TAB 6: DÒ TÌM MEMORY OFFSETS (Offset Scanner)
Công cụ hữu ích phục vụ nghiên cứu bộ nhớ game:
- Quét động vị trí con trỏ của `CPlayerPed`, `Local Player Coordinates (X, Y, Z)`, Ma trận xoay `Matrix`, `Ped Pool`, `Vehicle Pool`, v.v.
- Hỗ trợ công nghệ đọc an toàn Structured Exception Handling (SEH) không gây crash game khi gặp con trỏ trỏ vào vùng nhớ không hợp lệ.
- **Nút "Export Offsets to File":** Xuất toàn bộ địa chỉ bộ nhớ vừa quét được ra tệp `offsets_dump.txt` nằm ngay trong thư mục gốc của game.

---

### 🤖 TAB 7: LOGS HỆ THỐNG & KẾT NỐI MCP TCP BRIDGE
- **In-game Terminal Log:** Hiển thị trực tiếp các thông điệp ghi log của hệ thống, phát hiện sự kiện phím bấm, nạp thư viện và trạng thái hook.
- **Nút "Export Logs to File":** Lưu lại toàn bộ nhật ký ra tệp `CheatMenu.log`.
- **MCP Bridge (Cổng kết nối AI & Tự động hóa qua TCP):**
  - Khởi chạy một TCP Socket Server siêu nhẹ lắng nghe tại địa chỉ `127.0.0.1:13337`.
  - Cho phép các Agent AI hoặc công cụ Python bên ngoài giao tiếp, đọc dữ liệu nhân vật và điều khiển game bằng định dạng JSON chuẩn.

---

## 🐍 4. HƯỚNG DẪN KẾT NỐI MCP PYTHON CLIENT

Dự án cung cấp sẵn script điều khiển `scripts/mcp_client.py`. Bạn có thể mở PowerShell hoặc CMD trên máy và điều khiển game từ xa:

```bash
# Kiểm tra kết nối tới game
python scripts/mcp_client.py ping

# Đọc tọa độ, máu, giáp và trạng thái xe của người chơi
python scripts/mcp_client.py status

# Dịch chuyển tức thời tới tọa độ bất kỳ (VD: Grove Street)
python scripts/mcp_client.py tp 2495.2 -1687.5 13.5

# Triệu hồi một chiếc Infernus (Model ID: 411)
python scripts/mcp_client.py spawn 411

# Quét nhanh bộ nhớ và lấy danh sách Offset qua JSON
python scripts/mcp_client.py offsets

# Bật Godmode và Vô hạn đạn từ dòng lệnh
python scripts/mcp_client.py cheat player.godmode true
python scripts/mcp_client.py cheat combat.infinite_ammo true
```

---

## 🚀 5. HƯỚNG DẪN CÀI ĐẶT THỦ CÔNG

1. Tải tệp `CheatMenuSA.asi` từ thư mục Release hoặc thư mục `bin_release/`.
2. Sao chép tệp `CheatMenuSA.asi` vào thư mục cài đặt GTA San Andreas / GSOCity:
   - Đường dẫn: `D:\Program Files\GSOCity\GTA\` hoặc `D:\Program Files\GSOCity\GTA\scripts\`
3. Đảm bảo game đã có file `vorbisFile.dll` (hoặc `dinput8.dll`) ASI Loader.
4. Khởi động game, nhấn phím **`INSERT`** hoặc **`F11`** để trải nghiệm giao diện đẳng cấp!
