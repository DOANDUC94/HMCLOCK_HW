import os
import re

# Thư mục chứa các file .h cần gộp
FONT_DIR = "./fonts"
# Tên file bin đầu ra
OUTPUT_BIN = "all_fonts.bin"

def extract_hex_data(file_path):
    with open(file_path, "r", encoding="utf-8") as f:
        content = f.read()
    
    # Tìm tất cả các chuỗi có định dạng hex (0xXX) nằm trong cặp ngoặc nhọn { }
    array_content = re.search(r"\{([^}]+)\}", content)
    if not array_content:
        return None
        
    # Tách các giá trị hex chuyển thành list số nguyên
    hex_strings = re.findall(r"0x[0-9a-fA-F]+", array_content.group(1))
    return [int(x, 16) for x in hex_strings]

def main():
    if not os.path.exists(FONT_DIR):
        print(f"Thư mục '{FONT_DIR}' không tồn tại. Hãy tạo thư mục và bỏ các file .h vào.")
        return

    # Lấy danh sách file .h và sắp xếp theo tên để cố định thứ tự offset
    files = sorted([f for f in os.listdir(FONT_DIR) if f.endswith('.h')])
    
    if not files:
        print("Không tìm thấy file .h nào trong thư mục!")
        return

    combined_data = bytearray()
    current_offset = 0
    
    print(f"{'Tên Font':<25} | {'Kích thước (Bytes)':<20} | {'Vị trí Offset (Hex)':<20}")
    print("-" * 72)

    for file_name in files:
        file_path = os.path.join(FONT_DIR, file_name)
        data = extract_hex_data(file_path)
        
        if data is None:
            print(f"Bỏ qua {file_name} (Không tìm thấy mảng dữ liệu)")
            continue
            
        size = len(data)
        # In ra thông tin tên file, kích thước, và vị trí bắt đầu (Offset) dạng Hex
        print(f"{file_name:<25} | {size:<20} | 0x{current_offset:08X}")
        
        # Gom dữ liệu lại
        combined_data.extend(data)
        current_offset += size

    # Ghi toàn bộ dữ liệu ra file .bin duy nhất
    with open(OUTPUT_BIN, "wb") as f:
        f.write(combined_data)
        
    print("-" * 72)
    print(f"Xong! Đã gộp thành công vào file: {OUTPUT_BIN}")
    print(f"Tổng kích thước file bin: {len(combined_data)} bytes")

if __name__ == "__main__":
    main()