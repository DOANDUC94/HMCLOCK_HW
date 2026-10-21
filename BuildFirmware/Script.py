import os
import subprocess

# --- CẤU HÌNH ĐẦU VÀO / ĐẦU RA ---
FILE_FW_BIN = "firmware.bin"
FILE_VERSION_H = "firmware_version.h"
FILE_FONT = "all_fonts.bin"

EXE_MKIMAGE = os.path.join("bin", "mkimage.exe")

OUT_FW1_IMG = "fw1.img"
OUT_FINAL_FLASH = "firm256kb.bin"

# --- THÔNG SỐ VÙNG NHỚ (HEX) ---
FLASH_SIZE = 0x40000       # 256KB
ADDR_FW1 = 0x00000         # Vị trí FW1
ADDR_FW2 = 0x10000         # Vị trí FW2 (Copy của FW1)
ADDR_FONT = 0x20000        # Vị trí Font
ADDR_HEADER = 0x38000      # Vị trí Product Header (SBL trong OTP sẽ đọc ở đây)

def run_cmd(cmd):
    """Hàm thực thi câu lệnh hệ thống."""
    print(f"Executing: {cmd}")
    res = subprocess.run(cmd, shell=True)
    if res.returncode != 0:
        print(f"--> [LỖI] Thực thi thất bại câu lệnh: {cmd}")
        exit(1)

def main():
    # 1. Kiểm tra sự tồn tại của file cần thiết
    for f in [FILE_FW_BIN, FILE_VERSION_H, FILE_FONT, EXE_MKIMAGE]:
        if not os.path.exists(f):
            print(f"Lỗi: Không tìm thấy file quan trọng: '{f}'")
            return

    print("--- BƯỚC 1: Gọi mkimage.exe để đóng gói firmware.bin -> fw1.img ---")
    # Lệnh này sinh ra fw1.img chứa 32 bytes Image Header xịn ở đầu file
    cmd_mk_img = f'"{EXE_MKIMAGE}" single "{FILE_FW_BIN}" "{FILE_VERSION_H}" "{OUT_FW1_IMG}"'
    run_cmd(cmd_mk_img)

    # Đọc dữ liệu từ file fw1.img (đã bao gồm Header 32 byte)
    with open(OUT_FW1_IMG, "rb") as f:
        fw_img_data = f.read()

    # Đọc dữ liệu file Font
    with open(FILE_FONT, "rb") as f:
        font_data = f.read()

    print("\n--- BƯỚC 2: Khởi tạo bộ đệm Flash 256KB và map các phân vùng ---")
    # Tạo mảng byte trắng fill toàn bộ bằng 0xFF
    flash_data = bytearray([0xFF] * FLASH_SIZE)

    # Nạp FW1 (đã đóng gói img) vào 0x00000
    end_fw1 = ADDR_FW1 + len(fw_img_data)
    flash_data[ADDR_FW1:end_fw1] = fw_img_data
    print(f" -> Đã nạp FW1 (độ dài {len(fw_img_data)} bytes) vào 0x{ADDR_FW1:05X}")

    # Nạp FW2 (Copy của FW1) vào 0x10000
    end_fw2 = ADDR_FW2 + len(fw_img_data)
    flash_data[ADDR_FW2:end_fw2] = fw_img_data
    print(f" -> Đã nạp FW2 (độ dài {len(fw_img_data)} bytes) vào 0x{ADDR_FW2:05X}")

    # Nạp Font vào 0x20000
    end_font = ADDR_FONT + len(font_data)
    flash_data[ADDR_FONT:end_font] = font_data
    print(f" -> Đã nạp Font (độ dài {len(font_data)} bytes) vào 0x{ADDR_FONT:05X}")

    print("\n--- BƯỚC 3: Tạo Product Header cho SBL tại 0x38000 ---")
    # Định dạng Product Header 12 byte cho SBL trong OTP đọc:
    # - Byte 0-3: Signature 0x00005270 ("Rp")
    # - Byte 4-7: Active Image Address 1 (0x00000000)
    # - Byte 8-11: Active Image Address 2 (0x00010000)
    product_header = bytearray([
        0x70, 0x52, 0x00, 0x00,  # Signature: 0x00005270 ("Rp")
        0x00, 0x00, 0x00, 0x00,  # FW1 Address: 0x00000000
        0x00, 0x00, 0x01, 0x00   # FW2 Address: 0x00010000
    ])
    
    # Ghi Product Header vào 0x38000
    end_header = ADDR_HEADER + len(product_header)
    flash_data[ADDR_HEADER:end_header] = product_header
    print(f" -> Đã nạp Product Header vào 0x{ADDR_HEADER:05X}")

    # 4. Xuất file final
    with open(OUT_FINAL_FLASH, "wb") as f:
        f.write(flash_data)

    print(f"\n[OK] Đã tạo thành công file Flash tổng: '{OUT_FINAL_FLASH}'")

if __name__ == "__main__":
    main()
