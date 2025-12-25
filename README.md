# gimball-ski-game
Lần đầu tiên đưa game lên GitHub

# 🎮 Gimball Ski Noel - Phiên Bản PRO

Một trò chơi trượt tuyết Giáng Sinh vui nhộn được lập trình bằng C++ và thư viện SFML.

## ✨ Tính Năng Chính
-   Vật lý trượt tuyết mượt mà
-   Hệ thống hitbox thông minh (có thể né bằng cách cúi)
-   Nhiều loại chướng ngại vật: Cây thông, Người tuyết, Quả cầu tuyết
-   Hộp quà tăng sức mạnh (bất tử tạm thời)
-   Hiệu ứng hình ảnh: Tuyết rơi
-   Hệ thống âm thanh đầy đủ với nhạc nền

## 🎮 Cách Chơi
-   **Mũi tên Trái/Phải**: Di chuyển
-   **Mũi tên Lên**: Nhảy
-   **Phím S**: Cúi (để né chướng ngại vật cao)
-   **Giữ phím Xuống + Thả ra**: Siêu nhảy
-   **Phím D**: Bật/Tắt chế độ debug (xem hitbox)
-   **Phím R**: Chơi lại từ đầu
-   **Phím ESC**: Thoát game

## 🛠️ Hướng Dẫn Biên Dịch & Chạy Game
### Trên Windows (Với IDE Code::Blocks hoặc Dev-C++)
1.  **Cài đặt thư viện SFML**: Tải bộ SFML phù hợp với trình biên dịch của bạn từ [trang chủ SFML](https://www.sfml-dev.org/download.php).
2.  **Tạo project mới** trong IDE.
3.  **Thêm file `main.cpp`** vào project.
4.  **Cấu hình linker** để thêm các thư viện: `sfml-graphics`, `sfml-window`, `sfml-system`, `sfml-audio`.
5.  **Copy các file assets** (hình ảnh, âm thanh) vào thư mục chứa file `.exe` sau khi biên dịch.
6.  Biên dịch và chạy.

### Sử dụng Trình Biên Dịch Dòng Lệnh (g++)
```bash
g++ main.cpp -o GimballSkiNoel.exe -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
