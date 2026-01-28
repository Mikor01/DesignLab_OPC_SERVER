from PIL import Image
from PIL import ImageDraw
from PIL import ImageFont

WIDTH = 256
HEIGHT = 64

img = Image.new("1", (WIDTH, HEIGHT))
draw = ImageDraw.Draw(img)
font = ImageFont.truetype("Font/DejaVuSansMono.ttf", 12)




draw.text((-1, 22), "A", font=font, fill=1)
draw.text((-1, 44), "B", font=font, fill=1)


for i in range (1, 17):
    if i < 10:
        draw.text((13*i-2, 1), f"{i}", font=font, fill=1)
    else:
        draw.text((127+19*(i-10), 1), f"{i}", font=font, fill=1)


def draw_line():
    digit = 13
    start = 8
    for i in range(0, 10):
        draw.line((start + i*digit, 0, start+digit*i, 63), fill= 1)

    digit = 19
    start = 125
    for i in range(0, 7):
        draw.line((start + i*digit, 0, start+digit*i, 63), fill= 1)


def draw_rect():
    first = 23
    second = 45
    size = 12
    step = 13
    padding = -2

    for i in range(-1, 9):
        draw.rectangle((padding+(i+1)*step, first, padding+(i+1)*step+size, first+size), outline=1)
        draw.rectangle((padding+(i+1)*step, second, padding+(i+1)*step+size, second+size), outline=1)

    padding = 111
    step = 19
    for i in range(7):
        draw.rectangle((padding+(i+1)*step, first, padding+(i+1)*step+size, first+size), outline=1)
        draw.rectangle((padding+(i+1)*step, second, padding+(i+1)*step+size, second+size), outline=1)


def draw_circ():
    first = 29
    second = 51
    size = 12
    step = 13
    padding = 1

    for i in range(9):
        draw.circle((padding+(i+1)*step, first), size//2, outline=1)
        draw.circle((padding+(i+1)*step, second), size//2, outline=1)

    padding = padding+114
    step = 19
    for i in range(7):
        draw.circle((padding+(i+1)*step, first), size//2, outline=1)
        draw.circle((padding+(i+1)*step, second), size//2, outline=1)

# draw_line()
# draw_circ()
draw_rect()


img.show()
# img.save("Display_dot.png")
