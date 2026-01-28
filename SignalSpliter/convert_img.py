
from PIL import Image

def png_to_4bit_bitmap(png_path, output_path, size=(64, 64)):
    with Image.open(png_path) as img:
        # Zmień rozmiar na 64x64
        img = img.resize(size, Image.LANCZOS)
        # Konwersja do palety 16 kolorów (4 bity)
        img = img.convert("L")
        # Zapisz jako PNG lub BMP
        img.save(output_path)

def png_to_4bit_c_array(png_path, output_path, size=(40, 64), array_name="image_data"):
    with Image.open(png_path) as img:
        img = img.resize(size, Image.LANCZOS)
        img = img.convert('L')
        pixels = list(img.getdata())
        # Każdy bajt to dwa piksele (4 bity na piksel)
        packed = []
        for i in range(0, len(pixels), 2):
            hi = (pixels[i] & 0xF0) >> 4
            lo = (pixels[i+1] & 0xF0) >> 4 if i+1 < len(pixels) else 0

            hi = 15 - hi
            lo = 15 - lo


            packed.append((hi << 4) | lo)
        # Generowanie kodu C
        with open("/home/lukasz/Dokumenty/AGH/Alicja/SignalSpliter/Interface/logo_agh.h", "w") as f:
            f.write("#ifndef __LOGO_AGH_H__\n")
            f.write("#define __LOGO_AGH_H__\n\n")
            f.write(f"const uint8_t logo_agh[] = {{\n")
            
            mod = size[0] // 2

            for i, byte in enumerate(packed):
                if i % (mod) == 0:
                    f.write("    ")
                f.write(f"0x{byte:02X}, ")
                if (i+1) % (mod) == 0:
                    f.write("\n")
            f.write("\n};\n")
            f.write("#endif\n")

if __name__ == "__main__":
    import sys
    if len(sys.argv) != 3:
        print("Użycie: python main.py input.png output.png")
        sys.exit(1)
    # png_to_4bit_bitmap(sys.argv[1], f"{sys.argv[2]}.png")
    png_to_4bit_c_array(sys.argv[1], sys.argv[2])