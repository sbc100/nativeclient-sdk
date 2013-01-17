from PIL import Image


image_size = 512,512
img = Image.new("L",image_size,255)

for i in range(8):
	for x in range(512):
		img.putpixel((x,i*64),0)
		img.putpixel((i*64,x),0)

img.save('out.png')